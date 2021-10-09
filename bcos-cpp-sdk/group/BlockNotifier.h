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
 * @file BlockNotifier.h
 * @author: octopus
 * @date 2021-10-04
 */

#pragma once

#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace bcos
{
namespace cppsdk
{
namespace group
{
using BlockNotifierCallback = std::function<void(int64_t _blockNumber)>;
using BlockNotifierCallbacks = std::vector<BlockNotifierCallback>;

class BlockInfo
{
public:
    using Ptr = std::shared_ptr<BlockInfo>;
    using ConstPtr = std::shared_ptr<const BlockInfo>;

public:
    std::string group() const { return m_group; }
    void setGroup(const std::string& _group) { m_group = _group; }
    int64_t blockNumber() const { return m_blockNumber; }
    void setBlockNumber(int64_t _blockNumber) { m_blockNumber = _blockNumber; }

public:
    std::string toJson();
    bool fromJson(const std::string& _json);

private:
    std::string m_group;
    int64_t m_blockNumber;
};

class BlockNotifier
{
public:
    using Ptr = std::shared_ptr<BlockNotifier>;
    using ConstPtr = std::shared_ptr<const BlockNotifier>;

public:
    void onRecvBlockNotifier(const std::string& _msg);
    void onRecvBlockNotifier(BlockInfo::Ptr _blockInfo);

    bool getBlockNumberByGroup(const std::string& _group, int64_t& _blockNumber);
    void registerCallback(const std::string& _group, BlockNotifierCallback _callback);
    void removeGroup(const std::string& _group);

private:
    mutable std::shared_mutex x_locks;
    std::unordered_map<std::string, BlockNotifierCallbacks> m_group2callbacks;
    std::unordered_map<std::string, BlockInfo::Ptr> m_group2BlockInfo;
};
}  // namespace group
}  // namespace cppsdk
}  // namespace bcos
