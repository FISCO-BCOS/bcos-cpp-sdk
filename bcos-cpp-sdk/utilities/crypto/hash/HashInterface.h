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
 * @file HashInterface.h
 * @author: octopuswang
 * @date 2022-01-13
 */
#pragma once

#include <bcos-cpp-sdk/utilities/crypto/Common.h>
#include <bcos-utilities/Common.h>
#include <bcos-utilities/FixedBytes.h>

namespace bcos
{
namespace cppsdk
{
namespace utilities
{
enum class HashType : int
{
    KECCAK256,
    SM3,
    SHA3
};

using HashResult = h256;

class HashInterface
{
public:
    using Ptr = std::shared_ptr<HashInterface>;
    using ConstPtr = std::shared_ptr<const HashInterface>;

public:
    HashInterface(HashType _hashType) : m_hashType(_hashType) {}
    virtual ~HashInterface() {}

public:
    inline HashType hashType() const { return m_hashType; }

public:
    virtual HashResult hash(bytesConstRef _data) = 0;

    HashResult hash(const std::string& _data)
    {
        // TODO: ???
        // auto bytesData = fromHexString(_data);
        return hash(bytesConstRef((byte*)_data.data(), _data.size()));
    }

private:
    HashType m_hashType = HashType();
};
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos
