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
 * @file AMOPRequest.h
 * @author: octopus
 * @date 2021-08-23
 */
#pragma once
#include <bcos-framework/libutilities/Common.h>

namespace bcos
{
namespace cppsdk
{
namespace amop
{
class AMOPRequest
{
public:
    virtual ~AMOPRequest() {}

    // topic field length
    const static size_t TOPIC_MAX_LENGTH = 65535;
    const static size_t MESSAGE_MIN_LENGTH = 2;
    AMOPRequest() { m_data = bytesConstRef(); }
    using Ptr = std::shared_ptr<AMOPRequest>;

public:
    std::string topic() const { return m_topic; }
    void setTopic(const std::string& _topic) { m_topic = _topic; }
    void setData(bytesConstRef _data) { m_data = _data; }
    bytesConstRef data() const { return m_data; }

public:
    virtual bool encode(bcos::bytes& _buffer);
    virtual ssize_t decode(bytesConstRef _data);

private:
    std::string m_topic;
    bytesConstRef m_data;
};

class AMOPRequestFactory
{
public:
    using Ptr = std::shared_ptr<AMOPRequestFactory>;
    virtual ~AMOPRequestFactory() {}

public:
    virtual std::shared_ptr<AMOPRequest> buildRequest()
    {
        auto msg = std::make_shared<AMOPRequest>();
        return msg;
    }
};

}  // namespace amop
}  // namespace cppsdk
}  // namespace bcos
