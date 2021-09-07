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
 * @file EvenPush.cpp
 * @author: octopus
 * @date 2021-09-01
 */

#include <bcos-cpp-sdk/event/Common.h>
#include <bcos-cpp-sdk/event/EventPush.h>
#include <bcos-cpp-sdk/event/EventPushRequest.h>
#include <bcos-cpp-sdk/ws/Common.h>
#include <bcos-framework/interfaces/protocol/CommonError.h>
#include <bcos-framework/libutilities/Common.h>
#include <bcos-framework/libutilities/Log.h>
#include <json/reader.h>
#include <boost/core/ignore_unused.hpp>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::event;

void EventPush::start()
{
    if (m_running)
    {
        EVENT_IMPL(INFO) << LOG_BADGE("start") << LOG_DESC("event push is running");
        return;
    }
    m_running = true;

    m_timer = std::make_shared<boost::asio::deadline_timer>(
        boost::asio::make_strand(*m_ioc), boost::posix_time::milliseconds(10000));
    auto self = std::weak_ptr<EventPush>(shared_from_this());
    m_timer->async_wait([self](const boost::system::error_code&) {
        auto ep = self.lock();
        if (!ep)
        {
            return;
        }
        ep->doLoop();
    });

    EVENT_IMPL(INFO) << LOG_BADGE("start") << LOG_DESC("start event push successfully");
}

void EventPush::stop()
{
    if (!m_running)
    {
        EVENT_IMPL(INFO) << LOG_BADGE("stop") << LOG_DESC("event push is not running");
        return;
    }

    m_running = false;
    if (m_timer)
    {
        m_timer->cancel();
    }

    EVENT_IMPL(INFO) << LOG_BADGE("stop") << LOG_DESC("stop event push successfully");
}

void EventPush::doLoop()
{
    auto ss = m_wsService->sessions();
    if (ss.empty())
    {
        EVENT_IMPL(INFO) << LOG_BADGE("doLoop") << LOG_DESC(" no active sessions available, skip");
        return;
    }

    // std::vector<>
    std::shared_lock lock(x_tasks);
    for (const auto& taskEntry : m_interruptTasks)
    {
        auto task = taskEntry.second;
        std::string id = task->id();
        if (m_waitRespTasks.find(id) != m_waitRespTasks.end())
        {
            continue;
        }

        m_waitRespTasks.insert(id);

        // send task to remote and put it to m_waitRespTasks
        // m_wsService->asyncSendMessage();
    }
}

void EventPush::addTask(const std::string& _id, EventPushTask::Ptr _task)
{
    std::unique_lock lock(x_tasks);
    m_tasks[_id] = _task;
}

void EventPush::removeTask(const std::string& _id)
{
    std::unique_lock lock(x_tasks);
    m_tasks.erase(_id);
}

EventPushTask::Ptr EventPush::getTaskAndRemove(const std::string& _id)
{
    std::shared_lock lock(x_tasks);
    auto it = m_tasks.find(_id);
    if (it == m_tasks.end())
    {
        return nullptr;
    }

    auto task = it->second;
    m_tasks.erase(it);
    return task;
}

EventPushTask::Ptr EventPush::getTask(const std::string& _id)
{
    std::shared_lock lock(x_tasks);
    auto it = m_tasks.find(_id);
    if (it == m_tasks.end())
    {
        return nullptr;
    }

    return it->second;
}

void EventPush::interruptTasks(std::shared_ptr<ws::WsSession> _session)
{
    std::unique_lock lock(x_tasks);
    for (auto it = m_tasks.begin(); it != m_tasks.end();)
    {
        auto task = it->second;
        auto s = task->session();
        if (s.get() != _session.get())
        {
            ++it;
            continue;
        }

        EVENT_IMPL(INFO) << LOG_BADGE("interruptTasks")
                         << LOG_DESC("event push suspend for session disconnect")
                         << LOG_KV("id", task->id());

        it = m_tasks.erase(it);
        m_interruptTasks[task->id()] = task;
    }
}

void EventPush::onRecvEventPushMessage(
    std::shared_ptr<ws::WsMessage> _msg, std::shared_ptr<ws::WsSession> _session)
{
    boost::ignore_unused(_msg, _session);
}

void EventPush::onRecvSubEventRespMessage(const std::string& _id,
    std::shared_ptr<ws::WsMessage> _msg, std::shared_ptr<ws::WsSession> _session)
{
    boost::ignore_unused(_session);
    std::string resp = std::string(_msg->data()->begin(), _msg->data()->end());
    try
    {
        Json::Value root;
        Json::Reader jsonReader;
        if (!jsonReader.parse(resp, root))
        {
            // invalid json object
        }

        if (root.isMember("id"))
        {
            std::string id = root["id"].asString();
        }

        if (root.isMember("result"))
        {
            // int result = root["result"].asInt();
        }

        EVENT_IMPL(ERROR) << LOG_BADGE("onRecvSubEventRespMessage") << LOG_KV("id", _id);
    }
    catch (const std::exception& e)
    {
        EVENT_IMPL(ERROR) << LOG_BADGE("onRecvSubEventRespMessage") << LOG_KV("id", _id);
    }
}

void EventPush::subscribeEvent(
    const std::string& _group, EventPushParams::Ptr _params, Callback _callback)
{
    auto id = m_factory->newSeq();

    auto request = std::make_shared<EventPushRequest>();
    request->setId(id);
    request->setParams(_params);
    request->setGroup(_group);
    auto jsonReq = request->generateJson();

    auto message = m_factory->buildMessage();
    message->setType(ws::WsMessageType::EVENT_SUBSCRIBE);
    message->setData(std::make_shared<bcos::bytes>(jsonReq.begin(), jsonReq.end()));

    EVENT_IMPL(INFO) << LOG_BADGE("subscribeEvent") << LOG_DESC("subscribe event push")
                     << LOG_KV("id", id) << LOG_KV("group", _group) << LOG_KV("request", jsonReq);

    auto self = std::weak_ptr<EventPush>(shared_from_this());
    m_wsService->asyncSendMessage(message, ws::Options(-1),
        [id, _params, _callback, self](bcos::Error::Ptr _error, std::shared_ptr<ws::WsMessage> _msg,
            std::shared_ptr<ws::WsSession> _session) {
            auto ep = self.lock();
            if (!ep)
            {
                return;
            }

            if (_error && _error->errorCode() != bcos::protocol::CommonError::SUCCESS)
            {
                EVENT_IMPL(INFO) << LOG_BADGE("subscribeEvent")
                                 << LOG_DESC("callback response error") << LOG_KV("id", id)
                                 << LOG_KV("errorCode", _error->errorCode())
                                 << LOG_KV("errorMessage", _error->errorMessage());
                _callback(_error, "");
                return;
            }

            ep->onRecvSubEventRespMessage(id, _msg, _session);

            // TODO: handler response
            // subscribe successfully
            // auto task = std::make_shared<EventPushTask>();
            // task->setId(id);
            // task->setParams(_params);
            // task->setSession(_session);
            // task->setCallback(_callback);
            // ep->addTask(id, task);
        });
}

void EventPush::unsubscribeEvent(const std::string& _id, Callback _callback)
{
    auto task = getTask(_id);
    if (task == nullptr)
    {
        // TODO: task not exist
        _callback(nullptr, "");
        return;
    }

    auto message = m_factory->buildMessage();
    message->setType(ws::WsMessageType::EVENT_UNSUBSCRIBE);
    std::string params = "{\"id\":" + _id + "}";
    message->setData(std::make_shared<bcos::bytes>(params.begin(), params.end()));
    auto session = task->session();
    session->asyncSendMessage(message, ws::Options(-1),
        [](bcos::Error::Ptr, std::shared_ptr<ws::WsMessage>, std::shared_ptr<ws::WsSession>) {
            // boost::ignore_unused()
        });
}