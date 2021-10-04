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
 * @file BlockNotifier.cpp
 * @author: octopus
 * @date 2021-10-04
 */

#include <bcos-cpp-sdk/rpc/BlockNotifier.h>
#include <bcos-cpp-sdk/rpc/Common.h>
#include <json/json.h>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::jsonrpc;

std::string BlockInfo::toJson()
{
    Json::Value jValue;
    jValue["group"] = m_group;
    jValue["blockNumber"] = m_blockNumber;

    Json::FastWriter writer;
    std::string s = writer.write(jValue);
    return s;
}

bool BlockInfo::fromJson(const std::string& _json)
{
    std::string errorMessage;
    try
    {
        std::string group;
        int64_t blockNumber = -1;
        do
        {
            Json::Value root;
            Json::Reader jsonReader;
            if (!jsonReader.parse(_json, root))
            {
                errorMessage = "invalid json object";
                break;
            }

            if (!root.isMember("blockNumber"))
            {
                errorMessage = "request has no blockNumber field";
                break;
            }
            blockNumber = root["blockNumber"].asInt64();

            if (root.isMember("group"))
            {
                group = root["group"].asString();
            }

            m_blockNumber = blockNumber;
            m_group = group;

            RPC_BLOCKNOTIFIER_LOG(INFO) << LOG_BADGE("fromJson") << LOG_KV("group", m_group)
                                        << LOG_KV("blockNumber", m_blockNumber);

            return true;

        } while (0);

        RPC_BLOCKNOTIFIER_LOG(ERROR) << LOG_BADGE("fromJson") << LOG_DESC("Invalid JSON")
                                     << LOG_KV("errorMessage", errorMessage);
    }
    catch (const std::exception& e)
    {
        RPC_BLOCKNOTIFIER_LOG(ERROR) << LOG_BADGE("fromJson") << LOG_DESC("Invalid JSON")
                                     << LOG_KV("error", std::string(e.what()));
    }

    return false;
}

void BlockNotifier::onRecvBlockNotifier(const std::string& _msg)
{
    auto bi = std::make_shared<BlockInfo>();
    auto r = bi->fromJson(_msg);
    if (!r)
    {
        onRecvBlockNotifier(bi);
    }
}

void BlockNotifier::onRecvBlockNotifier(BlockInfo::Ptr _blockInfo)
{
    RPC_BLOCKNOTIFIER_LOG(INFO) << LOG_BADGE("onRecvBlockNotifier")
                                << LOG_DESC("update blockNumber of group")
                                << LOG_KV("group", _blockInfo->group())
                                << LOG_KV("blockNumber", _blockInfo->blockNumber());

    {  // callback
        std::shared_lock lock(x_locks);
        auto it = m_group2callbacks.find(_blockInfo->group());
        if (it != m_group2callbacks.end())
        {
            for (auto& callback : it->second)
            {
                callback(_blockInfo->blockNumber());
            }
        }
    }

    {  // update blockinfo
        std::unique_lock lock(x_locks);
        auto it = m_group2BlockInfo.find(_blockInfo->group());
        if (it != m_group2BlockInfo.end())
        {
            it->second->setBlockNumber(_blockInfo->blockNumber());
        }
        else
        {
            m_group2BlockInfo[_blockInfo->group()] = _blockInfo;
        }
    }
}

bool BlockNotifier::getBlockNumberByGroup(const std::string& _group, int64_t& _blockNumber)
{
    std::shared_lock lock(x_locks);
    auto it = m_group2BlockInfo.find(_group);
    if (it != m_group2BlockInfo.end())
    {
        _blockNumber = it->second->blockNumber();
        return true;
    }
    return false;
}

void BlockNotifier::registerCallback(const std::string& _group, BlockNotifierCallback _callback)
{
    RPC_BLOCKNOTIFIER_LOG(INFO) << LOG_BADGE("registerCallback") << LOG_KV("group", _group);
    std::unique_lock lock(x_locks);
    m_group2callbacks[_group].push_back(_callback);
}

void BlockNotifier::removeGroup(const std::string& _group)
{
    RPC_BLOCKNOTIFIER_LOG(INFO) << LOG_BADGE("removeGroup") << LOG_KV("group", _group);
    std::unique_lock lock(x_locks);
    m_group2callbacks.erase(_group);
    m_group2BlockInfo.erase(_group);
}
