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

#include <bcos-boostssl/websocket/Common.h>
#include <bcos-boostssl/websocket/WsMessage.h>
#include <bcos-boostssl/websocket/WsSession.h>
#include <bcos-cpp-sdk/event/Common.h>
#include <bcos-cpp-sdk/event/EventSub.h>
#include <bcos-cpp-sdk/event/EventSubMessageType.h>
#include <bcos-cpp-sdk/event/EventSubRequest.h>
#include <bcos-cpp-sdk/event/EventSubResponse.h>
#include <bcos-cpp-sdk/event/EventSubStatus.h>
#include <bcos-framework/interfaces/protocol/CommonError.h>
#include <bcos-framework/libutilities/Common.h>
#include <bcos-framework/libutilities/Log.h>
#include <json/reader.h>
#include <mutex>
#include <shared_mutex>

using namespace bcos;
using namespace bcos::boostssl;
using namespace bcos::boostssl::ws;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::event;

#define EP_DEFAULT_LOOP_PERIOD (10000)
#define EP_DEFAULT_SENDMSG_TIMEOUT (30000)

#define EP_LOOP_PERIOD(config) (config ? config->reconnectPeriod() : EP_DEFAULT_LOOP_PERIOD)
#define EP_SENDMSG_TIMEOUT(config) (config ? config->reconnectPeriod() : EP_DEFAULT_SENDMSG_TIMEOUT)

void EventSub::start()
{
    if (m_running)
    {
        EVENT_PUSH(INFO) << LOG_BADGE("start") << LOG_DESC("event sub is running");
        return;
    }
    m_running = true;

    m_timer = std::make_shared<boost::asio::deadline_timer>(boost::asio::make_strand(*m_ioc),
        boost::posix_time::milliseconds(EP_LOOP_PERIOD(m_config)));
    auto self = std::weak_ptr<EventSub>(shared_from_this());
    m_timer->async_wait([self](const boost::system::error_code&) {
        auto es = self.lock();
        if (!es)
        {
            return;
        }
        es->doLoop();
    });

    EVENT_PUSH(INFO) << LOG_BADGE("start") << LOG_DESC("start event sub successfully")
                     << LOG_KV("sendMsgTimeout", EP_SENDMSG_TIMEOUT(m_config))
                     << LOG_KV("reconnectPeriod", EP_LOOP_PERIOD(m_config));
}

void EventSub::stop()
{
    if (!m_running)
    {
        EVENT_PUSH(INFO) << LOG_BADGE("stop") << LOG_DESC("event sub is not running");
        return;
    }

    m_running = false;
    if (m_timer)
    {
        m_timer->cancel();
    }

    EVENT_PUSH(INFO) << LOG_BADGE("stop") << LOG_DESC("stop event sub successfully");
}

void EventSub::doLoop()
{
    if (m_suspendTasksCount.load() == 0)
    {
        return;
    }

    auto ss = m_wsService->sessions();
    if (ss.empty())
    {
        // EVENT_PUSH(INFO) << LOG_BADGE("doLoop") << LOG_DESC("no active sessions available");
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

        auto self = std::weak_ptr<EventSub>(shared_from_this());
        subscribeEventByTask(
            task, [id, self](bcos::Error::Ptr, const std::string&, const std::string&) {
                auto es = self.lock();
                if (!es)
                {
                    return;
                }

                es->removeWaitResp(id);
            });
    }
}

bool EventSub::addTask(EventSubTask::Ptr _task)
{
    std::unique_lock lock(x_tasks);
    removeSuspendTask(_task->id());
    if (m_workingTasks.find(_task->id()) == m_workingTasks.end())
    {
        m_workingTasks[_task->id()] = _task;
        return true;
    }

    return false;
}

