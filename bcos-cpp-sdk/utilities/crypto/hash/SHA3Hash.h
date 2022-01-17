/**
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
 * @brief interfaces for Hash
 * @file SM3Hash.h
 * @author: octopuswang
 * @date 2022-01-13
 */
#pragma once

#include <bcos-cpp-sdk/utilities/crypto/Common.h>
#include <bcos-cpp-sdk/utilities/crypto/hash/HashInterface.h>
#include <bcos-utilities/Common.h>

namespace bcos
{
namespace cppsdk
{
namespace utilities
{
class SHA3Hash : public HashInterface
{
public:
    SHA3Hash() : HashInterface(HashType::SHA3) {}

public:
    using Ptr = std::shared_ptr<SHA3Hash>;
    using ConstPtr = std::shared_ptr<const SHA3Hash>;

public:
    virtual HashResult hash(bytesConstRef _data) override;
};
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos
