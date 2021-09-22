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
#include <bcos-cpp-sdk/event/EventPushResponse.h>
#include <bcos-cpp-sdk/event/EventPushStatus.h>
#include <bcos-cpp-sdk/ws/Common.h>
#include <bcos-framework/interfaces/protocol/CommonError.h>
#include <bcos-framework/libutilities/Common.h>
#include <bcos-framework/libutilities/Log.h>
#include <json/reader.h>
#include <mutex>
#include <shared_mutex>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::event;

void EventPush::start()
{
    if (m_running)
    {
        EVENT_PUSH(INFO) << LOG_BADGE("start") << LOG_DESC("event push is running");
        return;
    }
    m_running = true;

    m_timer = std::make_shared<boost::asio::deadline_timer>(boost::asio::make_strand(*m_ioc),
        boost::posix_time::milliseconds(m_config->reconnectPeriod()));
    auto self = std::weak_ptr<EventPush>(shared_from_this());
    m_timer->async_wait([self](const boost::system::error_code&) {
        auto ep = self.lock();
        if (!ep)
        {
            return;
        }
        ep->doLoop();
    });

    EVENT_PUSH(INFO) << LOG_BADGE("start") << LOG_DESC("start event push successfully")
                     << LOG_KV("sendMsgTimeout", m_config->sendMsgTimeout())
                     << LOG_KV("reconnectPeriod", m_config->reconnectPeriod());
}

void EventPush::stop()
{
    if (!m_running)
    {
        EVENT_PUSH(INFO) << LOG_BADGE("stop") << LOG_DESC("event push is not running");
        return;
    }

    m_running = false;
    if (m_timer)
    {
        m_timer->cancel();
    }

    EVENT_PUSH(INFO) << LOG_BADGE("stop") << LOG_DESC("stop event push successfully");
}

void EventPush::doLoop()
{
    if (m_suspendTasksCount.load() == 0)
    {
        return;
    }

    auto ss = m_wsService->sessions();
    if (ss.empty())
    {
        EVENT_PUSH(INFO) << LOG_BADGE("doLoop") << LOG_DESC("no active sessions available");
        return;
    }

    std::shared_lock lock(x_tasks);
    for (const auto& taskEntry : m_suspendTasks)
    {
        auto task = taskEntry.second;
        std::string id = task->id();
        if (m_waitRespTasks.find(id) != m_waitRespTasks.end())
        {
            continue;
        }

        m_waitRespTasks.insert(id);

        auto self = std::weak_ptr<EventPush>(shared_from_this());
        subscribeEvent(task, [id, self](bcos::Error::Ptr, const std::string&, const std::string&) {
            auto ep = self.lock();
            if (!ep)
            {
                return;
            }

            ep->removeWaitResp(id);
        });
    }
}

void EventPush::addTask(const std::string& _id, EventPushTask::Ptr _task)
{
    std::unique_lock lock(x_tasks);
    auto it = m_suspendTasks.find(_id);
    if (it != m_suspendTasks.end())
    {
        m_suspendTasksCount--;
        m_suspendTasks.erase(it);
    }
    m_workingTasks[_id] = _task;
}

EventPushTask::Ptr EventPush::getTask(const std::string& _id)
{
    EventPushTask::Ptr task = nullptr;

    std::shared_lock lock(x_tasks);
    auto it = m_workingTasks.find(_id);
    if (it != m_workingTasks.end())
    {
        task = it->second;

        EVENT_PUSH(TRACE) << LOG_BADGE("getTask") << LOG_DESC("event push task is working")
                          << LOG_KV("id", task->id());
    }
    else
    {
        auto innerIt = m_suspendTasks.find(_id);
        if (innerIt != m_suspendTasks.end())
        {
            task = it->second;

            EVENT_PUSH(TRACE) << LOG_BADGE("getTask") << LOG_DESC("event push task suspend")
                              << LOG_KV("id", task->id());
        }
        else
        {
            EVENT_PUSH(DEBUG) << LOG_BADGE("getTask") << LOG_DESC("cannot found event push task")
                              << LOG_KV("id", task->id());
        }
    }

    return task;
}

