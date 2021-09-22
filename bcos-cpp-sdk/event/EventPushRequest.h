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
 * @file EvenPushRequest.h
 * @author: octopus
 * @date 2021-09-01
 */

#pragma once
#include <bcos-cpp-sdk/event/EventPushParams.h>
#include <bcos-cpp-sdk/event/EventPushTask.h>

namespace bcos
{
namespace cppsdk
{
namespace event
{
class EventPushUnsubRequest
{
public:
    using Ptr = std::shared_ptr<EventPushUnsubRequest>;

    virtual ~EventPushUnsubRequest() {}

public:
    void setId(const std::string& _id) { m_id = _id; }
    std::string id() const { return m_id; }

    void setGroup(const std::string& _group) { m_group = _group; }
    std::string group() const { return m_group; }

    virtual std::string generateJson() const;
    virtual bool fromJson(const std::string& _request);

private:
    std::string m_id;
    std::string m_group;
};

class EventPushSubRequest : public EventPushUnsubRequest
{
public:
    using Ptr = std::shared_ptr<EventPushSubRequest>;

    virtual ~EventPushSubRequest() {}

public:
    void setParams(std::shared_ptr<EventPushParams> _params) { m_params = _params; }
    std::shared_ptr<EventPushParams> params() const { return m_params; }

    void setState(std::shared_ptr<EventPushTaskState> _state) { m_state = _state; }
    std::shared_ptr<EventPushTaskState> state() const { return m_state; }

    std::string generateJson() const override;
    bool fromJson(const std::string& _request) override;

private:
    std::shared_ptr<EventPushParams> m_params;
    std::shared_ptr<EventPushTaskState> m_state;
};

}  // namespace event
}  // namespace cppsdk
}  // namespace bcos