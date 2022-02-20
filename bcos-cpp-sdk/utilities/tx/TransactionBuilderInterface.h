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
#include <bcos-cpp-sdk/utilities/crypto/Common.h>
#include <bcos-cpp-sdk/utilities/tx/Transaction.h>
#include <bcos-crypto/signature/key/KeyPair.h>
#include <bcos-utilities/Common.h>

namespace bcos
{
namespace cppsdk
{
namespace utilities
{
using CryptoType = crypto::KeyPairType;

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
     * @return bcostars::TransactionDataUniquePtr
     */
    virtual bcostars::TransactionDataUniquePtr createTransactionData(const std::string& _groupID,
        const string& _chainID, const std::string& _to, const bcos::bytes& _data,
        const std::string& _abi, int64_t _blockLimit) = 0;

    /**
     * @brief
     *
     * @param _transactionData
     * @return bytesConstPtr
     */
    virtual bytesConstPtr encodeTransactionData(
        const bcostars::TransactionData& _transactionData) = 0;

    /**
     * @brief
     *
     * @param _cryptoType
     * @param _transactionData
     * @return crypto::HashType
     */
    virtual crypto::HashType calculateTransactionDataHash(
        CryptoType _cryptoType, const bcostars::TransactionData& _transactionData) = 0;

    /**
     * @brief
     *
     * @param _keyPair
     * @param _hashType
     * @return crypto::HashType
     */
    virtual bcos::bytesConstPtr signTransactionDataHash(
        const bcos::crypto::KeyPairInterface& _keyPair,
        const crypto::HashType& _transactionDataHash) = 0;

    /**
     * @brief Create a Transaction object
     *
     * @param _transactionData
     * @param _signData
     * @param _hash
     * @param _attribute
     * @return bcostars::TransactionUniquePtr
     */
    virtual bcostars::TransactionUniquePtr createTransaction(
        const bcostars::TransactionData& _transactionData, const bcos::bytes& _signData,
        const crypto::HashType& _transactionDataHash, int32_t _attribute) = 0;

    /**
     * @brief
     *
     * @param _transaction
     * @return bytesConstPtr
     */
    virtual bytesConstPtr encodeTransaction(const bcostars::Transaction& _transaction) = 0;

public:
    /**
     * @brief Create a Signed Transaction object
     *
     * @param _keyPair
     * @param _groupID
     * @param _chainID
     * @param _to
     * @param _data
     * @param _abi
     * @param _blockLimit
     * @param _attribute
     * @return std::pair<std::string, std::string>
     */
    virtual std::pair<std::string, std::string> createSignedTransaction(
        const bcos::crypto::KeyPairInterface& _keyPair, const std::string& _groupID,
        const string& _chainID, const std::string& _to, const bcos::bytes& _data,
        const std::string& _abi, int64_t _blockLimit, int32_t _attribute) = 0;
};
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos