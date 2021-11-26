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

#include <bcos-boostssl/websocket/Common.h>
#include <bcos-boostssl/websocket/WsMessage.h>
#include <bcos-boostssl/websocket/WsService.h>
#include <bcos-boostssl/websocket/WsSession.h>
#include <bcos-cpp-sdk/SdkFactory.h>
#include <bcos-framework/libutilities/Common.h>
#include <bcos-framework/libutilities/Log.h>
#include <bcos-framework/libutilities/ThreadPool.h>
#include <boost/core/ignore_unused.hpp>
#include <cstdlib>
#include <memory>
#include <set>
#include <string>

using namespace bcos;
using namespace bcos::cppsdk;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void usage()
{
    std::cerr
        << "Usage: event-sub <host> <port> <group> <address> <from> <to>\n"
        << "Example:\n"
        << "    ./event-sub 127.0.0.1 20200 group 0x37a44585Bf1e9618FDb4C62c4c96189A07Dd4b48\n"
        << "    ./event-sub 127.0.0.1 20200 group 0x37a44585Bf1e9618FDb4C62c4c96189A07Dd4b48 1 10"
           "10\n";
    std::exit(0);
}


int main(int argc, char** argv)
{
    if (argc < 5)
    {
        usage();
    }

    std::vector<std::string> topics;
    std::string host = argv[1];
    uint16_t port = atoi(argv[2]);
    std::string group = argv[3];
    std::string address = argv[4];
    int64_t from = -1;
    int64_t to = -1;
    if (argc > 6)
    {
        from = atoi(argv[5]);
        to = atoi(argv[6]);
    }
    else if (argc > 5)
    {
        from = atoi(argv[6]);
    }

    std::cout << LOG_BADGE(" [EventSub] ===>>>> ") << LOG_KV("ip", host) << LOG_KV("port", port)
              << LOG_KV("group", group) << LOG_KV("address", address) << LOG_KV("from", from)
              << LOG_KV("to", to) << std::endl;

    auto config = std::make_shared<bcos::boostssl::ws::WsConfig>();
    config->setModel(bcos::boostssl::ws::WsModel::Client);

    bcos::boostssl::ws::EndPoint endpoint;
    endpoint.host = host;
    endpoint.port = port;

    auto peers = std::make_shared<bcos::boostssl::ws::EndPoints>();
    peers->push_back(endpoint);
    config->setConnectedPeers(peers);
    config->setThreadPoolSize(4);
    config->setDisableSsl(true);

    auto factory = std::make_shared<SdkFactory>();
    factory->setConfig(config);

    auto wsService = factory->buildService();
    auto eventSub = factory->buildEventSub(wsService);

    eventSub->start();

    auto params = std::make_shared<bcos::cppsdk::event::EventSubParams>();
    params->addAddress(address);
    params->setFromBlock(from);
    params->setToBlock(to);

    eventSub->subscribeEvent(
        group, params, [](bcos::Error::Ptr _error, const std::string& _events) {
            std::cout << LOG_BADGE(" response ===>>>> ") << std::endl;
            std::cout << LOG_BADGE(" \t ===>>>> ")
                      << LOG_KV("errorCode", _error ? _error->errorCode() : 0) << std::endl;
            std::cout << LOG_BADGE(" \t ===>>>> ")
                      << LOG_KV("errorMessage", _error ? _error->errorMessage() : "") << std::endl;
            std::cout << LOG_BADGE(" \t ===>>>> ") << LOG_KV("events", _events) << std::endl;
        });

    int i = 0;
    while (true)
    {
        std::cout << LOG_BADGE(" [EventSub] ===>>>> ") << LOG_DESC(" main running ") << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        i++;
    }

    return EXIT_SUCCESS;
}