EventPushTask::Ptr EventPush::getTaskAndRemove(const std::string& _id)
{
    EventPushTask::Ptr task = nullptr;

    std::shared_lock lock(x_tasks);
    auto it = m_workingTasks.find(_id);
    if (it != m_workingTasks.end())
    {  // remove from m_workingTasks
        task = it->second;
        m_workingTasks.erase(it);

        EVENT_PUSH(TRACE) << LOG_BADGE("getTaskAndRemove") << LOG_DESC("event push task is working")
                          << LOG_KV("id", task->id());
    }
    else
    {
        // remove from m_suspendTasks
        auto innerIt = m_suspendTasks.find(_id);
        if (innerIt != m_suspendTasks.end())
        {
            task = it->second;
            m_suspendTasksCount--;
            m_suspendTasks.erase(it);

            EVENT_PUSH(TRACE) << LOG_BADGE("getTaskAndRemove")
                              << LOG_DESC("event push task suspend") << LOG_KV("id", task->id());
        }
    }

    return task;
}

void EventPush::removeWaitResp(const std::string& _id)
{
    std::unique_lock lock(x_tasks);
    m_waitRespTasks.erase(_id);
}

void EventPush::suspendTasks(std::shared_ptr<ws::WsSession> _session)
{
    std::unique_lock lock(x_tasks);
    for (auto it = m_workingTasks.begin(); it != m_workingTasks.end();)
    {
        auto task = it->second;
        auto s = task->session();
        if (!s)
        {
            EVENT_PUSH(ERROR) << LOG_BADGE("suspendTasks")
                              << LOG_DESC("something is wrong for session is nullptr")
                              << LOG_KV("id", task->id());
            ++it;
            continue;
        }

        if (s.get() != _session.get())
        {
            ++it;
            continue;
        }

        // TODO : try to send the task again immediately is better ???
        EVENT_PUSH(INFO) << LOG_BADGE("suspendTasks")
                         << LOG_DESC("suspend event push task for network disconnect")
                         << LOG_KV("id", task->id());

        it = m_workingTasks.erase(it);
        task->setSession(nullptr);
        m_suspendTasksCount++;
        m_suspendTasks[task->id()] = task;
    }
}

void EventPush::onRecvEventPushMessage(
    std::shared_ptr<ws::WsMessage> _msg, std::shared_ptr<ws::WsSession> _session)
{
    /*
    {
        "id": "",
        "status": 0,
        "result": {
            "blockNumber": 111,
            "events": [
                {},
                {},
                {}
            ]
        }
    }
    */
    auto strResp = std::string(_msg->data()->begin(), _msg->data()->end());
    auto resp = std::make_shared<EventPushResponse>();
    if (!resp->fromJson(strResp))
    {
        EVENT_PUSH(ERROR) << LOG_BADGE("onRecvEventPushMessage")
                          << LOG_DESC("recv invalid event push message")
                          << LOG_KV("endpoint", _session->endPoint())
                          << LOG_KV("response", strResp);
        return;
    }

    EVENT_PUSH(DEBUG) << LOG_BADGE("onRecvEventPushMessage")
                      << LOG_DESC("receive event push message")
                      << LOG_KV("endpoint", _session->endPoint()) << LOG_KV("id", resp->id())
                      << LOG_KV("response", strResp);

    auto task = getTask(resp->id());
    if (task == nullptr)
    {
        EVENT_PUSH(ERROR) << LOG_BADGE("onRecvEventPushMessage")
                          << LOG_DESC("event push task not exist")
                          << LOG_KV("endpoint", _session->endPoint()) << LOG_KV("id", task->id())
                          << LOG_KV("response", strResp);
        return;
    }

    if (resp->status() == StatusCode::EndOfPush)
    {  // event push end
        getTaskAndRemove(resp->id());
        task->callback()(nullptr, resp->id(), strResp);

        EVENT_PUSH(INFO) << LOG_BADGE("onRecvEventPushMessage") << LOG_DESC("end of push")
                         << LOG_KV("endpoint", _session->endPoint()) << LOG_KV("id", task->id())
                         << LOG_KV("response", strResp);
    }
    else if (resp->status() != StatusCode::Success)
    {  // event push error
        getTaskAndRemove(resp->id());
        // normal event push
        task->callback()(nullptr, resp->id(), strResp);

        EVENT_PUSH(INFO) << LOG_BADGE("onRecvEventPushMessage") << LOG_DESC("event push error")
                         << LOG_KV("endpoint", _session->endPoint()) << LOG_KV("id", task->id())
                         << LOG_KV("response", strResp);
    }
    else
    {
        // event push
        task->callback()(nullptr, resp->id(), strResp);

        EVENT_PUSH(TRACE) << LOG_BADGE("onRecvEventPushMessage") << LOG_DESC("event push")
                          << LOG_KV("endpoint", _session->endPoint()) << LOG_KV("id", task->id())
                          << LOG_KV("response", strResp);
    }
}

void EventPush::subscribeEvent(EventPushTask::Ptr _task, Callback _callback)
{
    auto request = std::make_shared<EventPushSubRequest>();

    auto id = _task->id();
    auto group = _task->group();
    request->setId(id);
    request->setParams(_task->params());
    request->setGroup(_task->group());
    request->setState(_task->state());
    auto jsonReq = request->generateJson();

    auto message = m_messagefactory->buildMessage();
    message->setType(ws::WsMessageType::EVENT_SUBSCRIBE);
    message->setData(std::make_shared<bcos::bytes>(jsonReq.begin(), jsonReq.end()));

    EVENT_PUSH(INFO) << LOG_BADGE("subscribeEvent") << LOG_DESC("subscribe event push")
                     << LOG_KV("id", id) << LOG_KV("group", group) << LOG_KV("request", jsonReq);

    auto self = std::weak_ptr<EventPush>(shared_from_this());
    m_wsService->asyncSendMessage(message, ws::Options(m_config->sendMsgTimeout()),
        [id, _task, _callback, self](bcos::Error::Ptr _error, std::shared_ptr<ws::WsMessage> _msg,
            std::shared_ptr<ws::WsSession> _session) {
            auto ep = self.lock();
            if (!ep)
            {
                return;
            }

            if (_error && _error->errorCode() != bcos::protocol::CommonError::SUCCESS)
            {
                EVENT_PUSH(ERROR) << LOG_BADGE("subscribeEvent")
                                  << LOG_DESC("callback response error") << LOG_KV("id", id)
                                  << LOG_KV("errorCode", _error->errorCode())
                                  << LOG_KV("errorMessage", _error->errorMessage());

                _callback(_error, id, "subscribe event failed");
                return;
            }

            auto strResp = std::string(_msg->data()->begin(), _msg->data()->end());
            auto resp = std::make_shared<EventPushResponse>();
            if (!resp->fromJson(strResp))
            {
                EVENT_PUSH(ERROR) << LOG_BADGE("subscribeEvent")
                                  << LOG_DESC("invalid subscribe event response")
                                  << LOG_KV("id", id) << LOG_KV("response", strResp);
                _callback(nullptr, id, strResp);
            }
            else if (resp->status() != StatusCode::Success)
            {
                _callback(nullptr, id, strResp);
                EVENT_PUSH(ERROR) << LOG_BADGE("subscribeEvent")
                                  << LOG_DESC("callback response error") << LOG_KV("id", id)
                                  << LOG_KV("response", strResp);
            }
            else
            {
                // subscribe event successfully
                _task->setSession(_session);

                ep->addTask(id, _task);

                _callback(nullptr, id, strResp);
                EVENT_PUSH(INFO) << LOG_BADGE("subscribeEvent")
                                 << LOG_DESC("callback response success") << LOG_KV("id", id)
                                 << LOG_KV("response", strResp);
            }
        });
}

