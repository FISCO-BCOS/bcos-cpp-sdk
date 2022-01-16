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
 * @file KeyPair.h
 * @author: octopus
 * @date 2022-01-13
 */
#pragma once

#include <bcos-cpp-sdk/utilities/crypto/Common.h>
#include <bcos-cpp-sdk/utilities/crypto/hash/HashInterface.h>
#include <bcos-utilities/Common.h>
#include <bcos-utilities/DataConvertUtility.h>
#include <bcos-utilities/FixedBytes.h>

namespace bcos
{
namespace cppsdk
{
namespace utilities
{
class KeyPair
{
public:
    using Ptr = std::shared_ptr<KeyPair>;
    using ConstPtr = std::shared_ptr<const KeyPair>;

public:
    KeyPair(CryptoSuiteType _cryptoSuiteType, bytesConstPtr _privateKey, bytesConstPtr _publicKey)
      : m_cryptoSuiteType(_cryptoSuiteType), m_privateKey(_privateKey), m_publicKey(_publicKey)
    {}

private:
    CryptoSuiteType m_cryptoSuiteType;
    bytesConstPtr m_privateKey;
    bytesConstPtr m_publicKey;

public:
    CryptoSuiteType cryptoSuiteType() const { return m_cryptoSuiteType; }

    bytesConstPtr publicKey() const { return m_publicKey; }
    bytesConstPtr privateKey() const { return m_privateKey; }

    std::string hexPrivateKey() const { return toHexStringWithPrefix(*m_privateKey); }
    std::string hexPublicKey() const { return toHexStringWithPrefix(*m_publicKey); }

    Address address(HashInterface::Ptr _hash)
    {
        return right160(_hash->hash(bytesConstRef(m_publicKey->data(), m_publicKey->size())));
    }
};
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos