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
#include <bcos-cpp-sdk/amop/AMOPRequest.h>
#include <bcos-cpp-sdk/amop/Common.h>
#include <bcos-cpp-sdk/rpc/Common.h>
#include <bcos-cpp-sdk/rpc/JsonRpcImpl.h>
#include <memory>

using namespace bcos;
using namespace bcos::boostssl;
using namespace bcos::boostssl::ws;

using namespace bcos::cppsdk;
using namespace bcos::cppsdk::amop;
using namespace bcos::cppsdk::group;
using namespace bcos::cppsdk::jsonrpc;
using namespace bcos::cppsdk::event;

WsService::Ptr SdkFactory::buildWsService()
{
    auto wsService = std::make_shared<WsService>();
    auto initializer = std::make_shared<WsInitializer>();
    initializer->setConfig(m_config);
    initializer->initWsService(wsService);
    wsService->setWaitConnectFinish(true);
    return wsService;
}

bcos::cppsdk::jsonrpc::JsonRpcImpl::UniquePtr SdkFactory::buildJsonRpc(WsService::Ptr _wsService)
{
    auto blockNotifier = std::make_shared<BlockNotifier>();
    auto jsonRpc = std::make_unique<JsonRpcImpl>();
    auto factory = std::make_shared<JsonRpcRequestFactory>();
    jsonRpc->setFactory(factory);
    jsonRpc->setBlockNotifier(blockNotifier);
    jsonRpc->setService(_wsService);

    auto blockNotifierWeakPtr = std::weak_ptr<BlockNotifier>(blockNotifier);

    _wsService->registerMsgHandler(bcos::cppsdk::jsonrpc::MessageType::BLOCK_NOTIFY,
        [blockNotifierWeakPtr](
            std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            auto bi = blockNotifierWeakPtr.lock();
            if (!bi)
            {
                return;
            }

            auto blkMsg = std::string(_msg->data()->begin(), _msg->data()->end());
            bi->onRecvBlockNotifier(blkMsg);

            BCOS_LOG(INFO) << "[WS]" << LOG_DESC("receive block notify")
                           << LOG_KV("endpoint", _session->endPoint()) << LOG_KV("blk", blkMsg);
        });

    jsonRpc->setSender(
        [_wsService](const std::string& _request, bcos::cppsdk::jsonrpc::RespFunc _respFunc) {
            auto data = std::make_shared<bcos::bytes>(_request.begin(), _request.end());
            auto msg = _wsService->messageFactory()->buildMessage();
            msg->setType(bcos::cppsdk::jsonrpc::MessageType::RPC_REQUEST);
            msg->setData(data);

            _wsService->asyncSendMessage(msg, Options(-1),
                [_respFunc](bcos::Error::Ptr _error, std::shared_ptr<WsMessage> _msg,
                    std::shared_ptr<WsSession> _session) {
                    (void)_session;
                    _respFunc(_error, _msg ? _msg->data() : nullptr);
                });
        });
    return jsonRpc;
}

bcos::cppsdk::amop::AMOP::UniquePtr SdkFactory::buildAMOP(WsService::Ptr _wsService)
{
    auto amop = std::make_unique<AMOP>();

    auto topicManager = std::make_shared<TopicManager>();
    auto requestFactory = std::make_shared<bcos::cppsdk::amop::AMOPRequestFactory>();
    auto messageFactory = std::make_shared<WsMessageFactory>();

    amop->setTopicManager(topicManager);
    amop->setRequestFactory(requestFactory);
    amop->setMessageFactory(messageFactory);
    amop->setService(_wsService);

    // TODO: is it safe to use raw pointer???
    auto amopPoint = amop.get();

    _wsService->registerMsgHandler(bcos::cppsdk::amop::MessageType::AMOP_REQUEST,
        [amopPoint](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            if (amopPoint)
            {
                amopPoint->onRecvAMOPRequest(_msg, _session);
            }
        });
    _wsService->registerMsgHandler(bcos::cppsdk::amop::MessageType::AMOP_RESPONSE,
        [amopPoint](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            if (amopPoint)
            {
                amopPoint->onRecvAMOPResponse(_msg, _session);
            }
        });
    _wsService->registerMsgHandler(bcos::cppsdk::amop::MessageType::AMOP_BROADCAST,
        [amopPoint](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            if (amopPoint)
            {
                amopPoint->onRecvAMOPBroadcast(_msg, _session);
            }
        });

    _wsService->registerConnectHandler([amopPoint](std::shared_ptr<WsSession> _session) {
        if (amopPoint)
        {
            amopPoint->updateTopicsToRemote(_session);
        }
    });
    return amop;
}

bcos::cppsdk::event::EventSub::UniquePtr SdkFactory::buildEventSub(WsService::Ptr _wsService)
{
    auto es = std::make_unique<event::EventSub>();
    auto messageFactory = std::make_shared<WsMessageFactory>();

    es->setMessageFactory(messageFactory);
    es->setWsService(_wsService);
    es->setConfig(_wsService->config());
    es->setIoc(_wsService->ioc());

    // TODO: is it safe to use raw pointer???
    auto eventPoint = es.get();

    _wsService->registerMsgHandler(bcos::cppsdk::event::MessageType::EVENT_LOG_PUSH,
        [eventPoint](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            if (eventPoint)
            {
                eventPoint->onRecvEventSubMessage(_msg, _session);
            }
        });

    return es;
}