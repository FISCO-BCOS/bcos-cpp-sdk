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
 * @file KeyPairBuilder.h
 * @author: octopus
 * @date 2022-01-13
 */
#pragma once

#include <bcos-cpp-sdk/utilities/crypto/KeyPair.h>
#include <bcos-utilities/Common.h>
#include <memory>
namespace bcos
{
namespace cppsdk
{
namespace utilities
{
class KeyPairBuilder
{
public:
    using Ptr = std::shared_ptr<KeyPairBuilder>;
    using ConstPtr = std::shared_ptr<const KeyPairBuilder>;

public:
    KeyPair::Ptr genKeyPair(CryptoSuiteType _cryptoSuiteType);
    KeyPair::Ptr genKeyPair(CryptoSuiteType _cryptoSuiteType, bytesConstPtr _privateKey);

    KeyPair::Ptr loadKeyPair(const std::string& _pemPath);
    void storeKeyPair(KeyPair::Ptr _keyPair, const std::string& _keyPairPath = "");
};
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos