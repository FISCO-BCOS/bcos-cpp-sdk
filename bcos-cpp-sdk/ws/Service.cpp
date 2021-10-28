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
 * @file Service.cpp
 * @author: octopus
 * @date 2021-10-22
 */

#include <bcos-boostssl/websocket/WsError.h>
#include <bcos-cpp-sdk/ws/Common.h>
#include <bcos-cpp-sdk/ws/ProtocolVersion.h>
#include <bcos-cpp-sdk/ws/Service.h>
#include <bcos-framework/interfaces/multigroup/GroupInfo.h>
#include <bcos-framework/interfaces/protocol/CommonError.h>
#include <bcos-framework/libutilities/Common.h>
#include <memory>
#include <shared_mutex>
#include <type_traits>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::service;
using namespace bcos::boostssl;
using namespace bcos::boostssl::ws;

// ---------------------overide begin--------------------------------------------------------------

void Service::start()
{
    bcos::boostssl::ws::WsService::start();

    waitForConnectionEstablish();
}

void Service::stop()
{
    bcos::boostssl::ws::WsService::stop();
}

void Service::waitForConnectionEstablish()
{
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::milliseconds(waitConnectFinishTimeout());

    while (true)
    {
        // sleep for connection establish
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (handshakeSucCount())
        {
            RPC_WS_LOG(INFO) << LOG_BADGE("waitForConnectionEstablish")
                             << LOG_DESC("wait for websocket connection handshake success")
                             << LOG_KV("suc count", handshakeSucCount());
            break;
        }

        if (std::chrono::high_resolution_clock::now() < end)
        {
            continue;
        }
        else
        {
            stop();
            RPC_WS_LOG(ERROR) << LOG_BADGE("waitForConnectionEstablish")
                              << LOG_DESC("wait for websocket connection handshake timeout")
                              << LOG_KV("timeout", waitConnectFinishTimeout());

            BOOST_THROW_EXCEPTION(std::runtime_error("The websocket connection handshake timeout"));
            return;
        }
    }
}

void Service::onConnect(Error::Ptr _error, std::shared_ptr<WsSession> _session)
{
    bcos::boostssl::ws::WsService::onConnect(_error, _session);

    startHandshake(_session);
}

void Service::onDisconnect(Error::Ptr _error, std::shared_ptr<WsSession> _session)
{
    bcos::boostssl::ws::WsService::onDisconnect(_error, _session);

    std::string endPoint = _session ? _session->endPoint() : std::string();
    if (!endPoint.empty())
    {
        clearGroupInfo(endPoint);
    }
}

void Service::onRecvMessage(std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session)
{
    auto seq = std::string(_msg->seq()->begin(), _msg->seq()->end());
    if (!checkHandshakeDone(_session))
    {
        // Note: The message is received before the handshake with the node is complete
        RPC_WS_LOG(ERROR) << LOG_BADGE("onRecvMessage")
                          << LOG_DESC(
                                 "websocket service unable to handler message before handshake"
                                 "with the node successfully")
                          << LOG_KV("endpoint", _session ? _session->endPoint() : std::string(""))
                          << LOG_KV("seq", seq);

        _session->drop(bcos::boostssl::ws::WsError::UserDisconnect);
        return;
    }

    bcos::boostssl::ws::WsService::onRecvMessage(_msg, _session);
}

// ---------------------overide end ---------------------------------------------------------------

// ---------------------send message begin---------------------------------------------------------
void Service::asyncSendMessageByGroup(const std::string& _group,
    std::shared_ptr<bcos::boostssl::ws::WsMessage> _msg, bcos::boostssl::ws::Options _options,
    bcos::boostssl::ws::RespCallBack _respFunc)
{
    std::set<std::string> endPoints;
    auto b = getEndPointsByGroup(_group, endPoints);
    if (!b)
    {
        auto error = std::make_shared<Error>(
            WsError::EndPointNotExist, "there has no connection available for the group");
        _respFunc(error, nullptr, nullptr);
        return;
    }

    asyncSendMessage(endPoints, _msg, _options, _respFunc);
}

void Service::asyncSendMessageByGroupAndNode(const std::string& _group, const std::string& _node,
    std::shared_ptr<bcos::boostssl::ws::WsMessage> _msg, bcos::boostssl::ws::Options _options,
    bcos::boostssl::ws::RespCallBack _respFunc)
{
    if (_group.empty())
    {  // random send
        asyncSendMessage(_msg, _options, _respFunc);
    }
    else if (_node.empty())
    {  // send message by group
        asyncSendMessageByGroup(_group, _msg, _options, _respFunc);
    }
    else
    {  // send message by group and node
        std::set<std::string> endPoints;
        auto ok = getEndPointsByGroupAndNode(_group, _node, endPoints);
        if (!ok)
        {
            auto error = std::make_shared<Error>(
                WsError::EndPointNotExist, "there has no connection available for the group/node");
            _respFunc(error, nullptr, nullptr);
            return;
        }

        asyncSendMessage(endPoints, _msg, _options, _respFunc);
    }
}
// ---------------------send message end---------------------------------------------------------


bool Service::checkHandshakeDone(std::shared_ptr<bcos::boostssl::ws::WsSession> _session)
{
    return _session && _session->version();
}

void Service::startHandshake(std::shared_ptr<bcos::boostssl::ws::WsSession> _session)
{
    auto message = messageFactory()->buildMessage();
    message->setType(ws::MessageType::HANDESHAKE);

    RPC_WS_LOG(INFO) << LOG_BADGE("startHandshake")
                     << LOG_KV("endpoint", _session ? _session->endPoint() : std::string(""));

    auto session = _session;
    auto service = std::dynamic_pointer_cast<Service>(shared_from_this());
    _session->asyncSendMessage(message, Options(m_wsHandshakeTimeout),
        [session, service](bcos::Error::Ptr _error, std::shared_ptr<WsMessage> _msg,
            std::shared_ptr<WsSession> _session) {
            if (_error && _error->errorCode() != bcos::protocol::CommonError::SUCCESS)
            {
                RPC_WS_LOG(ERROR) << LOG_BADGE("startHandshake")
                                  << LOG_DESC("callback response error")
                                  << LOG_KV("endpoint",
                                         session ? session->endPoint() : std::string(""))
                                  << LOG_KV("errorCode", _error ? _error->errorCode() : -1)
                                  << LOG_KV("errorMessage",
                                         _error ? _error->errorMessage() : std::string(""));
                session->drop(bcos::boostssl::ws::WsError::UserDisconnect);
                return;
            }

            std::string pvString = std::string(_msg->data()->begin(), _msg->data()->end());
            auto pv = std::make_shared<ProtocolVersion>();
            if (!pv->fromJson(pvString))
            {
                _session->drop(bcos::boostssl::ws::WsError::UserDisconnect);

                RPC_WS_LOG(ERROR) << LOG_BADGE("startHandshake")
                                  << LOG_DESC("invalid protocol version json string")
                                  << LOG_KV("endpoint",
                                         session ? session->endPoint() : std::string(""));
                return;
            }

            // set protocol version
            session->setVersion(pv->protocolVersion());

            auto groupInfoList = pv->groupInfoList();
            for (auto& groupInfo : groupInfoList)
            {
                service->updateGroupInfo(groupInfo);
            }

            service->increaseHandshakeSucCount();

            RPC_WS_LOG(INFO) << LOG_BADGE("startHandshake") << LOG_DESC("handshake successfully")
                             << LOG_KV(
                                    "endpoint", _session ? _session->endPoint() : std::string(""))
                             << LOG_KV("handshake version", _session->version())
                             << LOG_KV("groupInfoList size", groupInfoList.size())
                             << LOG_KV("handshake string", pvString);
        });
}