EventSubTask::Ptr EventSub::getTask(const std::string& _id, bool includeSuspendTask)
{
    EventSubTask::Ptr task = nullptr;

    std::shared_lock lock(x_tasks);
    auto it = m_workingTasks.find(_id);
    if (it != m_workingTasks.end())
    {
        task = it->second;

        EVENT_PUSH(TRACE) << LOG_BADGE("getTask") << LOG_DESC("event sub task is working")
                          << LOG_KV("id", _id);
    }
    else if (includeSuspendTask)
    {
        auto innerIt = m_suspendTasks.find(_id);
        if (innerIt != m_suspendTasks.end())
        {
            task = innerIt->second;

            EVENT_PUSH(TRACE) << LOG_BADGE("getTask") << LOG_DESC("event sub task suspend")
                              << LOG_KV("id", _id);
        }
        else
        {
            EVENT_PUSH(DEBUG) << LOG_BADGE("getTask") << LOG_DESC("cannot found event sub task")
                              << LOG_KV("id", _id);
        }
    }

    return task;
}

EventSubTask::Ptr EventSub::getTaskAndRemove(const std::string& _id, bool includeSuspendTask)
{
    EventSubTask::Ptr task = nullptr;

    std::shared_lock lock(x_tasks);
    auto it = m_workingTasks.find(_id);
    if (it != m_workingTasks.end())
    {  // remove from m_workingTasks
        task = it->second;
        m_workingTasks.erase(it);

        EVENT_PUSH(TRACE) << LOG_BADGE("getTaskAndRemove") << LOG_DESC("event sub task is working")
                          << LOG_KV("id", _id);
    }
    else if (includeSuspendTask)
    {
        // remove from m_suspendTasks
        auto innerIt = m_suspendTasks.find(_id);
        if (innerIt != m_suspendTasks.end())
        {
            task = innerIt->second;
            m_suspendTasksCount--;
            m_suspendTasks.erase(innerIt);

            EVENT_PUSH(TRACE) << LOG_BADGE("getTaskAndRemove") << LOG_DESC("event sub task suspend")
                              << LOG_KV("id", _id);
        }
    }

    return task;
}

bool EventSub::removeWaitResp(const std::string& _id)
{
    std::unique_lock lock(x_tasks);
    return 0 != m_waitRespTasks.erase(_id);
}

bool EventSub::addSuspendTask(EventSubTask::Ptr _task)
{
    if (m_suspendTasks.find(_task->id()) == m_suspendTasks.end())
    {
        m_suspendTasksCount++;
        m_suspendTasks[_task->id()] = _task;
        return true;
    }

    return false;
}

bool EventSub::removeSuspendTask(const std::string& _id)
{
    // remove from suspendTasks
    auto it = m_suspendTasks.find(_id);
    if (it != m_suspendTasks.end())
    {
        m_suspendTasksCount--;
        m_suspendTasks.erase(it);
        return true;
    }
    return false;
}

std::size_t EventSub::suspendTasks(std::shared_ptr<WsSession> _session)
{
    std::size_t retCount = 0;
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

        // TODO : try to resend the task immediately is better ???
        EVENT_PUSH(INFO) << LOG_BADGE("suspendTasks")
                         << LOG_DESC("suspend event sub task for network disconnect")
                         << LOG_KV("id", task->id());

        it = m_workingTasks.erase(it);
        task->setSession(nullptr);
        addSuspendTask(task);
        retCount++;
    }

    return retCount;
}

