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
 * @file JsonRcpImpl.cpp
 * @author: octopus
 * @date 2021-08-10
 */

#include <bcos-cpp-sdk/rpc/Common.h>
#include <bcos-cpp-sdk/rpc/JsonRpcImpl.h>
#include <boost/core/ignore_unused.hpp>
#include <string>

using namespace bcos;
using namespace cppsdk;
using namespace jsonrpc;

void JsonRcpImpl::call(
    const std::string& _group, const std::string& _to, const std::string& _data, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);
    params.append(_to);
    params.append(_data);

    auto request = m_factory->buildRequest("call", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("call") << LOG_KV("request", s);
}

void JsonRcpImpl::sendTransaction(
    const std::string& _group, const std::string& _data, bool _requireProof, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);
    params.append(_data);
    params.append(_requireProof);

    auto request = m_factory->buildRequest("sendTransaction", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("sendTransaction") << LOG_KV("request", s);
}

void JsonRcpImpl::getTransaction(
    const std::string& _group, const std::string& _txHash, bool _requireProof, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);
    params.append(_txHash);
    params.append(_requireProof);

    auto request = m_factory->buildRequest("getTransaction", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getTransaction") << LOG_KV("request", s);
}

void JsonRcpImpl::getTransactionReceipt(
    const std::string& _group, const std::string& _txHash, bool _requireProof, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);
    params.append(_txHash);
    params.append(_requireProof);

    auto request = m_factory->buildRequest("getTransactionReceipt", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getTransactionReceipt") << LOG_KV("request", s);
}

void JsonRcpImpl::getBlockByHash(const std::string& _group, const std::string& _blockHash,
    bool _onlyHeader, bool _onlyTxHash, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);
    params.append(_blockHash);
    params.append(_onlyHeader);
    params.append(_onlyTxHash);

    auto request = m_factory->buildRequest("getBlockByHash", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getBlockByHash") << LOG_KV("request", s);
}

void JsonRcpImpl::getBlockByNumber(const std::string& _group, int64_t _blockNumber,
    bool _onlyHeader, bool _onlyTxHash, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);
    params.append(_blockNumber);
    params.append(_onlyHeader);
    params.append(_onlyTxHash);

    auto request = m_factory->buildRequest("getBlockByNumber", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getBlockByNumber") << LOG_KV("request", s);
}

void JsonRcpImpl::getBlockHashByNumber(
    const std::string& _group, int64_t _blockNumber, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);
    params.append(_blockNumber);

    auto request = m_factory->buildRequest("getBlockHashByNumber", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getBlockHashByNumber") << LOG_KV("request", s);
}

void JsonRcpImpl::getBlockNumber(const std::string& _group, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);

    auto request = m_factory->buildRequest("getBlockNumber", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getBlockNumber") << LOG_KV("request", s);
}

void JsonRcpImpl::getCode(
    const std::string& _group, const std::string _contractAddress, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);
    params.append(_contractAddress);

    auto request = m_factory->buildRequest("getCode", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getCode") << LOG_KV("request", s);
}

void JsonRcpImpl::getSealerList(const std::string& _group, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);

    auto request = m_factory->buildRequest("getSealerList", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getSealerList") << LOG_KV("request", s);
}

void JsonRcpImpl::getObserverList(const std::string& _group, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);

    auto request = m_factory->buildRequest("getObserverList", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getObserverList") << LOG_KV("request", s);
}

void JsonRcpImpl::getPbftView(const std::string& _group, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);

    auto request = m_factory->buildRequest("getPbftView", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getPbftView") << LOG_KV("request", s);
}

void JsonRcpImpl::getPendingTxSize(const std::string& _group, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);

    auto request = m_factory->buildRequest("getPendingTxSize", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getPendingTxSize") << LOG_KV("request", s);
}

void JsonRcpImpl::getSyncStatus(const std::string& _group, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);

    auto request = m_factory->buildRequest("getSyncStatus", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getSyncStatus") << LOG_KV("request", s);
}

void JsonRcpImpl::getSystemConfigByKey(
    const std::string& _group, const std::string& _keyValue, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);
    params.append(_keyValue);

    auto request = m_factory->buildRequest("getSystemConfigByKey", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getSystemConfigByKey") << LOG_KV("request", s);
}

void JsonRcpImpl::getTotalTransactionCount(const std::string& _group, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);

    auto request = m_factory->buildRequest("getTotalTransactionCount", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getTotalTransactionCount") << LOG_KV("request", s);
}

void JsonRcpImpl::getPeers(const std::string& _group, RespFunc _respFunc)
{
    boost::ignore_unused(_group);
    Json::Value params = Json::Value(Json::arrayValue);
    // params.append(_group);

    auto request = m_factory->buildRequest("getPeers", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getPeers") << LOG_KV("request", s);
}

void JsonRcpImpl::getNodeInfo(RespFunc _respFunc)
{
    Json::Value params = Json::Value(Json::arrayValue);

    auto request = m_factory->buildRequest("getNodeInfo", params);
    auto s = request->toString();
    m_sender(s, _respFunc);
    RPCIMPL_LOG(DEBUG) << LOG_BADGE("getNodeInfo") << LOG_KV("request", s);
}