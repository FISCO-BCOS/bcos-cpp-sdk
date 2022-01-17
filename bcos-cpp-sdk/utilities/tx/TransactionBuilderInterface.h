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
 * @file TransactionBuilderInterface.h
 * @author: octopus
 * @date 2022-01-13
 */
#pragma once
#include <bcos-cpp-sdk/utilities/crypto/KeyPair.h>
#include <bcos-cpp-sdk/utilities/tx/Transaction.h>
#include <bcos-utilities/Common.h>

namespace bcos
{
namespace cppsdk
{
namespace utilities
{
class TransactionBuilderInterface
{
public:
    virtual ~TransactionBuilderInterface() {}

public:
    virtual bcostars::TransactionDataPtr createTransaction(const std::string& _to,
        const bcos::bytes& _data, const string& _chainID, const std::string& _groupID,
        int64_t _blockLimit) = 0;

    virtual bytesConstPtr encodeAndSign(
        bcostars::TransactionDataConstPtr _transactionData, const KeyPair& _keyPair) = 0;

    virtual std::string createSignedTransaction(const std::string& _to, const bcos::bytes& _data,
        const string& _chainID, const std::string& _groupID, int64_t _blockLimit,
        const KeyPair& _keyPair) = 0;
};
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos