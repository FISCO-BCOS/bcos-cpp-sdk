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
 * @file get_group_info.cpp
 * @author: octopus
 * @date 2021-08-24
 */

#include <bcos-boostssl/utilities/Common.h>
#include <bcos-boostssl/utilities/ThreadPool.h>
#include <bcos-boostssl/websocket/Common.h>
#include <bcos-boostssl/websocket/WsMessage.h>
#include <bcos-boostssl/websocket/WsService.h>
#include <bcos-boostssl/websocket/WsSession.h>
#include <bcos-boostssl/websocket/WsTools.h>
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
    std::cerr << "Usage: get-group-info <group> <host1:port1> <host2:port2>  \n"
              << "Example:\n"
              << "    ./get-group-info group 127.0.0.1:20200 127.0.0.1:20201"
                 "\n";
    std::exit(0);
}


int main(int argc, char** argv)
{
    if (argc < 3)
    {
        usage();
    }

    auto peers = std::make_shared<bcos::boostssl::ws::EndPoints>();
    std::string group = argv[1];

    for (int i = 2; i < argc; i++)
    {
        bcos::boostssl::ws::EndPoint endpoint;
        ws::WsTools::stringToEndPoint(argv[i], endpoint);
        peers->push_back(endpoint);
    }

    std::cout << LOG_BADGE(" [GetGroupInfo] parameters ===>>>> ") << LOG_KV("\n\t # group", group)
              << std::endl;

    auto config = std::make_shared<bcos::boostssl::ws::WsConfig>();
    config->setModel(bcos::boostssl::ws::WsModel::Client);
    config->setConnectedPeers(peers);
    config->setThreadPoolSize(4);
    config->setDisableSsl(true);

    auto factory = std::make_shared<SdkFactory>();
    factory->setConfig(config);

    auto sdk = factory->buildSdk();
    sdk->start();

    int i = 0;
    while (true)
    {
        sdk->jsonRpc()->getGroupInfo(group, [](auto&& _error, auto&& _respData) {
            (void)_error;
            (void)_respData;
            std::cout << LOG_BADGE(" [GetGroupInfo] ===>>>> ")
                      << LOG_KV("resp", std::string(_respData->begin(), _respData->end()))
                      << std::endl;
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        i++;
    }

    return EXIT_SUCCESS;
}