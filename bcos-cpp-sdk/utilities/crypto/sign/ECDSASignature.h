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
 * @file ECDSASignature.h
 * @author: octopus
 * @date 2022-01-13
 */
#pragma once

#include <bcos-cpp-sdk/utilities/crypto/KeyPair.h>
#include <bcos-cpp-sdk/utilities/crypto/hash/Keccak256Hash.h>
#include <bcos-cpp-sdk/utilities/crypto/sign/SignatureInterface.h>

namespace bcos
{
namespace cppsdk
{
namespace utilities
{
class ECDSASignature : public SignatureInterface
{
public:
    ECDSASignature() : SignatureInterface(SignatureType::ECDSA, std::make_shared<Keccak256Hash>())
    {}

public:
    using Ptr = std::shared_ptr<ECDSASignature>;
    using ConstPtr = std::shared_ptr<const ECDSASignature>;

public:
    virtual bytesPointer sign(HashResult _hash, KeyPair::Ptr _keyPair) override;

    virtual bool verify(
        bytesConstPtr _publicKey, HashResult _hashResult, bytesConstPtr _signature) override;
};
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos