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
 * @file pub_client.cpp
 * @author: octopus
 * @date 2021-08-24
 */

#include "bcos-cpp-sdk/ws/WsMessageType.h"
#include <bcos-cpp-sdk/Config.h>
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
#include <set>
#include <string>

using namespace bcos;
using namespace bcos::cppsdk;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void usage()
{
    std::cerr << "Usage: broadcast-client <host> <port> <topic> <message>\n"
              << "Example:\n"
              << "    ./broadcast-client 127.0.0.1 20200 topic\n";
    std::exit(0);
}


int main(int argc, char** argv)
{
    if (argc < 4)
    {
        usage();
    }

    std::string host = argv[1];
    uint16_t port = atoi(argv[2]);
    std::string topic = argv[3];
    std::string msg;
    if (argc > 4)
    {
        msg = argv[4];
    }

    BCOS_LOG(INFO) << LOG_DESC("broadcast client") << LOG_KV("ip", host) << LOG_KV("port", port)
                   << LOG_KV("topic", topic);

    auto config = std::make_shared<bcos::cppsdk::Config>();

    bcos::cppsdk::EndPoint endpoint;
    endpoint.host = host;
    endpoint.port = port;

    auto peers = std::make_shared<EndPoints>();
    peers->push_back(endpoint);
    config->setPeers(peers);
    config->setThreadPoolSize(4);

    auto threadPool = std::make_shared<bcos::ThreadPool>("t_sub", 4);

    auto factory = std::make_shared<SdkFactory>();
    factory->setConfig(config);

    auto wsService = factory->buildWsService();
    auto amop = factory->buildAMOP(wsService);

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

    int i = 0;
    while (true)
    {
        BCOS_LOG(INFO) << LOG_BADGE(" [AMOP] ===>>>> ") << LOG_DESC(" broadcast ")
                       << LOG_KV("topic", topic) << LOG_KV("message", msg);
        amop->broadcast(topic, bytesConstRef((bcos::byte*)msg.data(), msg.size()));
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        i++;
    }

    return EXIT_SUCCESS;
}