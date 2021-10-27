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
 * @file ProtocolVersion.h
 * @author: octopus
 * @date 2021-10-26
 */

#include <bcos-cpp-sdk/ws/Common.h>
#include <bcos-cpp-sdk/ws/ProtocolVersion.h>
#include <json/json.h>
#include <json/value.h>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::service;

bool ProtocolVersion::fromJson(const std::string& _json)
{
    try
    {
        Json::Value root;
        Json::Reader jsonReader;
        std::string errorMessage;

        auto groupInfoFactory = std::make_shared<group::GroupInfoFactory>();
        auto chainNodeInfoFactory = std::make_shared<group::ChainNodeInfoFactory>();
        do
        {
            if (!jsonReader.parse(_json, root))
            {
                errorMessage = "invalid json object";
                break;
            }

            if (!root.isMember("protocolVersion"))
            {  // id field not exist
                errorMessage = "Cannot find \'protocolVersion\' field";
                break;
            }

            int protocolVersion = root["protocolVersion"].asInt();
            setProtocolVersion(protocolVersion);

            if (root.isMember("groupInfoList") && root["groupInfoList"].isArray())
            {
                auto& jGroupInfoList = root["groupInfoList"];
                for (Json::ArrayIndex i = 0; i < jGroupInfoList.size(); ++i)
                {
                    auto groupInfo = groupInfoFactory->createGroupInfo();
                    groupInfo->setChainNodeInfoFactory(chainNodeInfoFactory);

                    Json::FastWriter writer;
                    std::string str = writer.write(jGroupInfoList[i]);
                    groupInfo->deserialize(str);
                    m_groupInfoList.push_back(groupInfo);
                }
            }

            RPC_WS_LOG(INFO) << LOG_BADGE("fromJson") << LOG_DESC("parser protocol version")
                             << LOG_KV("protocolVersion", protocolVersion)
                             << LOG_KV("groupInfoList size", m_groupInfoList.size());

            return true;

        } while (0);

        RPC_WS_LOG(ERROR) << LOG_BADGE("fromJson")
                          << LOG_DESC("invalid protocol version json string")
                          << LOG_KV("json", _json) << LOG_KV("error", errorMessage);
    }
    catch (const std::exception& e)
    {
        RPC_WS_LOG(ERROR) << LOG_BADGE("fromJson")
                          << LOG_DESC("invalid protocol version json string")
                          << LOG_KV("json", _json) << LOG_KV("exception", e.what());
    }

    return false;
}

Json::Value ProtocolVersion::toJson()
{
    /*
       {
       "protocolVersion": 1,
       "groupInfoList": []
       }
   */
    Json::Value jResult;

    jResult["protocolVersion"] = m_protocolVersion;
    jResult["groupInfoList"] = Json::Value(Json::arrayValue);

    for (const auto& groupInfo : m_groupInfoList)
    {
        jResult["groupInfoList"].append(groupInfo->serialize());
    }

    return jResult;
}

std::string ProtocolVersion::toJsonString()
{
    Json::Value jResult = toJson();

    Json::FastWriter writer;
    std::string result = writer.write(jResult);
    RPC_WS_LOG(ERROR) << LOG_BADGE("toJson") << LOG_DESC("generator protocol version string")
                      << LOG_KV("json", result);
    return result;
}