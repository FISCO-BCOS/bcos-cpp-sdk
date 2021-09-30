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

#include <bcos-boostssl/websocket/WsConnector.h>
#include <bcos-boostssl/websocket/WsInitializer.h>
#include <bcos-boostssl/websocket/WsMessage.h>
#include <bcos-boostssl/websocket/WsService.h>
#include <bcos-cpp-sdk/SdkFactory.h>
#include <bcos-cpp-sdk/amop/AMOP.h>
#include <bcos-cpp-sdk/amop/AMOPMessageType.h>
#include <bcos-cpp-sdk/amop/AMOPRequest.h>
#include <bcos-cpp-sdk/event/EventSubMessageType.h>
#include <bcos-cpp-sdk/rpc/JsonRpcImpl.h>
#include <bcos-cpp-sdk/rpc/RpcMessageType.h>
#include <memory>

using namespace bcos;
using namespace bcos::boostssl;
using namespace bcos::boostssl::ws;

using namespace bcos::cppsdk;
using namespace bcos::cppsdk::amop;
using namespace bcos::cppsdk::jsonrpc;
using namespace bcos::cppsdk::event;

WsService::Ptr SdkFactory::buildWsService()
{
    auto wsService = std::make_shared<WsService>();
    auto initializer = std::make_shared<WsInitializer>();
    initializer->setConfig(m_config);
    initializer->initWsService(wsService);
    return wsService;
}

bcos::cppsdk::jsonrpc::JsonRpcImpl::Ptr SdkFactory::buildJsonRpc(WsService::Ptr _wsService)
{
    auto jsonRpc = std::make_shared<JsonRpcImpl>();
    auto factory = std::make_shared<JsonRpcRequestFactory>();
    jsonRpc->setFactory(factory);
    auto wsServicePtr = std::weak_ptr<WsService>(_wsService);

    // TODO: how to handler block notify message
    _wsService->registerMsgHandler(RpcMessageType::BLOCK_NOTIFY,
        [wsServicePtr](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            auto blkMsg = std::string(_msg->data()->begin(), _msg->data()->end());

            BCOS_LOG(INFO) << "[WS]" << LOG_DESC("receive block notify")
                           << LOG_KV("endpoint", _session->endPoint()) << LOG_KV("blk", blkMsg);
        });

    jsonRpc->setSender(
        [wsServicePtr](const std::string& _request, bcos::cppsdk::jsonrpc::RespFunc _respFunc) {
            auto wsService = wsServicePtr.lock();
            if (!wsService)
            {
                return;
            }

            auto data = std::make_shared<bcos::bytes>(_request.begin(), _request.end());
            auto msg = wsService->messageFactory()->buildMessage();
            msg->setType(RpcMessageType::RPC_REQUEST);
            msg->setData(data);

            wsService->asyncSendMessage(msg, Options(-1),
                [_respFunc](bcos::Error::Ptr _error, std::shared_ptr<WsMessage> _msg,
                    std::shared_ptr<WsSession> _session) {
                    (void)_session;
                    _respFunc(_error, _msg ? _msg->data() : nullptr);
                });
        });
    return jsonRpc;
}

bcos::cppsdk::amop::AMOP::Ptr SdkFactory::buildAMOP(WsService::Ptr _wsService)
{
    auto amop = std::make_shared<AMOP>();
    auto topicManager = std::make_shared<TopicManager>();
    auto requestFactory = std::make_shared<bcos::cppsdk::amop::AMOPRequestFactory>();
    auto messageFactory = std::make_shared<WsMessageFactory>();

    amop->setTopicManager(topicManager);
    amop->setRequestFactory(requestFactory);
    amop->setMessageFactory(messageFactory);

    auto self = std::weak_ptr<AMOP>(amop);
    _wsService->registerMsgHandler(AMOPMessageType::AMOP_REQUEST,
        [self](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            auto amop = self.lock();
            if (amop)
            {
                amop->onRecvAMOPRequest(_msg, _session);
            }
        });
    _wsService->registerMsgHandler(AMOPMessageType::AMOP_RESPONSE,
        [self](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            auto amop = self.lock();
            if (amop)
            {
                amop->onRecvAMOPResponse(_msg, _session);
            }
        });
    _wsService->registerMsgHandler(AMOPMessageType::AMOP_BROADCAST,
        [self](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            auto amop = self.lock();
            if (amop)
            {
                amop->onRecvAMOPBroadcast(_msg, _session);
            }
        });

    auto amopWeakPtr = std::weak_ptr<AMOP>(amop);
    _wsService->registerConnectHandler([amopWeakPtr](std::shared_ptr<WsSession> _session) {
        auto amop = amopWeakPtr.lock();
        if (amop)
        {
            amop->updateTopicsToRemote(_session);
        }
    });

    auto wsServicePtr = std::weak_ptr<WsService>(_wsService);
    amop->setService(wsServicePtr);
    return amop;
}

bcos::cppsdk::event::EventSub::Ptr SdkFactory::buildEventSub(WsService::Ptr _wsService)
{
    auto es = std::make_shared<event::EventSub>();
    auto messageFactory = std::make_shared<WsMessageFactory>();

    es->setMessageFactory(messageFactory);
    es->setWsService(_wsService);
    es->setConfig(_wsService->config());
    es->setIoc(_wsService->ioc());

    auto self = std::weak_ptr<event::EventSub>(es);
    _wsService->registerMsgHandler(EventSubMessageType::EVENT_LOG_PUSH,
        [self](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            auto es = self.lock();
            if (es)
            {
                es->onRecvEventSubMessage(_msg, _session);
            }
        });

    return es;
}