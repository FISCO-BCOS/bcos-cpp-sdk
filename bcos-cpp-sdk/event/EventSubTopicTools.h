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
 * @file EvenSubTopicTools.h
 * @author: octopus
 * @date 2021-09-01
 */

#pragma once
#include <bcos-boostssl/utilities/Common.h>
#include <bcos-cpp-sdk/event/Common.h>
#include <json/value.h>
#include <set>
#include <string>
#include <vector>

#define TOPIC_LENGTH (64)

namespace bcos
{
namespace cppsdk
{
namespace event
{
class EvenSubTopicTools
{
public:
    using Ptr = std::shared_ptr<EvenSubTopicTools>;
    using ConstPtr = std::shared_ptr<const EvenSubTopicTools>;

public:
    static bool validTopic(const std::string& _topic)
    {
        if ((_topic.compare(0, 2, "0x") == 0) || (_topic.compare(0, 2, "0X") == 0))
        {
            return _topic.length() == (TOPIC_LENGTH + 2);
        }

        return _topic.length() == TOPIC_LENGTH;
    }

public:
    // TODO: toTopic impl, sm or none sm

    // uint => topic
    static std::string u256ToTopic(bcos::boostssl::utilities::u256 _u)
    {
        (void)_u;
        return std::string("");
    }

    // int => topic
    static std::string i256ToTopic(bcos::boostssl::utilities::s256 _i)
    {
        (void)_i;
        return std::string("");
    }

    // string => topic
    static std::string stringToTopic(const std::string& _str)
    {
        (void)_str;
        return std::string("");
    }

    // bytes => topic
    static std::string bytesToTopic(const bcos::boostssl::utilities::bytes& _bs)
    {
        (void)_bs;
        return std::string("");
    }

    // bytesN(eg:bytes32) => topic
    static std::string bytesNToTopic(const bcos::boostssl::utilities::bytes& _bsn)
    {
        (void)_bsn;
        return std::string("");
    }
};

}  // namespace event
}  // namespace cppsdk
}  // namespace bcos