void EventSub::onRecvEventSubMessage(
    std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session)
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
    auto resp = std::make_shared<EventSubResponse>();
    if (!resp->fromJson(strResp))
    {
        EVENT_PUSH(ERROR) << LOG_BADGE("onRecvEventSubMessage")
                          << LOG_DESC("recv invalid event sub message")
                          << LOG_KV("endpoint", _session->endPoint())
                          << LOG_KV("response", strResp);
        return;
    }

    EVENT_PUSH(DEBUG) << LOG_BADGE("onRecvEventSubMessage") << LOG_DESC("receive event sub message")
                      << LOG_KV("id", resp->id()) << LOG_KV("endpoint", _session->endPoint())
                      << LOG_KV("response", strResp);

    auto task = getTask(resp->id());
    if (task == nullptr)
    {
        EVENT_PUSH(ERROR) << LOG_BADGE("onRecvEventSubMessage")
                          << LOG_DESC("event sub task not exist") << LOG_KV("id", task->id())
                          << LOG_KV("endpoint", _session->endPoint())
                          << LOG_KV("response", strResp);
        return;
    }

    if (resp->status() == StatusCode::EndOfPush)
    {  // event sub end
        getTaskAndRemove(resp->id());
        task->callback()(nullptr, resp->id(), strResp);

        EVENT_PUSH(INFO) << LOG_BADGE("onRecvEventSubMessage") << LOG_DESC("end of push")
                         << LOG_KV("endpoint", _session->endPoint()) << LOG_KV("id", task->id())
                         << LOG_KV("response", strResp);
    }
    else if (resp->status() != StatusCode::Success)
    {  // event sub error
        getTaskAndRemove(resp->id());
        // normal event sub
        task->callback()(nullptr, resp->id(), strResp);

        EVENT_PUSH(INFO) << LOG_BADGE("onRecvEventSubMessage") << LOG_DESC("event sub error")
                         << LOG_KV("endpoint", _session->endPoint()) << LOG_KV("id", task->id())
                         << LOG_KV("response", strResp);
    }
    else
    {
        int64_t blockNumber = -1;
        // NOTE: update the latest blocknumber of event sub for network disconnect continue
        const auto& jResp = resp->jResp();
        if (jResp.isMember("result") && jResp["result"].isMember("blockNumber") &&
            jResp["result"]["blockNumber"].isInt64())
        {
            blockNumber = jResp["result"]["blockNumber"].asInt64();
            task->state()->setCurrentBlockNumber(blockNumber);
        }

        task->callback()(nullptr, resp->id(), strResp);

        EVENT_PUSH(TRACE) << LOG_BADGE("onRecvEventSubMessage") << LOG_DESC("event sub")
                          << LOG_KV("endpoint", _session->endPoint()) << LOG_KV("id", task->id())
                          << LOG_KV("blockNumber", blockNumber) << LOG_KV("response", strResp);
    }
}

void EventSub::subscribeEventByTask(EventSubTask::Ptr _task, Callback _callback)
{
    auto id = _task->id();
    auto group = _task->group();

    auto request = std::make_shared<EventSubSubRequest>();
    request->setId(id);
    request->setParams(_task->params());
    request->setGroup(_task->group());
    request->setState(_task->state());

    auto jsonReq = request->generateJson();

    auto message = m_messagefactory->buildMessage();
    message->setType(EventSubMessageType::EVENT_SUBSCRIBE);
    message->setData(std::make_shared<bcos::bytes>(jsonReq.begin(), jsonReq.end()));

    EVENT_PUSH(INFO) << LOG_BADGE("subscribeEventByTask") << LOG_DESC("subscribe event")
                     << LOG_KV("id", id) << LOG_KV("group", group) << LOG_KV("request", jsonReq);

    auto self = std::weak_ptr<EventSub>(shared_from_this());
    m_wsService->asyncSendMessage(message, Options(EP_SENDMSG_TIMEOUT(m_config)),
        [id, _task, _callback, self](bcos::Error::Ptr _error, std::shared_ptr<WsMessage> _msg,
            std::shared_ptr<WsSession> _session) {
            auto es = self.lock();
            if (!es)
            {
                return;
            }

            if (_error && _error->errorCode() != bcos::protocol::CommonError::SUCCESS)
            {
                EVENT_PUSH(ERROR) << LOG_BADGE("subscribeEventByTask")
                                  << LOG_DESC("callback response error") << LOG_KV("id", id)
                                  << LOG_KV("errorCode", _error->errorCode())
                                  << LOG_KV("errorMessage", _error->errorMessage());

                _callback(_error, id, "");
                return;
            }

            auto strResp = std::string(_msg->data()->begin(), _msg->data()->end());
            auto resp = std::make_shared<EventSubResponse>();
            if (!resp->fromJson(strResp))
            {
                EVENT_PUSH(ERROR) << LOG_BADGE("subscribeEventByTask")
                                  << LOG_DESC("invalid subscribe event response")
                                  << LOG_KV("id", id) << LOG_KV("response", strResp);
                _callback(nullptr, id, strResp);
            }
            else if (resp->status() != StatusCode::Success)
            {
                _callback(nullptr, id, strResp);
                EVENT_PUSH(ERROR) << LOG_BADGE("subscribeEventByTask")
                                  << LOG_DESC("callback response error") << LOG_KV("id", id)
                                  << LOG_KV("response", strResp);
            }
            else
            {
                // subscribe event successfully, set network session for unsubscribe
                _task->setSession(_session);

                es->addTask(_task);

                _callback(nullptr, id, strResp);
                EVENT_PUSH(INFO) << LOG_BADGE("subscribeEventByTask")
                                 << LOG_DESC("callback response success") << LOG_KV("id", id)
                                 << LOG_KV("response", strResp);
            }
        });
}

