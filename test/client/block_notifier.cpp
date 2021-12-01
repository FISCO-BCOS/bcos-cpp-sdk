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
 * @file block_notifier.cpp
 * @author: octopus
 * @date 2021-08-24
 */

#include <bcos-boostssl/utilities/Common.h>
#include <bcos-boostssl/utilities/ThreadPool.h>
#include <bcos-boostssl/websocket/Common.h>
#include <bcos-boostssl/websocket/WsMessage.h>
#include <bcos-boostssl/websocket/WsService.h>
#include <bcos-boostssl/websocket/WsSession.h>
#include <bcos-cpp-sdk/SdkFactory.h>
#include <boost/core/ignore_unused.hpp>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <set>
#include <string>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::boostssl;
using namespace bcos::boostssl::utilities;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void usage()
{
    std::cerr << "Usage: block-notifier <host> <port> <group> \n"
              << "Example:\n"
              << "    ./block-notifier 127.0.0.1 20200 group "
                 "\n";
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
    std::string group = argv[3];

    std::cout << LOG_BADGE(" [BlockNotifier] parameters ===>>>> ") << LOG_KV("\n\t # ip", host)
              << LOG_KV("\n\t # port", port) << LOG_KV("\n\t # group", group) << std::endl;

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

    auto service = factory->buildService();
    service->start();

    service->registerBlockNumberNotifier(
        group, [](const std::string& _group, int64_t _blockNumber) {
            std::cout << LOG_BADGE(" [Block] ===>>>> ") << LOG_KV("group", _group)
                      << LOG_KV("blockNumber", _blockNumber) << std::endl;
        });

    int i = 0;
    while (true)
    {
        std::cout << LOG_BADGE(" [BlockNotifier] ===>>>> ") << LOG_DESC(" main running ")
                  << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        i++;
    }

    return EXIT_SUCCESS;
}