void Service::onNotifyGroupInfo(
    const std::string& _groupInfoJson, std::shared_ptr<bcos::boostssl::ws::WsSession> _session)
{
    std::string endPoint = _session->endPoint();
    RPC_WS_LOG(TRACE) << LOG_BADGE("onNotifyGroupInfo") << LOG_KV("endPoint", endPoint)
                      << LOG_KV("groupInfoJson", _groupInfoJson);

    try
    {
        auto groupInfo = m_groupInfoFactory->createGroupInfo();
        groupInfo->setChainNodeInfoFactory(m_chainNodeInfoFactory);
        groupInfo->deserialize(_groupInfoJson);

        updateGroupInfo(endPoint, groupInfo);
        updateGroupInfo(groupInfo);
    }
    catch (const std::exception& e)
    {
        RPC_WS_LOG(ERROR) << LOG_BADGE("onNotifyGroupInfo") << LOG_KV("endPoint", endPoint)
                          << LOG_KV("e", boost::diagnostic_information(e))
                          << LOG_KV("groupInfoJson", _groupInfoJson);
    }
}

void Service::onNotifyGroupInfo(std::shared_ptr<bcos::boostssl::ws::WsMessage> _msg,
    std::shared_ptr<bcos::boostssl::ws::WsSession> _session)
{
    std::string groupInfo = std::string(_msg->data()->begin(), _msg->data()->end());

    RPC_WS_LOG(INFO) << LOG_BADGE("onNotifyGroupInfo") << LOG_KV("groupInfo", groupInfo);

    return onNotifyGroupInfo(groupInfo, _session);
}

void Service::clearGroupInfo(const std::string& _endPoint)
{
    RPC_WS_LOG(INFO) << LOG_BADGE("clearGroupInfo") << LOG_KV("endPoint", _endPoint);
    {
        std::unique_lock lock(x_lock);
        for (auto it = m_endPointsMapper.begin(); it != m_endPointsMapper.end();)
        {
            for (auto innerIt = it->second.begin(); innerIt != it->second.end();)
            {
                innerIt->second.erase(_endPoint);

                if (innerIt->second.empty())
                {
                    RPC_WS_LOG(INFO)
                        << LOG_BADGE("clearGroupInfo") << LOG_DESC("clear node")
                        << LOG_KV("group", it->first) << LOG_KV("node", innerIt->first);
                    innerIt = it->second.erase(innerIt);
                }
                else
                {
                    innerIt++;
                }
            }

            if (it->second.empty())
            {
                RPC_WS_LOG(INFO) << LOG_BADGE("clearGroupInfo") << LOG_DESC("clear group")
                                 << LOG_KV("group", it->first);
                it = m_endPointsMapper.erase(it);
            }
            else
            {
                it++;
            }
        }
    }

    // Note: for debug
    printGroupInfo();
}

void Service::clearGroupInfo(const std::string& _groupID, const std::string& _endPoint)
{
    RPC_WS_LOG(INFO) << LOG_BADGE("clearGroupInfo") << LOG_KV("endPoint", _endPoint)
                     << LOG_KV("groupID", _groupID);

    {
        std::unique_lock lock(x_lock);
        auto it = m_endPointsMapper.find(_groupID);
        if (it == m_endPointsMapper.end())
        {
            return;
        }

        auto& groupMapper = it->second;
        for (auto innerIt = groupMapper.begin(); innerIt != groupMapper.end();)
        {
            innerIt->second.erase(_endPoint);
            if (innerIt->second.empty())
            {
                RPC_WS_LOG(INFO) << LOG_BADGE("clearGroupInfo") << LOG_DESC("clear node")
                                 << LOG_KV("group", it->first) << LOG_KV("endPoint", _endPoint)
                                 << LOG_KV("node", innerIt->first);
                innerIt = it->second.erase(innerIt);
            }
            else
            {
                innerIt++;
            }
        }
    }

    // Note: for debug
    // printGroupInfo();
}