void EventSub::subscribeEvent(
    const std::string& _group, EventSubParams::ConstPtr _params, Callback _callback)
{
    auto task = std::make_shared<EventSubTask>();
    task->setId(m_messagefactory->newSeq());
    task->setGroup(_group);
    task->setParams(_params);
    task->setCallback(_callback);
    task->setState(std::make_shared<EventSubTaskState>());

    return subscribeEventByTask(task, _callback);
}

void EventSub::unsubscribeEvent(const std::string& _id, Callback _callback)
{
    auto task = getTaskAndRemove(_id);
    if (task == nullptr)
    {
        // TODO: error code define
        auto error = std::make_shared<Error>(-1, "event sub task not found");
        _callback(error, _id, "");
        EVENT_PUSH(ERROR) << LOG_BADGE("unsubscribeEvent") << LOG_DESC("event sub task not found")
                          << LOG_KV("id", _id);
        return;
    }

    auto session = task->session();
    if (!session)
    {
        _callback(nullptr, _id, "");
        EVENT_PUSH(INFO) << LOG_BADGE("unsubscribeEvent") << LOG_DESC("task is suspend")
                         << LOG_KV("id", _id);
        return;
    }

    auto request = std::make_shared<EventSubUnsubRequest>();
    request->setId(_id);
    request->setGroup(task->group());
    auto strReq = request->generateJson();

    auto message = m_messagefactory->buildMessage();
    message->setType(EventSubMessageType::EVENT_UNSUBSCRIBE);
    message->setData(std::make_shared<bcos::bytes>(strReq.begin(), strReq.end()));

    session->asyncSendMessage(message, Options(EP_SENDMSG_TIMEOUT(m_config)),
        [_id, _callback](
            bcos::Error::Ptr _error, std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession>) {
            if (_error && _error->errorCode() != bcos::protocol::CommonError::SUCCESS)
            {
                EVENT_PUSH(ERROR) << LOG_BADGE("unsubscribeEvent")
                                  << LOG_DESC("callback response error") << LOG_KV("id", _id)
                                  << LOG_KV("errorCode", _error->errorCode())
                                  << LOG_KV("errorMessage", _error->errorMessage());

                _callback(_error, _id, "");
                return;
            }

            auto strResp = std::string(_msg->data()->begin(), _msg->data()->end());
            auto resp = std::make_shared<EventSubResponse>();
            if (!resp->fromJson(strResp))
            {
                EVENT_PUSH(ERROR) << LOG_BADGE("unsubscribeEvent")
                                  << LOG_DESC("callback invalid response") << LOG_KV("id", _id)
                                  << LOG_KV("response", strResp);

                // TODO:
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