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
 * @file EvenPush.h
 * @author: octopus
 * @date 2021-09-01
 */

#pragma once
#include <bcos-boostssl/websocket/WsConfig.h>
#include <bcos-boostssl/websocket/WsService.h>
#include <bcos-cpp-sdk/event/EventSubInterface.h>
#include <bcos-cpp-sdk/event/EventSubTask.h>
#include <atomic>
#include <shared_mutex>
#include <utility>

namespace bcos
{
namespace cppsdk
{
namespace event
{
class EventSub : public EventSubInterface, public std::enable_shared_from_this<EventSub>
{
public:
    using Ptr = std::shared_ptr<EventSub>;
    EventSub() {}
    virtual ~EventSub() { stop(); }

public:
    virtual void start() override;
    virtual void stop() override;

    virtual void subscribeEvent(
        const std::string& _group, EventSubParams::ConstPtr _params, Callback _callback) override;
    virtual void unsubscribeEvent(const std::string& _id, Callback _callback) override;

public:
    void doLoop();

public:
    void subscribeEventByTask(EventSubTask::Ptr _task, Callback _callback);
    void onRecvEventSubMessage(std::shared_ptr<bcos::boostssl::ws::WsMessage> _msg,
        std::shared_ptr<bcos::boostssl::ws::WsSession> _session);

public:
    bool addTask(EventSubTask::Ptr _task);
    EventSubTask::Ptr getTask(const std::string& _id, bool includeSuspendTask = true);
    EventSubTask::Ptr getTaskAndRemove(const std::string& _id, bool includeSuspendTask = true);
    bool removeWaitResp(const std::string& _id);
    bool addSuspendTask(EventSubTask::Ptr _task);
    bool removeSuspendTask(const std::string& _id);

    std::size_t suspendTasks(std::shared_ptr<bcos::boostssl::ws::WsSession> _session);

    void setWsService(bcos::boostssl::ws::WsService::Ptr _wsService) { m_wsService = _wsService; }
    bcos::boostssl::ws::WsService::Ptr wsService() const { return m_wsService; }

    void setIoc(std::shared_ptr<boost::asio::io_context> _ioc) { m_ioc = _ioc; }
    std::shared_ptr<boost::asio::io_context> ioc() const { return m_ioc; }

    void setMessageFactory(std::shared_ptr<bcos::boostssl::ws::WsMessageFactory> _messageFactory)
    {
        m_messagefactory = _messageFactory;
    }
    std::shared_ptr<bcos::boostssl::ws::WsMessageFactory> messageFactory() const
    {
        return m_messagefactory;
    }

    boostssl::ws::WsConfig::ConstPtr config() const { return m_config; }
    void setConfig(boostssl::ws::WsConfig::ConstPtr _config) { m_config = _config; }

    uint32_t suspendTasksCount() const { return m_suspendTasksCount.load(); }
    const std::unordered_map<std::string, EventSubTask::Ptr>& suspendTasks() const
    {
        return m_suspendTasks;
    }
    const std::unordered_map<std::string, EventSubTask::Ptr>& workingtasks() const
    {
        return m_workingTasks;
    }

private:
    bool m_running = false;
    mutable std::shared_mutex x_tasks;
    std::unordered_map<std::string, EventSubTask::Ptr> m_workingTasks;
    std::atomic<uint32_t> m_suspendTasksCount{0};
    std::unordered_map<std::string, EventSubTask::Ptr> m_suspendTasks;
    std::set<std::string> m_waitRespTasks;
    // timer
    std::shared_ptr<boost::asio::deadline_timer> m_timer;
    // io context
    std::shared_ptr<boost::asio::io_context> m_ioc;
    // message factory
    std::shared_ptr<bcos::boostssl::ws::WsMessageFactory> m_messagefactory;
    // websocket service
    bcos::boostssl::ws::WsService::Ptr m_wsService;
    //
    boostssl::ws::WsConfig::ConstPtr m_config;
};
}  // namespace event
}  // namespace cppsdk
}  // namespace bcos