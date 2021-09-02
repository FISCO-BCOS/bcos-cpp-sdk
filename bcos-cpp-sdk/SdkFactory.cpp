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
#include <bcos-cpp-sdk/amop/AMOP.h>
#include <bcos-cpp-sdk/rpc/JsonRpcImpl.h>
#include <bcos-cpp-sdk/ws/WsMessage.h>
#include <bcos-cpp-sdk/ws/WsTools.h>
#include <memory>

using namespace bcos;
using namespace bcos::ws;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::amop;
using namespace bcos::cppsdk::jsonrpc;

#define AMOP_MODULE (1000)
#define EVENTPUSH_MODULE (1001)

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

bcos::cppsdk::jsonrpc::JsonRcpImpl::Ptr SdkFactory::buildJsonRpc(
    bcos::ws::WsService::Ptr _wsService)
{
    auto jsonRpc = std::make_shared<JsonRcpImpl>();
    auto factory = std::make_shared<JsonRpcRequestFactory>();
    jsonRpc->setFactory(factory);
    auto wsServicePtr = std::weak_ptr<bcos::ws::WsService>(_wsService);
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
    return jsonRpc;
}

bcos::cppsdk::amop::AMOP::Ptr SdkFactory::buildAMOP(bcos::ws::WsService::Ptr _wsService)
{
    auto amop = std::make_shared<AMOP>();
    auto topicManager = std::make_shared<TopicManager>();
    auto requestFactory = std::make_shared<bcos::ws::AMOPRequestFactory>();
    auto messageFactory = std::make_shared<bcos::ws::WsMessageFactory>();

    amop->setTopicManager(topicManager);
    amop->setRequestFactory(requestFactory);
    amop->setMessageFactory(messageFactory);

    auto self = std::weak_ptr<AMOP>(amop);
    _wsService->registerMsgHandler(WsMessageType::AMOP_REQUEST,
        [self](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            auto amop = self.lock();
            if (amop)
            {
                amop->onRecvAMOPRequest(_msg, _session);
            }
        });
    _wsService->registerMsgHandler(WsMessageType::AMOP_RESPONSE,
        [self](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            auto amop = self.lock();
            if (amop)
            {
                amop->onRecvAMOPResponse(_msg, _session);
            }
        });
    _wsService->registerMsgHandler(WsMessageType::AMOP_BROADCAST,
        [self](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            auto amop = self.lock();
            if (amop)
            {
                amop->onRecvAMOPBroadcast(_msg, _session);
            }
        });

    auto amopWeakPtr = std::weak_ptr<AMOP>(amop);
    _wsService->registerModConnectHandler(
        AMOP_MODULE, [amopWeakPtr](std::shared_ptr<WsSession> _session) {
            auto amop = amopWeakPtr.lock();
            if (amop)
            {
                amop->updateTopicsToRemote(_session);
            }
        });
    /*
    _wsService->registerModDisconnectHandler(
        AMOP_MODULE, [amopWeakPtr](std::shared_ptr<WsSession> _session) {

        });
    */

    auto wsServicePtr = std::weak_ptr<bcos::ws::WsService>(_wsService);
    amop->setService(wsServicePtr);
    return amop;
}