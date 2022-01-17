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
 * @file SM2Signature.h
 * @author: octopus
 * @date 2022-01-13
 */
#pragma once

#include <bcos-cpp-sdk/utilities/crypto/KeyPair.h>
#include <bcos-cpp-sdk/utilities/crypto/hash/SM3Hash.h>
#include <bcos-cpp-sdk/utilities/crypto/sign/SignatureInterface.h>

namespace bcos
{
namespace cppsdk
{
namespace utilities
{
class SM2Signature : public SignatureInterface
{
public:
    SM2Signature() : SignatureInterface(SignatureType::SM, std::make_shared<SM3Hash>()) {}

public:
    using Ptr = std::shared_ptr<SM2Signature>;
    using ConstPtr = std::shared_ptr<const SM2Signature>;

public:
    virtual bytesPointer sign(HashResult _hash, KeyPair::Ptr _keyPair) override;
    virtual bool verify(
        bytesConstPtr _publicKey, HashResult _hashResult, bytesConstPtr _signature) override;
};
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos