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
 * @file SdkFactory.cpp
 * @author: octopus
 * @date 2021-08-21
 */

#include <bcos-cpp-sdk/SdkFactory.h>
#include <bcos-cpp-sdk/rpc/JsonRpcImpl.h>
#include <bcos-cpp-sdk/ws/WsMessage.h>
#include <bcos-cpp-sdk/ws/WsTools.h>

using namespace bcos;
using namespace bcos::ws;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::jsonrpc;

bcos::ws::WsService::Ptr SdkFactory::buildWsService()
{
    auto messageFactory = std::make_shared<bcos::ws::WsMessageFactory>();
    auto requestFactory = std::make_shared<bcos::ws::AMOPRequestFactory>();
    auto ioc = std::make_shared<boost::asio::io_context>();
    auto resolver = std::make_shared<boost::asio::ip::tcp::resolver>(*ioc);
    auto tools = std::make_shared<WsTools>();

    auto wsService = std::make_shared<bcos::ws::WsService>();
    wsService->setConfig(m_config);
    wsService->setThreadPool(m_threadPool);
    wsService->setIoc(ioc);
    wsService->setResolver(resolver);
    wsService->setTools(tools);
    wsService->setMessageFactory(messageFactory);
    wsService->setRequestFactory(requestFactory);
    wsService->initMethod();
    return wsService;
}

bcos::cppsdk::jsonrpc::JsonRcpImpl::Ptr SdkFactory::buildJsonRpc()
{
    auto jsonRpc = std::make_shared<JsonRcpImpl>();
    auto factory = std::make_shared<JsonRpcRequestFactory>();
    jsonRpc->setFactory(factory);
    return jsonRpc;
}