void Service::updateGroupInfo(const std::string& _endPoint, bcos::group::GroupInfo::Ptr _groupInfo)
{
    RPC_WS_LOG(INFO) << LOG_BADGE("updateGroupInfo") << LOG_KV("endPoint", _endPoint)
                     << LOG_KV("group", _groupInfo->groupID())
                     << LOG_KV("chainID", _groupInfo->chainID())
                     << LOG_KV("genesisConfig", _groupInfo->genesisConfig())
                     << LOG_KV("iniConfig", _groupInfo->iniConfig())
                     << LOG_KV("nodesNum", _groupInfo->nodesNum());
    const auto& group = _groupInfo->groupID();
    const auto& nodes = _groupInfo->nodeInfos();

    {
        // remove first
        clearGroupInfo(group, _endPoint);
    }

    {
        // update
        std::unique_lock lock(x_lock);
        auto& groupMapper = m_endPointsMapper[group];
        for (const auto& node : nodes)
        {
            auto& nodeMapper = groupMapper[node.first];
            nodeMapper.insert(_endPoint);
        }
    }

    // Note: for debug
    printGroupInfo();
}

bool Service::getEndPointsByGroup(const std::string& _group, std::set<std::string>& _endPoints)
{
    // std::unique_lock lock(x_lock);
    std::shared_lock lock(x_lock);
    auto it = m_endPointsMapper.find(_group);
    if (it == m_endPointsMapper.end())
    {
        RPC_WS_LOG(WARNING) << LOG_BADGE("getEndPointsByGroup") << LOG_DESC("group not exist")
                            << LOG_KV("group", _group);
        return false;
    }

    for (auto& nodeMapper : it->second)
    {
        _endPoints.insert(nodeMapper.second.begin(), nodeMapper.second.end());
    }

    RPC_WS_LOG(INFO) << LOG_BADGE("getEndPointsByGroup") << LOG_KV("group", _group)
                     << LOG_KV("endPoints", _endPoints.size());
    return true;
}

bool Service::getEndPointsByGroupAndNode(
    const std::string& _group, const std::string& _node, std::set<std::string>& _endPoints)
{
    std::shared_lock lock(x_lock);
    auto it = m_endPointsMapper.find(_group);
    if (it == m_endPointsMapper.end())
    {
        RPC_WS_LOG(WARNING) << LOG_BADGE("getEndPointsByGroupAndNode")
                            << LOG_DESC("group not exist") << LOG_KV("group", _group)
                            << LOG_KV("node", _node);
        return false;
    }

    auto innerIt = it->second.find(_node);
    if (innerIt == it->second.end())
    {
        RPC_WS_LOG(WARNING) << LOG_BADGE("getEndPointsByGroupAndNode") << LOG_DESC("node not exist")
                            << LOG_KV("group", _group) << LOG_KV("node", _node);
        return false;
    }

    _endPoints = innerIt->second;

    RPC_WS_LOG(INFO) << LOG_BADGE("getEndPointsByGroupAndNode") << LOG_KV("group", _group)
                     << LOG_KV("node", _node) << LOG_KV("endPoints", _endPoints.size());
    return true;
}

void Service::printGroupInfo()
{
    std::shared_lock lock(x_lock);

    RPC_WS_LOG(INFO) << LOG_BADGE("printGroupInfo")
                     << LOG_KV("total count", m_endPointsMapper.size());

    for (const auto& groupMapper : m_endPointsMapper)
    {
        RPC_WS_LOG(INFO) << LOG_BADGE("printGroupInfo") << LOG_DESC("group list")
                         << LOG_KV("group", groupMapper.first)
                         << LOG_KV("count", groupMapper.second.size());
        for (const auto& nodeMapper : groupMapper.second)
        {
            RPC_WS_LOG(INFO) << LOG_BADGE("printGroupInfo") << LOG_DESC("node list")
                             << LOG_KV("group", groupMapper.first)
                             << LOG_KV("node", nodeMapper.first)
                             << LOG_KV("count", nodeMapper.second.size());
        }
    }
}

bcos::group::GroupInfo::Ptr Service::getGroupInfo(const std::string& _groupID)
{
    std::shared_lock lock(x_lock);
    auto it = m_group2GroupInfo.find(_groupID);
    if (it != m_group2GroupInfo.end())
    {
        return it->second;
    }

    return nullptr;
}

