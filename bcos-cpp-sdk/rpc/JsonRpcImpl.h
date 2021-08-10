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

namespace bcos {
namespace cppsdk {
namespace jsonrpc {

using JsonRpcRespCallback = std::function<void(const std::string &_resp)>;

class JsonRcpImpl : public JsonRpcInterface {
public:
  using Ptr = std::shared_ptr<JsonRcpImpl>;

  JsonRcpImpl() = default;
  virtual ~JsonRcpImpl() = default;

  virtual void start() override;
  virtual void stop() override;

  void syncSendRPCRequest(const std::string &_request,
                          JsonRpcRespCallback _callback);
  std::string syncSendRPCRequest(const std::string &_request);

public:
  virtual std::string getBlockNumber() override;

public:
private:
  uint32_t m_groupID;
};

} // namespace jsonrpc
} // namespace cppsdk
} // namespace bcos