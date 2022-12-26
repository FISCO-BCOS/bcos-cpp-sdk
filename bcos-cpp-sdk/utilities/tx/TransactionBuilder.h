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
 * @file TransactionBuilder.h
 * @author: octopus
 * @date 2022-01-13
 */
#pragma once
#include <bcos-cpp-sdk/utilities/tx/Transaction.h>
#include <bcos-cpp-sdk/utilities/tx/TransactionBuilderInterface.h>
#include <bcos-crypto/hash/Keccak256.h>
#include <bcos-crypto/hash/SM3.h>
#include <bcos-crypto/interfaces/crypto/CryptoSuite.h>
#include <bcos-crypto/signature/secp256k1/Secp256k1Crypto.h>
#include <bcos-crypto/signature/sm2/SM2Crypto.h>
#include <bcos-utilities/Common.h>
#include <memory>

namespace bcos
{
namespace cppsdk
{
namespace utilities
{
class TransactionBuilder : public TransactionBuilderInterface
{
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
        const std::string& _abi, int64_t _blockLimit) override;

    /**
     * @brief
     *
     * @param _transactionData
     * @return bytesConstPtr
     */
    bytesConstPtr encodeTransactionData(const bcostars::TransactionData& _transactionData) override;

    /**
     * @brief
     *
     * @param _cryptoType
     * @param _transactionData
     * @return crypto::HashType
     */
    virtual crypto::HashType calculateTransactionDataHash(
        CryptoType _cryptoType, const bcostars::TransactionData& _transactionData) override;

    /**
     * @brief
     *
     * @param _keyPair
     * @param _transactionDataHash
     * @return bcos::bytesConstPtr
     */
    virtual bcos::bytesConstPtr signTransactionDataHash(
        const bcos::crypto::KeyPairInterface& _keyPair,
        const crypto::HashType& _transactionDataHash) override;

    /**
     * @brief Create a Transaction object
     *
     * @param _transactionData
     * @param _signData
     * @param _hash
     * @param _attribute
     * @param _extraData
     * @return bcostars::TransactionUniquePtr
     */
    virtual bcostars::TransactionUniquePtr createTransaction(
        const bcostars::TransactionData& _transactionData, const bcos::bytes& _signData,
        const crypto::HashType& _hash, int32_t _attribute,
        const std::string& _extraData = "") override;

    /**
     * @brief
     *
     * @param _transaction
     * @return bytesConstPtr
     */
    virtual bytesConstPtr encodeTransaction(const bcostars::Transaction& _transaction) override;

    /**
     * @brief Create a Signed Transaction object
     *
     * @param _transactionData
     * @param _signData
     * @param _transactionDataHash
     * @param _attribute
     * @param _extraData
     * @return bytesConstPtr
     */
    virtual bytesConstPtr createSignedTransaction(const bcostars::TransactionData& _transactionData,
        const bcos::bytes& _signData, const crypto::HashType& _transactionDataHash,
        int32_t _attribute, const std::string& _extraData = "") override;

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
     * @param _extraData
     * @return std::pair<std::string, std::string>
     */
    virtual std::pair<std::string, std::string> createSignedTransaction(
        const bcos::crypto::KeyPairInterface& _keyPair, const std::string& _groupID,
        const string& _chainID, const std::string& _to, const bcos::bytes& _data,
        const std::string& _abi, int64_t _blockLimit, int32_t _attribute,
        const std::string& _extraData = "") override;


    u256 genRandomUint256();

public:
    auto ecdsaCryptoSuite() -> auto& { return m_ecdsaCryptoSuite; }
    auto smCryptoSuite() -> auto& { return m_smCryptoSuite; }

private:
    bcos::crypto::CryptoSuite::UniquePtr m_ecdsaCryptoSuite =
        std::make_unique<bcos::crypto::CryptoSuite>(std::make_shared<bcos::crypto::Keccak256>(),
            std::make_shared<bcos::crypto::Secp256k1Crypto>(), nullptr);

    bcos::crypto::CryptoSuite::UniquePtr m_smCryptoSuite =
        std::make_unique<bcos::crypto::CryptoSuite>(std::make_shared<bcos::crypto::SM3>(),
            std::make_shared<bcos::crypto::SM2Crypto>(), nullptr);
};
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos