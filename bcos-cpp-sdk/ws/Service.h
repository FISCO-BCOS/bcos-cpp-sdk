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
 * @file Service.h
 * @author: octopus
 * @date 2021-10-22
 */

#pragma once
#include <bcos-boostssl/utilities/Common.h>
#include <bcos-boostssl/utilities/Error.h>
#include <bcos-boostssl/websocket/WsService.h>
#include <bcos-cpp-sdk/multigroup/GroupInfo.h>
#include <bcos-cpp-sdk/multigroup/GroupInfoFactory.h>
#include <bcos-cpp-sdk/ws/BlockNumberInfo.h>
#include <functional>
#include <set>
#include <unordered_map>

namespace bcos
{
namespace cppsdk
{
namespace service
{
using BlockNotifierCallback = std::function<void(const std::string& _group, int64_t _blockNumber)>;
using BlockNotifierCallbacks = std::vector<BlockNotifierCallback>;

class Service : public bcos::boostssl::ws::WsService
{
public:
    using Ptr = std::shared_ptr<Service>;
    using ConstPtr = std::shared_ptr<const Service>;

public:
    // ---------------------overide begin------------------------------------

    virtual void start() override;
    virtual void stop() override;

    virtual void onConnect(bcos::boostssl::utilities::Error::Ptr _error,
        std::shared_ptr<bcos::boostssl::ws::WsSession> _session) override;

    virtual void onDisconnect(bcos::boostssl::utilities::Error::Ptr _error,
        std::shared_ptr<bcos::boostssl::ws::WsSession> _session) override;

    virtual void onRecvMessage(std::shared_ptr<bcos::boostssl::ws::WsMessage> _msg,
        std::shared_ptr<bcos::boostssl::ws::WsSession> _session) override;
    // ---------------------overide end -------------------------------------

    void waitForConnectionEstablish();

public:
    // ---------------------send message begin-------------------------------
    virtual void asyncSendMessageByGroupAndNode(const std::string& _group, const std::string& _node,
        std::shared_ptr<bcos::boostssl::ws::WsMessage> _msg, bcos::boostssl::ws::Options _options,
        bcos::boostssl::ws::RespCallBack _respFunc);
    // ---------------------oversend message begin----------------------------

public:
    virtual void startHandshake(std::shared_ptr<bcos::boostssl::ws::WsSession> _session);
    virtual bool checkHandshakeDone(std::shared_ptr<bcos::boostssl::ws::WsSession> _session);

    void clearGroupInfoByEp(const std::string& _endPoint);
    void clearGroupInfoByEp(const std::string& _endPoint, const std::string& _groupID);
    void updateGroupInfoByEp(const std::string& _endPoint, bcos::group::GroupInfo::Ptr _groupInfo);
    void onNotifyGroupInfo(
        const std::string& _groupInfo, std::shared_ptr<bcos::boostssl::ws::WsSession> _session);
    void onNotifyGroupInfo(std::shared_ptr<bcos::boostssl::ws::WsMessage> _msg,
        std::shared_ptr<bcos::boostssl::ws::WsSession> _session);

public:
    //------------------------------ Block Notifier begin --------------------------
    bool getBlockNumber(const std::string& _group, int64_t& _blockNumber);

    void onRecvBlockNotifier(const std::string& _msg);
    void onRecvBlockNotifier(BlockNumberInfo::Ptr _blockNumberInfo);
    void removeBlockNumberInfo(const std::string& _group);

    void registerBlockNumberNotifier(const std::string& _group, BlockNotifierCallback _callback);
    //------------------------------ Block Notifier end  ----------------------------

public:
    bcos::group::GroupInfo::Ptr getGroupInfo(const std::string& _groupID);
    void updateGroupInfo(const std::string& _endPoint, bcos::group::GroupInfo::Ptr _groupInfo);

public:
    bool getEndPointsByGroup(const std::string& _group, std::set<std::string>& _endPoints);
    bool getEndPointsByGroupAndNode(
        const std::string& _group, const std::string& _node, std::set<std::string>& _endPoints);

    void printGroupInfo();
    bcos::group::GroupInfoFactory::Ptr groupInfoFactory() const { return m_groupInfoFactory; }
    void setGroupInfoFactory(bcos::group::GroupInfoFactory::Ptr _groupInfoFactory)
    {
        m_groupInfoFactory = _groupInfoFactory;
    }

    bcos::group::ChainNodeInfoFactory::Ptr chainNodeInfoFactory() const
    {
        return m_chainNodeInfoFactory;
    }

    void setChainNodeInfoFactory(bcos::group::ChainNodeInfoFactory::Ptr _chainNodeInfoFactory)
    {
        m_chainNodeInfoFactory = _chainNodeInfoFactory;
    }

public:
    uint32_t wsHandshakeTimeout() const { return m_wsHandshakeTimeout; }
    void setWsHandshakeTimeout(uint32_t _wsHandshakeTimeout)
    {
        m_wsHandshakeTimeout = _wsHandshakeTimeout;
    }

    uint32_t handshakeSucCount() const { return m_handshakeSucCount.load(); }

    void increaseHandshakeSucCount() { m_handshakeSucCount++; }

private:
    uint32_t m_wsHandshakeTimeout = 10000;  // 10s

    std::atomic<uint32_t> m_handshakeSucCount = 0;

private:
    mutable boost::shared_mutex x_lock;
    // group => node => endpoints
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>>
        m_group2Node2Endpoints;

    // endpoint => group => groupInfo
    std::unordered_map<std::string, std::unordered_map<std::string, bcos::group::GroupInfo::Ptr>>
        m_endPoint2GroupId2GroupInfo;
    //
    bcos::group::GroupInfoFactory::Ptr m_groupInfoFactory;
    //
    bcos::group::ChainNodeInfoFactory::Ptr m_chainNodeInfoFactory;

    mutable boost::shared_mutex x_blockNotifierLock;
    // group => blockNotifier
    std::unordered_map<std::string, BlockNotifierCallbacks> m_group2callbacks;
    std::unordered_map<std::string, BlockNumberInfo::Ptr> m_group2BlockNumber;
};

}  // namespace service
}  // namespace cppsdk
}  // namespace bcos
