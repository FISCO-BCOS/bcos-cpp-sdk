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
#include <bcos-cpp-sdk/utilities/crypto/KeyPair.h>
#include <bcos-cpp-sdk/utilities/crypto/KeyPairBuilder.h>
#include <bcos-cpp-sdk/utilities/crypto/hash/HashInterface.h>
#include <bcos-cpp-sdk/utilities/crypto/hash/Keccak256Hash.h>
#include <bcos-cpp-sdk/utilities/crypto/hash/SM3Hash.h>
#include <bcos-cpp-sdk/utilities/crypto/sign/ECDSASignature.h>
#include <bcos-cpp-sdk/utilities/crypto/sign/SM2Signature.h>
#include <bcos-cpp-sdk/utilities/crypto/sign/SignatureInterface.h>
#include <bcos-utilities/Common.h>
#include <bcos-utilities/DataConvertUtility.h>
#include <bcos-utilities/FixedBytes.h>
#include <memory>

namespace bcos
{
namespace cppsdk
{
namespace utilities
{
class CryptoSuite
{
public:
    using Ptr = std::shared_ptr<CryptoSuite>;
    using ConstPtr = std::shared_ptr<const CryptoSuite>;

public:
    CryptoSuite(const KeyPair& _keyPair)
      : m_keyPair(std::make_shared<KeyPair>(
            _keyPair.cryptoSuiteType(), _keyPair.privateKey(), _keyPair.publicKey())),
        m_cryptoSuiteType(_keyPair.cryptoSuiteType())
    {
        initialize(_keyPair.cryptoSuiteType());
    }

    CryptoSuite(CryptoSuiteType _cryptoSuiteType) : m_cryptoSuiteType(_cryptoSuiteType)
    {
        auto keyPairBuilder = std::make_shared<KeyPairBuilder>();

        m_keyPair = keyPairBuilder->genKeyPair(_cryptoSuiteType);
        initialize(_cryptoSuiteType);
    }

private:
    void initialize(CryptoSuiteType _cryptoSuiteType)
    {
        m_sign = (_cryptoSuiteType == CryptoSuiteType::ECDSA_TYPE ?
                      m_sign = std::make_shared<ECDSASignature>() :
                      m_sign = std::make_shared<SM2Signature>());
    }

private:
    KeyPair::Ptr m_keyPair;
    SignatureInterface::Ptr m_sign;
    CryptoSuiteType m_cryptoSuiteType;

public:
    CryptoSuiteType cryptoSuiteType() const { return m_cryptoSuiteType; }
    KeyPair::Ptr getKeyPair() const { return m_keyPair; }
    SignatureInterface::Ptr getSign() const { return m_sign; }

    Address address() const { return m_keyPair->address(m_sign->getHash()); }

public:
    HashResult hash(bytesConstRef _data) { return m_sign->getHash()->hash(_data); }
    HashResult hash(const std::string& _hexData) const { return m_sign->getHash()->hash(_hexData); }

    bytesConstPtr sign(bytesConstRef _data) { return m_sign->sign(_data, m_keyPair); }
};
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos