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
 * @file JsonRPCInterface.h
 * @author: octopus
 * @date 2021-05-24
 */

#pragma once
#include <functional>
#include <memory>

namespace bcos {
namespace cppsdk {
namespace jsonrpc {

class JsonRpcInterface {
public:
  using Ptr = std::shared_ptr<JsonRpcInterface>;

  JsonRpcInterface() = default;
  virtual ~JsonRpcInterface() {}

public:
  virtual void start() = 0;
  virtual void stop() = 0;

public:
  virtual std::string getBlockNumber() = 0;
};

} // namespace jsonrpc
} // namespace cppsdk
} // namespace bcos