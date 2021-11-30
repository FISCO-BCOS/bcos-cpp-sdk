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
    std::cerr
        << "Usage: event-sub <host> <port> <group> <address> <from> <to> <topics>\n"
        << "Example:\n"
        << "    ./event-sub 127.0.0.1 20200 group 1 10 0x37a44585Bf1e9618FDb4C62c4c96189A07Dd4b48 "
           "\n";
    std::exit(0);
}


int main(int argc, char** argv)
{
    if (argc < 6)
    {
        usage();
    }

    std::string host = argv[1];
    uint16_t port = atoi(argv[2]);
    std::string group = argv[3];

    int64_t from = atoi(argv[4]);
    int64_t to = atoi(argv[5]);

    std::string address;
    std::vector<std::string> topics;

    if (argc > 7)
    {
        address = argv[6];
        for (int i = 7; i < argc; i++)
        {
            topics.push_back(argv[i]);
        }
    }
    else if (argc > 6)
    {
        address = argv[6];
    }

    std::cout << LOG_BADGE(" [EventSub] parameters ===>>>> ") << LOG_KV("\n\t # ip", host)
              << LOG_KV("\n\t # port", port) << LOG_KV("\n\t # group", group)
              << LOG_KV("\n\t # address", address) << LOG_KV("\n\t # from", from)
              << LOG_KV("\n\t # to", to) << std::endl;


    std::cout << LOG_DESC("\n\t # topics: ");
    for (auto& topic : topics)
    {
        std::cout << topic << " ";
    }

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
    if (!address.empty())
    {
        params->addAddress(address);
    }
    params->setFromBlock(from);
    params->setToBlock(to);
    for (std::size_t i = 0; i < topics.size(); ++i)
    {
        params->addTopic(i, topics[i]);
    }

    eventSub->subscribeEvent(group, params, [](Error::Ptr _error, const std::string& _events) {
        std::cout << " response ===>>>> " << std::endl;
        if (_error)
        {
            std::cout << " \t ===>>>> " << LOG_KV("errorCode", _error->errorCode()) << std::endl;
            std::cout << " \t ===>>>> " << LOG_KV("errorMessage", _error->errorMessage())
                      << std::endl;
        }
        else
        {
            std::cout << " \t ===>>>> " << LOG_KV("events", _events) << std::endl;
        }
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