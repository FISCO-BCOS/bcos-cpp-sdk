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
 * @file EvenRequest.h
 * @author: octopus
 * @date 2021-09-01
 */

#pragma once
#include <bcos-cpp-sdk/event/EventParams.h>

namespace bcos
{
namespace cppsdk
{
namespace event
{
class EventRequest
{
public:
    using Ptr = std::shared_ptr<EventRequest>;

public:
    void setId(const std::string& _id) { m_id = _id; }
    std::string id() const { return m_id; }

    void setGroup(const std::string& _group) { m_group = _group; }
    std::string group() const { return m_group; }

    void setParams(std::shared_ptr<EventParams> _params) { m_params = _params; }
    std::shared_ptr<EventParams> params() const { return m_params; }

    std::string generateJson() const;
    bool initFromJson(const std::string& _request);

private:
    std::string m_id;
    std::string m_group;
    std::shared_ptr<EventParams> m_params;
};

}  // namespace event
}  // namespace cppsdk
}  // namespace bcos