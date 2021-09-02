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
 * @file AMOP.cpp
 * @author: octopus
 * @date 2021-08-23
 */
#include <bcos-cpp-sdk/amop/AMOP.h>
#include <bcos-cpp-sdk/amop/Common.h>
#include <bcos-cpp-sdk/ws/WsMessageType.h>
#include <bcos-cpp-sdk/ws/WsService.h>
#include <bcos-cpp-sdk/ws/WsSession.h>
#include <bcos-framework/libutilities/Common.h>
#include <json/json.h>
#include <boost/core/ignore_unused.hpp>
#include <memory>


using namespace bcos;
using namespace bcos::ws;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::amop;

// subscribe topics
void AMOP::subscribe(const std::set<std::string>& _topics)
{
    // add topics to manager and update topics to server
    m_topicManager->addTopics(_topics);
    updateTopicsToRemote();
}

// subscribe topics
void AMOP::unsubscribe(const std::set<std::string>& _topics)
{
    // add topics to manager
    m_topicManager->removeTopics(_topics);
    updateTopicsToRemote();
}

// query all subscribed topics
void AMOP::querySubTopics(std::set<std::string>& _topics)
{
    _topics = m_topicManager->topics();
    AMOP_CLIENT(INFO) << LOG_BADGE("querySubTopics") << LOG_KV("topics size", _topics.size());
}

// subscribe topic with callback
void AMOP::subscribe(const std::string& _topic, AMOPCallback _callback)
{
    m_topicManager->addTopic(_topic);
    addTopicCallback(_topic, _callback);
    updateTopicsToRemote();
    AMOP_CLIENT(INFO) << LOG_BADGE("subscribe") << LOG_DESC("subscribe topic with callback")
                      << LOG_KV("topic", _topic);
}

// publish message
void AMOP::publish(const std::string& _topic, std::shared_ptr<bcos::bytes>& _msg, uint32_t timeout,
    AMOPCallback _callback)
{
    auto request = m_requestFactory->buildRequest();
    request->setTopic(_topic);
    request->setData(bytesConstRef(_msg->data(), _msg->size()));

    auto buffer = std::make_shared<bcos::bytes>();
    request->encode(*buffer);

    auto sendMsg = m_messageFactory->buildMessage();
    sendMsg->setType(bcos::ws::WsMessageType::AMOP_REQUEST);
    sendMsg->setData(buffer);

    auto sendBuffer = std::make_shared<bcos::bytes>();
    sendMsg->encode(*sendBuffer);

    auto service = m_service.lock();
    if (service)
    {
        AMOP_CLIENT(INFO) << LOG_BADGE("publish") << LOG_DESC("publish message to topic")
                          << LOG_KV("topic", _topic);
        service->asyncSendMessage(sendMsg, ws::Options(timeout),
            [_callback](bcos::Error::Ptr _error, std::shared_ptr<ws::WsMessage> _msg,
                std::shared_ptr<ws::WsSession> _session) { _callback(_error, _msg, _session); });
    }
}

// broadcast message
void AMOP::broadcast(const std::string& _topic, std::shared_ptr<bcos::bytes>& _msg)
{
    auto request = m_requestFactory->buildRequest();
    request->setTopic(_topic);
    request->setData(bytesConstRef(_msg->data(), _msg->size()));

    auto buffer = std::make_shared<bcos::bytes>();
    request->encode(*buffer);

    auto sendMsg = m_messageFactory->buildMessage();
    sendMsg->setType(bcos::ws::WsMessageType::AMOP_BROADCAST);
    sendMsg->setData(buffer);

    auto sendBuffer = std::make_shared<bcos::bytes>();
    sendMsg->encode(*sendBuffer);

    auto service = m_service.lock();
    if (service)
    {
        service->broadcastMessage(sendMsg);
    }
}

// set default callback
void AMOP::setCallback(AMOPCallback _callback)
{
    m_callback = _callback;
}

void AMOP::updateTopicsToRemote()
{
    auto service = m_service.lock();
    if (!service)
    {
        return;
    }

    auto ss = service->sessions();
    for (auto session : ss)
    {
        updateTopicsToRemote(session);
    }
}

void AMOP::updateTopicsToRemote(std::shared_ptr<ws::WsSession> _session)
{
    std::string request = m_topicManager->topicsToJsonString();
    auto msg = m_messageFactory->buildMessage();
    msg->setType(bcos::ws::WsMessageType::AMOP_SUBTOPIC);
    msg->setData(std::make_shared<bcos::bytes>(request.begin(), request.end()));

    _session->asyncSendMessage(msg);

    AMOP_CLIENT(INFO) << LOG_BADGE("updateTopicsToRemote")
                      << LOG_DESC("send subscribe message to ws server")
                      << LOG_KV("endpoint", _session->endPoint()) << LOG_KV("topics", request);
}

void AMOP::onRecvAMOPRequest(
    std::shared_ptr<ws::WsMessage> _msg, std::shared_ptr<ws::WsSession> _session)
{
    auto request = m_requestFactory->buildRequest();
    request->decode(bytesConstRef(_msg->data()->data(), _msg->data()->size()));
    auto topic = request->topic();
    auto data = std::make_shared<bcos::bytes>(request->data().begin(), request->data().end());

    WEBSOCKET_VERSION(INFO) << LOG_DESC("onRecvAMOPRequest")
                            << LOG_KV("endpoint", _session->endPoint())
                            << LOG_KV("data size", data->size());

    AMOPCallback callback = getCallbackByTopic(topic);
    if (!callback && m_callback)
    {
        callback = m_callback;
    }

    if (callback)
    {
        callback(nullptr, _msg, _session);
    }
    else
    {
        WEBSOCKET_VERSION(ERROR) << LOG_BADGE("onRecvAMOPRequest")
                                 << LOG_DESC("there has no callback register for the topic")
                                 << LOG_KV("topic", topic);
    }
}

void AMOP::onRecvAMOPResponse(
    std::shared_ptr<ws::WsMessage> _msg, std::shared_ptr<ws::WsSession> _session)
{
    auto seq = std::string(_msg->seq()->begin(), _msg->seq()->end());
    boost::ignore_unused(_msg, _session);
    WEBSOCKET_VERSION(WARNING) << LOG_DESC("onRecvAMOPResponse") << LOG_KV("seq", seq)
                               << LOG_KV("endpoint", _session->endPoint());
}

void AMOP::onRecvAMOPBroadcast(
    std::shared_ptr<ws::WsMessage> _msg, std::shared_ptr<ws::WsSession> _session)
{
    auto request = m_requestFactory->buildRequest();
    request->decode(bytesConstRef(_msg->data()->data(), _msg->data()->size()));
    auto topic = request->topic();
    auto data = std::make_shared<bcos::bytes>(request->data().begin(), request->data().end());

    WEBSOCKET_VERSION(INFO) << LOG_DESC("onRecvAMOPBroadcast")
                            << LOG_KV("endpoint", _session->endPoint())
                            << LOG_KV("data size", data->size());

    AMOPCallback callback = getCallbackByTopic(topic);
    if (!callback && m_callback)
    {
        callback = m_callback;
    }

    if (callback)
    {
        callback(nullptr, _msg, _session);
    }
    else
    {
        WEBSOCKET_VERSION(ERROR) << LOG_BADGE("onRecvAMOPBroadcast")
                                 << LOG_DESC("there has no callback register for the topic")
                                 << LOG_KV("topic", topic);
    }
}