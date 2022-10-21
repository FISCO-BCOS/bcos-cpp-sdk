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
 * @file Signature.cpp
 * @author: lucasli
 * @date 2022-12-14
 */
#include <bcos-cpp-sdk/utilities/Common.h>
#include <bcos-cpp-sdk/utilities/crypto/Signature.h>
#include <bcos-crypto/interfaces/crypto/KeyInterface.h>
#include <bcos-crypto/signature/hsmSM2/HsmSM2Crypto.h>
#include <bcos-crypto/signature/key/KeyPair.h>
#include <bcos-utilities/Common.h>
#include <memory>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::utilities;

std::shared_ptr<bcos::bytes> Signature::sign(const bcos::crypto::KeyPairInterface& _keyPair,
    const bcos::crypto::HashType& _hash, const std::string _hsmLibPath)
{
    auto crypto_type = _keyPair.keyPairType();
    if (crypto_type == CryptoType::HsmSM2)
    {
        auto signatureImpl = std::make_shared<bcos::crypto::HsmSM2Crypto>(_hsmLibPath);
        return signatureImpl->sign(_keyPair, _hash);
    }
}

bool Signature::verify(CryptoType crypto_type, std::shared_ptr<bcos::bytes const> _pubKeyBytes,
    const bcos::crypto::HashType& _hash, bcos::bytesConstRef _signatureData,
    const std::string _hsmLibPath)
{
    if (crypto_type == CryptoType::HsmSM2)
    {
        auto signatureImpl = std::make_shared<bcos::crypto::HsmSM2Crypto>(_hsmLibPath);
        return signatureImpl->verify(_pubKeyBytes, _hash, _signatureData);
    }
}

bcos::crypto::PublicPtr Signature::recover(CryptoType crypto_type,
    const bcos::crypto::HashType& _hash, bcos::bytesConstRef _signatureData,
    const std::string _hsmLibPath)
{
    if (crypto_type == CryptoType::HsmSM2)
    {
        auto signatureImpl = std::make_shared<bcos::crypto::HsmSM2Crypto>(_hsmLibPath);
        return signatureImpl->recover(_hash, _signatureData);
    }
}

std::pair<bool, bcos::bytes> Signature::recoverAddress(CryptoType crypto_type,
    bcos::crypto::Hash::Ptr _hashImpl, bcos::bytesConstRef _in, const std::string _hsmLibPath)
{
    if (crypto_type == CryptoType::HsmSM2)
    {
        auto signatureImpl = std::make_shared<bcos::crypto::HsmSM2Crypto>(_hsmLibPath);
        return signatureImpl->recoverAddress(_hashImpl, _in);
    }
}
