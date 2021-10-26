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
#include <bcos-cpp-sdk/ws/Service.h>
#include <memory>

using namespace bcos;
using namespace bcos::boostssl;
using namespace bcos::boostssl::ws;

using namespace bcos::cppsdk;
using namespace bcos::cppsdk::amop;
using namespace bcos::cppsdk::jsonrpc;
using namespace bcos::cppsdk::event;
using namespace bcos::cppsdk::service;

Service::Ptr SdkFactory::buildService()
{
    auto service = std::make_shared<Service>();
    auto initializer = std::make_shared<WsInitializer>();

    auto groupInfoFactory = std::make_shared<bcos::group::GroupInfoFactory>();
    auto chainNodeInfoFactory = std::make_shared<bcos::group::ChainNodeInfoFactory>();


    initializer->setConfig(m_config);
    initializer->initWsService(service);
    service->setWaitConnectFinish(true);
    service->setGroupInfoFactory(groupInfoFactory);
    service->setChainNodeInfoFactory(chainNodeInfoFactory);

    service->registerMsgHandler(bcos::cppsdk::jsonrpc::MessageType::BLOCK_NOTIFY,
        [service](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            auto blkMsg = std::string(_msg->data()->begin(), _msg->data()->end());

            service->onRecvBlockNotifier(blkMsg);

            BCOS_LOG(INFO) << "[WS]" << LOG_DESC("receive block notify")
                           << LOG_KV("endpoint", _session->endPoint()) << LOG_KV("blk", blkMsg);
        });

    service->registerMsgHandler(bcos::cppsdk::jsonrpc::MessageType::GROUP_NOTIFY,
        [service](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
            std::string groupInfo = std::string(_msg->data()->begin(), _msg->data()->end());

            service->onNotifyGroupInfo(groupInfo, _session);

            BCOS_LOG(INFO) << "[WS]" << LOG_DESC("receive group info notify")
                           << LOG_KV("endpoint", _session->endPoint())
                           << LOG_KV("groupInfo", groupInfo);
        });

    return service;
}

bcos::cppsdk::jsonrpc::JsonRpcImpl::UniquePtr SdkFactory::buildJsonRpc(Service::Ptr _service)
{
    auto jsonRpc = std::make_unique<JsonRpcImpl>();
    auto factory = std::make_shared<JsonRpcRequestFactory>();
    jsonRpc->setFactory(factory);
    jsonRpc->setService(_service);

    jsonRpc->setSender([_service](const std::string& _group, const std::string& _node,
                           const std::string& _request, bcos::cppsdk::jsonrpc::RespFunc _respFunc) {
        auto data = std::make_shared<bcos::bytes>(_request.begin(), _request.end());
        auto msg = _service->messageFactory()->buildMessage();
        msg->setType(bcos::cppsdk::jsonrpc::MessageType::RPC_REQUEST);
        msg->setData(data);

        _service->asyncSendMessageByGroupAndNode(_group, _node, msg, Options(-1),
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
        {  // TODO: it should update topics info to remote when service handshake successfully
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