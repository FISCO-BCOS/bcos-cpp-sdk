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
 * @file JsonRpcRequest.h
 * @author: octopus
 * @date 2021-08-10
 */

#pragma once
#include <any>
#include <atomic>
#include <json/json.h>
#include <list>
#include <memory>
#include <string>

namespace bcos {
namespace cppsdk {
namespace jsonrpc {

class JsonRpcRequest {
public:
  using Ptr = std::shared_ptr<JsonRpcRequest>;

  JsonRpcRequest() = default;
  ~JsonRpcRequest() = default;

public:
  void setJsonrpc(const std::string _jsonrpc) { m_jsonrpc = _jsonrpc; }
  std::string jsonrpc() { return m_jsonrpc; }
  void setMethod(const std::string _method) { m_method = _method; }
  std::string method() { return m_method; }
  void setId(int64_t _id) { m_id = _id; }
  int64_t id() { return m_id; }

public:
  // TODO: how to encode variables type of params
  std::string toJsonWithParams(Json::Value jParams);

private:
  std::string m_jsonrpc = "3.0";
  std::string m_method;
  int64_t m_id{1};
};

class JsonRpcRequestFactory {
public:
  using Ptr = std::shared_ptr<JsonRpcRequestFactory>;
  JsonRpcRequestFactory() {}

public:
  JsonRpcRequest::Ptr buildRequest(const std::string &_method) {
    auto request = std::make_shared<JsonRpcRequest>();
    auto id = nextID();
    request->setId(id);
    request->setMethod(_method);
    return request;
  }

  int64_t nextID() {
    int64_t _id = ++id;
    return _id;
  }

private:
  std::atomic<int64_t> id{0};
};

} // namespace jsonrpc
} // namespace cppsdk
} // namespace bcos