void EventPush::subscribeEvent(
    const std::string& _group, EventPushParams::Ptr _params, Callback _callback)
{
    auto task = std::make_shared<EventPushTask>();
    task->setId(m_messagefactory->newSeq());
    task->setGroup(_group);
    task->setParams(_params);
    task->setCallback(_callback);
    task->setState(std::make_shared<EventPushTaskState>());

    return subscribeEvent(task, _callback);
}

void EventPush::unsubscribeEvent(const std::string& _id, Callback _callback)
{
    auto task = getTaskAndRemove(_id);
    if (task == nullptr)
    {
        // TODO: error code define
        auto error = std::make_shared<Error>(-1, "event push task not found");
        _callback(error, _id, "event push task not found");
        EVENT_PUSH(ERROR) << LOG_BADGE("unsubscribeEvent") << LOG_DESC("event push task not found")
                          << LOG_KV("id", _id);
        return;
    }

    auto session = task->session();
    if (!session)
    {
        _callback(nullptr, _id, "unsubscribe event successfully(task is suspend)");
        EVENT_PUSH(INFO) << LOG_BADGE("unsubscribeEvent") << LOG_DESC("task is suspend")
                         << LOG_KV("id", _id);
        return;
    }

    auto request = std::make_shared<EventPushUnsubRequest>();
    request->setId(_id);
    request->setGroup(task->group());
    auto strReq = request->generateJson();

    auto message = m_messagefactory->buildMessage();
    message->setType(ws::WsMessageType::EVENT_UNSUBSCRIBE);
    message->setData(std::make_shared<bcos::bytes>(strReq.begin(), strReq.end()));

    session->asyncSendMessage(message, ws::Options(m_config->sendMsgTimeout()),
        [_id, _callback](bcos::Error::Ptr _error, std::shared_ptr<ws::WsMessage> _msg,
            std::shared_ptr<ws::WsSession>) {
            if (_error && _error->errorCode() != bcos::protocol::CommonError::SUCCESS)
            {
                EVENT_PUSH(ERROR) << LOG_BADGE("unsubscribeEvent")
                                  << LOG_DESC("callback response error") << LOG_KV("id", _id)
                                  << LOG_KV("errorCode", _error->errorCode())
                                  << LOG_KV("errorMessage", _error->errorMessage());

                _callback(_error, _id, "");
                return;
            }

            auto resp = std::make_shared<EventPushResponse>();
            auto strResp = std::string(_msg->data()->begin(), _msg->data()->end());
            if (!resp->fromJson(strResp))
            {
                EVENT_PUSH(ERROR) << LOG_BADGE("unsubscribeEvent")
                                  << LOG_DESC("callback invalid response") << LOG_KV("id", _id)
                                  << LOG_KV("response", strResp);

                _callback(nullptr, _id, strResp);
            }
            else if (resp->status() != StatusCode::Success)
            {
                EVENT_PUSH(ERROR) << LOG_BADGE("unsubscribeEvent")
                                  << LOG_DESC("callback response error") << LOG_KV("id", _id)
                                  << LOG_KV("status", resp->status())
                                  << LOG_KV("response", strResp);

                _callback(nullptr, _id, strResp);
            }
            else
            {
                _callback(nullptr, _id, strResp);

                EVENT_PUSH(INFO) << LOG_BADGE("unsubscribeEvent")
                                 << LOG_DESC("callback response success") << LOG_KV("id", _id)
                                 << LOG_KV("status", resp->status()) << LOG_KV("response", strResp);
            }
        });
}