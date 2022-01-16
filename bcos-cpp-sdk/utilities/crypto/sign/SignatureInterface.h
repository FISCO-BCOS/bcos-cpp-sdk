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
 * @file Signature.h
 * @author: octopus
 * @date 2022-01-13
 */
#pragma once

#include <bcos-cpp-sdk/utilities/crypto/Common.h>
#include <bcos-cpp-sdk/utilities/crypto/KeyPair.h>
#include <bcos-cpp-sdk/utilities/crypto/hash/HashInterface.h>
#include <bcos-utilities/Common.h>
#include <memory>

namespace bcos
{
namespace cppsdk
{
namespace utilities
{
enum class SignatureType : int
{
    ECDSA = 1,
    SM = 2
};

class SignatureInterface
{
public:
    SignatureInterface(SignatureType _signType, HashInterface::Ptr _hashPtr)
      : m_hashPtr(_hashPtr), m_signType(_signType)
    {}
    virtual ~SignatureInterface() {}

public:
    using Ptr = std::shared_ptr<SignatureInterface>;
    using ConstPtr = std::shared_ptr<const SignatureInterface>;

public:
    HashResult hash(bytesConstRef _data) const { return m_hashPtr->hash(_data); }
    SignatureType signType() const { return m_signType; }
    HashInterface::Ptr getHash() const { return m_hashPtr; }

    bytesPointer sign(bytesConstRef _data, KeyPair::Ptr _keyPair)
    {
        auto hashResult = m_hashPtr->hash(_data);
        return sign(hashResult, _keyPair);
    }

    bool verify(bytesConstPtr _publicKey, bytesConstRef _data, bytesConstPtr _signature)
    {
        auto hashResult = m_hashPtr->hash(_data);
        return verify(_publicKey, hashResult, _signature);
    }

public:
    virtual bytesPointer sign(HashResult _hashResult, KeyPair::Ptr _keyPair) = 0;
    virtual bool verify(
        bytesConstPtr _publicKey, HashResult _hashResult, bytesConstPtr _signature) = 0;

private:
    HashInterface::Ptr m_hashPtr;
    SignatureType m_signType;
};
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos