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
 * @file RpcInterface.h
 * @author: octopus
 * @date 2021-08-10
 */

#pragma once
#include <bcos-cpp-sdk/rpc/JsonRpcInterface.h>
#include <bcos-cpp-sdk/rpc/JsonRpcRequest.h>
#include <functional>

namespace bcos
{
namespace cppsdk
{
namespace jsonrpc
{
class JsonRcpImpl : public JsonRpcInterface
{
public:
    using Ptr = std::shared_ptr<JsonRcpImpl>;

    JsonRcpImpl() = default;
    virtual ~JsonRcpImpl() = default;

    virtual void start() override;
    virtual void stop() override;

public:
    virtual void call(const std::string& _group, const std::string& _to, const std::string& _data,
        RespFunc _respFunc) override;

    virtual void sendTransaction(const std::string& _group, const std::string& _data,
        bool _requireProof, RespFunc _respFunc) override;

    virtual void getTransaction(const std::string& _group, const std::string& _txHash,
        bool _requireProof, RespFunc _respFunc) override;

    virtual void getTransactionReceipt(const std::string& _group, const std::string& _txHash,
        bool _requireProof, RespFunc _respFunc) override;

    virtual void getBlockByHash(const std::string& _group, const std::string& _blockHash,
        bool _onlyHeader, bool _onlyTxHash, RespFunc _respFunc) override;

    virtual void getBlockByNumber(const std::string& _group, int64_t _blockNumber, bool _onlyHeader,
        bool _onlyTxHash, RespFunc _respFunc) override;

    virtual void getBlockHashByNumber(
        const std::string& _group, int64_t _blockNumber, RespFunc _respFunc) override;

    virtual void getBlockNumber(const std::string& _group, RespFunc _respFunc) override;

    virtual void getCode(
        const std::string& _group, const std::string _contractAddress, RespFunc _respFunc) override;

    virtual void getSealerList(const std::string& _group, RespFunc _respFunc) override;

    virtual void getObserverList(const std::string& _group, RespFunc _respFunc) override;

    virtual void getPbftView(const std::string& _group, RespFunc _respFunc) override;

    virtual void getPendingTxSize(const std::string& _group, RespFunc _respFunc) override;

    virtual void getSyncStatus(const std::string& _group, RespFunc _respFunc) override;

    virtual void getSystemConfigByKey(
        const std::string& _group, const std::string& _keyValue, RespFunc _respFunc) override;

    virtual void getTotalTransactionCount(const std::string& _group, RespFunc _respFunc) override;

    virtual void getPeers(const std::string& _group, RespFunc _respFunc) override;

    virtual void getNodeInfo(RespFunc _respFunc) override;

public:
    JsonRpcRequestFactory::Ptr factory() const { return m_factory; }
    void setFactory(JsonRpcRequestFactory::Ptr _factory) { m_factory = _factory; }
    std::function<void(const std::string& _request, RespFunc _respFunc)> sender() const
    {
        return m_sender;
    }
    void setSender(std::function<void(const std::string& _request, RespFunc _respFunc)> _sender)
    {
        m_sender = _sender;
    }

private:
    JsonRpcRequestFactory::Ptr m_factory;
    std::function<void(const std::string& _request, RespFunc _respFunc)> m_sender;
};

}  // namespace jsonrpc
}  // namespace cppsdk
}  // namespace bcos