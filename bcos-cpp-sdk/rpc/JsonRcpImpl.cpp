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

void JsonRcpImpl::start() {}

void JsonRcpImpl::stop() {}

std::string JsonRcpImpl::syncSendRPCRequest(const std::string &_request) {
  boost::ignore_unused(_request);
  return std::string();
}

std::string JsonRcpImpl::getBlockNumber() {
  /*
  auto request = m_jsonRPCRequestFactory->buildJsonRPCRequest("getBlockNumber");
  Json::Value jParams = Json::Value(Json::arrayValue);
  jParams.append(m_groupID);
  auto jsonValue = request->toJsonWithParams(jParams);
  */
  return std::string();
}