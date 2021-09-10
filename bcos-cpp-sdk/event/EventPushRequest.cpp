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
 * @file EvenPushRequest.cpp
 * @author: octopus
 * @date 2021-09-03
 */

#include <bcos-cpp-sdk/event/Common.h>
#include <bcos-cpp-sdk/event/EventPushRequest.h>

#include <json/json.h>
#include <exception>
#include <memory>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::event;

std::string EventPushRequest::generateJson() const
{
    /*
    {
    "id": "",
    "group": ""
    }
    */
    /*
    {
    "id": "",
    "group": "",
    "params": {
        "fromBlock": -1,
        "toBlock": -1,
        "addresses": [
        "0xca5ed56862869c25da0bdf186e634aac6c6361ee"
        ],
        "topics": [
        "0x91c95f04198617c60eaf2180fbca88fc192db379657df0e412a9f7dd4ebbe95d"
        ]
        }
    }
    */

    Json::Value jResult;
    // id
    jResult["id"] = m_id;
    // group
    jResult["group"] = m_group;

    if (!m_params)
    {
        Json::FastWriter writer;
        std::string result = writer.write(jResult);
        return result;
    }

    Json::Value jParams;
    // fromBlock
    jParams["fromBlock"] =
        m_state->currentBlockNumber() > 0 ? m_state->currentBlockNumber() : m_params->fromBlock();
    // toBlock
    jParams["toBlock"] = m_params->toBlock();
    // addresses
    Json::Value jAddresses(Json::arrayValue);
    for (const auto& addr : m_params->addresses())
    {
        jAddresses.append(addr);
    }
    jParams["addresses"] = jAddresses;
    // topics
    Json::Value jTopics(Json::arrayValue);
    for (const auto& inTopics : m_params->topics())
    {
        if (!inTopics.empty())
        {
            Json::Value jInTopics(Json::arrayValue);
            for (const auto& topic : inTopics)
            {
                jInTopics.append(topic);
            }
            jTopics.append(jInTopics);
        }
        else
        {
            Json::Value jInTopics(Json::nullValue);
            jTopics.append(jInTopics);
        }
    }
    jParams["topics"] = jTopics;
    // params
    jResult["params"] = jParams;

    Json::FastWriter writer;
    std::string result = writer.write(jResult);
    return result;
}

bool EventPushRequest::fromJson(const std::string& _request)
{
    std::string id;
    std::string group;
    EventPushParams::Ptr params = std::make_shared<EventPushParams>();

    try
    {
        Json::Value root;
        Json::Reader jsonReader;
        std::string errorMessage;
        do
        {
            if (!jsonReader.parse(_request, root))
            {
                errorMessage = "invalid json object, parse request failed";
                break;
            }

            if (!root.isMember("id"))
            {  // id field not exist
                errorMessage = "\'id\' field not exist";
                break;
            }
            id = root["id"].asString();

            if (!root.isMember("group"))
            {
                // group field not exist
                errorMessage = "\'group\' field not exist";
                break;
            }
            group = root["group"].asString();

            if (!root.isMember("params"))
            {  // params field not exist
                errorMessage = "\'params\' field not exist";
                break;
            }

            auto& jParams = root["params"];
            if (jParams.isMember("fromBlock"))
            {
                params->setFromBlock(jParams["fromBlock"].asInt64());
            }

            if (jParams.isMember("toBlock"))
            {
                params->setToBlock(jParams["toBlock"].asInt64());
            }

            if (jParams.isMember("addresses"))
            {
                auto& jAddresses = jParams["addresses"];
                for (Json::Value::ArrayIndex index = 0; index < jAddresses.size(); ++index)
                {
                    params->addAddress(jAddresses[index].asString());
                }
            }

            if (jParams.isMember("topics"))
            {
                auto& jTopics = jParams["topics"];

                for (Json::Value::ArrayIndex index = 0; index < jTopics.size(); ++index)
                {
                    auto& jIndex = jParams[index];
                    if (jIndex.isNull())
                    {
                        continue;
                    }

                    if (jIndex.isArray())
                    {  // array topics
                        for (Json::Value::ArrayIndex innerIndex = 0; innerIndex < jIndex.size();
                             ++innerIndex)
                        {
                            params->addTopic(index, jIndex[innerIndex].asString());
                        }
                    }
                    else
                    {  // single topic, string value
                        params->addTopic(index, jIndex.asString());
                    }
                }
            }

            m_id = id;
            m_group = group;
            m_params = params;

            EVENT_REQUEST(INFO) << LOG_BADGE("fromJson")
                                << LOG_DESC("parse event push request success")
                                << LOG_KV("group", m_group) << LOG_KV("id", m_id);

            return true;

        } while (0);

        EVENT_REQUEST(ERROR) << LOG_BADGE("fromJson") << LOG_DESC("invalid event push request")
                             << LOG_KV("request", _request) << LOG_KV("errorMessage", errorMessage);
    }
    catch (const std::exception& e)
    {
        EVENT_REQUEST(ERROR) << LOG_BADGE("fromJson") << LOG_DESC("invalid json object")

                             << LOG_KV("request", _request)
                             << LOG_KV("error", std::string(e.what()));
    }

    return false;
}
