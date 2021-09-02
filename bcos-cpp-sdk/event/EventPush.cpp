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

#include <bcos-cpp-sdk/event/EventPush.h>
#include <bcos-framework/interfaces/protocol/CommonError.h>
#include <bcos-framework/libutilities/Common.h>
#include <boost/core/ignore_unused.hpp>
#include <memory>
#include <mutex>
#include <shared_mutex>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::event;

void EventPush::start()
{
    if (m_running)
    {
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
}
void EventPush::stop()
{
    if (!m_running)
    {
        return;
    }

    m_running = false;
    if (m_timer)
    {
        m_timer->cancel();
    }
}
void EventPush::doLoop()
{
    auto ss = m_wsService->sessions();
    if (ss.empty())
    {
        // no active sessions available
        return;
    }

    std::shared_lock lock(x_tasks);
    for (const auto& taskEntry : m_interruptTasks)
    {
        auto task = taskEntry.second;
        std::string id = task->id();
        if (m_waitRespTasks.find(id) != m_waitRespTasks.end())
        {
            continue;
        }

        // send this task to remote and put it to m_waitRespTasks
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

        it = m_tasks.erase(it);
        m_interruptTasks[task->id()] = task;
    }
}

void EventPush::subscribeEvent(EventParams::Ptr _params, Callback _callback)
{
    auto id = m_factory->newSeq();
    auto message = m_factory->buildMessage();
    message->setType(ws::WsMessageType::EVENT_SUBSCRIBE);
    // TODO: encode params
    auto self = std::weak_ptr<EventPush>(shared_from_this());
    m_wsService->asyncSendMessage(message, ws::Options(-1),
        [id, _params, _callback, self](bcos::Error::Ptr _error, std::shared_ptr<ws::WsMessage> _msg,
            std::shared_ptr<ws::WsSession> _session) {
            boost::ignore_unused(_error, _msg, _session);
            if (_error && _error->errorCode() != bcos::protocol::CommonError::SUCCESS)
            {
                // TODO: error handler
                return;
            }

            auto ep = self.lock();
            if (!ep)
            {
                return;
            }

            // TODO: check if subscribe successfully
            // if the operation is timeout , unsubscribe the event
            //

            auto task = std::make_shared<EventPushTask>();
            task->setId(id);
            task->setParams(_params);
            task->setSession(_session);
            ep->addTask(id, task);
        });
}

void EventPush::unsubscribeEvent(const std::string& _id)
{
    auto task = getTask(_id);
    if (task == nullptr)
    {
        // TODO: add return code
        return;
    }

    auto message = m_factory->buildMessage();
    message->setType(ws::WsMessageType::EVENT_UNSUBSCRIBE);
    std::string params = "{\"id\":" + _id + "}";
    message->setData(std::make_shared<bcos::bytes>(params.begin(), params.end()));
    auto session = task->session();
    session->asyncSendMessage(message);
}