void Service::updateGroupInfo(bcos::group::GroupInfo::Ptr _groupInfo)
{
    RPC_WS_LOG(INFO) << LOG_BADGE("updateGroupInfo") << LOG_KV("groupID", _groupInfo->groupID())
                     << LOG_KV("chainID", _groupInfo->chainID())
                     << LOG_KV("nodesNum", _groupInfo->nodesNum());
    std::unique_lock lock(x_lock);
    m_group2GroupInfo[_groupInfo->groupID()] = _groupInfo;
}

void Service::removeGroupInfo(const std::string& _groupID)
{
    RPC_WS_LOG(INFO) << LOG_BADGE("removeGroupInfo") << LOG_KV("groupID", _groupID);
    std::unique_lock lock(x_lock);
    m_group2GroupInfo.erase(_groupID);
}

//------------------------------ Block Notifier Begin --------------------------
bool Service::getBlockNumber(const std::string& _group, int64_t& _blockNumber)
{
    std::shared_lock lock(x_blockNotifierLock);
    auto it = m_group2BlockNumber.find(_group);
    if (it != m_group2BlockNumber.end())
    {
        _blockNumber = it->second->blockNumber();
        return true;
    }
    return false;
}

void Service::removeBlockNumberInfo(const std::string& _group)
{
    RPC_WS_LOG(INFO) << LOG_BADGE("removeBlockNumberInfo") << LOG_KV("group", _group);
    std::unique_lock lock(x_blockNotifierLock);
    m_group2callbacks.erase(_group);
    m_group2BlockNumber.erase(_group);
}

void Service::onRecvBlockNotifier(const std::string& _msg)
{
    auto bi = std::make_shared<BlockNumberInfo>();
    auto r = bi->fromJson(_msg);
    if (r)
    {
        onRecvBlockNotifier(bi);
    }
}

void Service::onRecvBlockNotifier(BlockNumberInfo::Ptr _blockNumber)
{
    RPC_WS_LOG(INFO) << LOG_BADGE("onRecvBlockNotifier")
                     << LOG_DESC("receive block number notifier")
                     << LOG_KV("group", _blockNumber->group())
                     << LOG_KV("node", _blockNumber->node())
                     << LOG_KV("blockNumber", _blockNumber->blockNumber());

    bool blockNumberUpdate = false;
    {  // update blockinfo
        std::unique_lock lock(x_blockNotifierLock);
        auto it = m_group2BlockNumber.find(_blockNumber->group());
        if (it != m_group2BlockNumber.end())
        {
            auto blockNumber = it->second->blockNumber();
            if (_blockNumber->blockNumber() > blockNumber)
            {
                it->second->setBlockNumber(_blockNumber->blockNumber());
                blockNumberUpdate = true;
            }
        }
        else
        {
            m_group2BlockNumber[_blockNumber->group()] = _blockNumber;
            blockNumberUpdate = true;
        }
    }

    if (blockNumberUpdate)
    {
        RPC_WS_LOG(INFO) << LOG_BADGE("onRecvBlockNotifier") << LOG_DESC("update blockNumber")
                         << LOG_KV("group", _blockNumber->group())
                         << LOG_KV("node", _blockNumber->node())
                         << LOG_KV("blockNumber", _blockNumber->blockNumber());

        std::shared_lock lock(x_blockNotifierLock);
        auto it = m_group2callbacks.find(_blockNumber->group());
        if (it != m_group2callbacks.end())
        {
            for (auto& callback : it->second)
            {
                callback(_blockNumber->group(), _blockNumber->blockNumber());
            }
        }
    }
}

void Service::registerBlockNumberNotifier(
    const std::string& _group, BlockNotifierCallback _callback)
{
    RPC_WS_LOG(INFO) << LOG_BADGE("registerBlockNumberNotifier") << LOG_KV("group", _group);
    std::unique_lock lock(x_blockNotifierLock);
    m_group2callbacks[_group].push_back(_callback);
}
//------------------------------ Block Notifier End --------------------------