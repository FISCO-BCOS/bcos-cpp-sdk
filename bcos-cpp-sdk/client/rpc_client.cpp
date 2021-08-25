/*
 *  Copyright (C) 2021 FISCO BCOS.
 *  SPDX-License-Identifier: Apache-2.0
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @file rpc_client.h
 * @author: octopus
 * @date 2021-08-24
 */

#include "bcos-cpp-sdk/ws/WsMessageType.h"
#include <bcos-cpp-sdk/SdkConfig.h>
#include <bcos-cpp-sdk/SdkFactory.h>
#include <bcos-cpp-sdk/ws/Common.h>
#include <bcos-cpp-sdk/ws/WsMessage.h>
#include <bcos-cpp-sdk/ws/WsService.h>
#include <bcos-cpp-sdk/ws/WsSession.h>
#include <bcos-framework/libutilities/Common.h>
#include <bcos-framework/libutilities/Log.h>
#include <bcos-framework/libutilities/ThreadPool.h>
#include <boost/core/ignore_unused.hpp>
#include <memory>
#include <string>


using namespace bcos;
using namespace bcos::cppsdk;
using Handler = std::function<void(boost::beast::websocket::stream<boost::beast::tcp_stream>&&)>;

//------------------------------------------------------------------------------

void fail(boost::beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
    std::exit(-1);
}

//------------------------------------------------------------------------------

void usage()
{
    std::cerr << "Usage: rpc-client <host> <port>\n"
              << "Example:\n"
              << "    ./rpc-client 127.0.0.1 20200\n";
    std::exit(0);
}


int main(int argc, char** argv)
{
    if (argc != 3)
    {
        usage();
    }

    std::string host = argv[1];
    uint16_t port = atoi(argv[2]);

    BCOS_LOG(INFO) << LOG_DESC("rpc client") << LOG_KV("ip", host) << LOG_KV("port", port);

    auto config = std::make_shared<bcos::cppsdk::SdkConfig>();

    std::set<boostssl::net::NodeIPEndpoint> peers;
    boost::asio::ip::address addr = boost::asio::ip::make_address(host);
    boostssl::net::NodeIPEndpoint nodeIPEndpoint(addr, port);
    peers.insert(nodeIPEndpoint);
    config->setPeers(peers);

    auto threadPool = std::make_shared<bcos::ThreadPool>("t_sub", 4);

    auto factory = std::make_shared<SdkFactory>();
    factory->setConfig(config);
    factory->setThreadPool(threadPool);

    auto wsService = factory->buildWsService();
    auto jsonRpc = factory->buildJsonRpc();
    auto wsServicePtr = std::weak_ptr<bcos::ws::WsService>(wsService);
    jsonRpc->setSender(
        [wsServicePtr](const std::string& _request, bcos::cppsdk::jsonrpc::RespFunc _respFunc) {
            auto wsService = wsServicePtr.lock();
            if (!wsService)
            {
                return;
            }

            auto data = std::make_shared<bcos::bytes>(_request.begin(), _request.end());
            auto msg = wsService->messageFactory()->buildMessage();
            msg->setType(ws::WsMessageType::RPC_REQUEST);
            msg->setData(data);

            wsService->asyncSendMessage(msg, bcos::ws::Options(-1),
                [_respFunc](bcos::Error::Ptr _error, std::shared_ptr<bcos::ws::WsMessage> _msg,
                    std::shared_ptr<bcos::ws::WsSession> _session) {
                    boost::ignore_unused(_session);
                    _respFunc(_error, _msg ? _msg->data() : nullptr);
                });
        });
    wsService->start();

    auto ioc = wsService->ioc();
    std::size_t threadC = 4;
    std::shared_ptr<std::vector<std::thread>> threads =
        std::make_shared<std::vector<std::thread>>();
    threads->reserve(threadC);
    for (auto i = threadC; i > 0; --i)
    {
        threads->emplace_back([&ioc]() { ioc->run(); });
    }

    while (true)
    {
        BCOS_LOG(INFO) << LOG_DESC(" ==> getNodeInfo ");
        jsonRpc->getNodeInfo([](bcos::Error::Ptr _error, std::shared_ptr<bcos::bytes> _resp) {
            boost::ignore_unused(_error);
            std::string strResp;
            if (_resp)
            {
                strResp = std::string(_resp->begin(), _resp->end());
            }
            BCOS_LOG(INFO) << LOG_DESC(" ==> getNodeInfo ")
                           << LOG_KV("errorCode", _error ? _error->errorCode() : -1)
                           << LOG_KV(
                                  "errorMessage", _error ? _error->errorMessage() : std::string(""))
                           << LOG_KV("resp", strResp);
        });

        BCOS_LOG(INFO) << LOG_DESC(" ==> getBlockNumber ");
        jsonRpc->getBlockNumber(
            "", [](bcos::Error::Ptr _error, std::shared_ptr<bcos::bytes> _resp) {
                boost::ignore_unused(_error);
                std::string strResp;
                if (_resp)
                {
                    strResp = std::string(_resp->begin(), _resp->end());
                }
                BCOS_LOG(INFO) << LOG_DESC(" ==> getBlockNumber ")
                               << LOG_KV("errorCode", _error ? _error->errorCode() : -1)
                               << LOG_KV("errorMessage",
                                      _error ? _error->errorMessage() : std::string(""))
                               << LOG_KV("resp", strResp);
            });
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return EXIT_SUCCESS;
}