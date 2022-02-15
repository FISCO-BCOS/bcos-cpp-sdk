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
    /**
     * @brief Create a Transaction Data object
     *
     * @param _groupID
     * @param _chainID
     * @param _to
     * @param _data
     * @param _abi
     * @param _blockLimit
     * @return bcostars::TransactionDataPtr
     */
    virtual bcostars::TransactionDataPtr createTransactionData(const std::string& _groupID,
        const string& _chainID, const std::string& _to, const bcos::bytes& _data,
        const std::string& _abi, int64_t _blockLimit) = 0;

    /**
     * @brief encode transaction and sign
     *
     * @param _transactionData
     * @param _attribute
     * @param _keyPair
     * @return std::pair<std::string, std::string>
     */
    virtual std::pair<std::string, std::string> encodeAndSign(
        bcostars::TransactionDataConstPtr _transactionData, int32_t _attribute,
        const KeyPair& _keyPair) = 0;

public:
    /**
     * @brief Create a Signed Transaction object
     *
     * @param _keyPair
     * @param _groupID
     * @param _chainID
     * @param _to
     * @param _data
     * @param _blockLimit
     * @param _attribute
     * @return std::pair<std::string, std::string>
     */
    virtual std::pair<std::string, std::string> createSignedTransaction(const KeyPair& _keyPair,
        const std::string& _groupID, const string& _chainID, const std::string& _to,
        const bcos::bytes& _data, int64_t _blockLimit, int32_t _attribute) = 0;

    /**
     * @brief Create a Deploy Contract Transaction object
     *
     * @param _keyPair
     * @param _groupID
     * @param _chainID
     * @param _data
     * @param _abi
     * @param _blockLimit
     * @param _attribute
     * @return std::pair<std::string, std::string>
     */
    virtual std::pair<std::string, std::string> createDeployContractTransaction(
        const KeyPair& _keyPair, const std::string& _groupID, const string& _chainID,
        const bcos::bytes& _data, const std::string& _abi, int64_t _blockLimit,
        int32_t _attribute) = 0;
};
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos