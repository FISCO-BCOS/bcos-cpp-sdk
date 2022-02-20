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
 * @file TransactionBuilder.cpp
 * @author: octopus
 * @date 2022-01-13
 */
#include <bcos-cpp-sdk/utilities/tx/Transaction.h>
#include <bcos-cpp-sdk/utilities/tx/TransactionBuilder.h>
#include <bcos-crypto/interfaces/crypto/CryptoSuite.h>
#include <bcos-utilities/Common.h>
#include <bcos-utilities/DataConvertUtility.h>
#include <bcos-utilities/FixedBytes.h>
#include <memory>
#include <utility>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::utilities;

/**
 * @brief
 *
 * @param _transactionData
 * @return bytesConstPtr
 */
bytesConstPtr TransactionBuilder::encodeTransactionData(
    const bcostars::TransactionData& _transactionData)
{
    tars::TarsOutputStream<tars::BufferWriter> output;
    _transactionData.writeTo(output);

    auto buffer = std::make_shared<bcos::bytes>();
    buffer->assign(output.getBuffer(), output.getBuffer() + output.getLength());
    return buffer;
}

/**
 * @brief
 *
 * @param _transactionData
 * @return bytesConstPtr
 */
bytesConstPtr TransactionBuilder::encodeTransaction(const bcostars::Transaction& _transaction)
{
    tars::TarsOutputStream<tars::BufferWriter> output;
    _transaction.writeTo(output);

    auto buffer = std::make_shared<bcos::bytes>();
    buffer->assign(output.getBuffer(), output.getBuffer() + output.getLength());
    return buffer;
}

/**
 * @brief
 *
 * @param _groupID
 * @param _chainID
 * @param _to
 * @param _data
 * @param _abi
 * @param _blockLimit
 * @return bcostars::TransactionDataUniquePtr
 */
bcostars::TransactionDataUniquePtr TransactionBuilder::createTransactionData(
    const std::string& _groupID, const string& _chainID, const std::string& _to,
    const bcos::bytes& _data, const std::string& _abi, int64_t _blockLimit)
{
    auto fixBytes256 = FixedBytes<256>().generateRandomFixedBytes();

    auto _transactionData = std::make_unique<bcostars::TransactionData>();
    _transactionData->version = 0;
    _transactionData->chainID = _chainID;
    _transactionData->groupID = _groupID;
    _transactionData->to = _to;
    _transactionData->blockLimit = _blockLimit;
    _transactionData->nonce = u256(fixBytes256.hexPrefixed()).str(10);
    _transactionData->abi = _abi;

    BCOS_LOG(TRACE) << LOG_BADGE("TransactionBuilder::createTransaction")
                    << LOG_KV("hex", fixBytes256.hexPrefixed())
                    << LOG_KV("nonce", _transactionData->nonce);

    _transactionData->input.insert(_transactionData->input.begin(), _data.begin(), _data.end());
    // TODO: trim 0x prefix ???
    // _transactionData->to =
    //    (_to.compare(0, 2, "0x") == 0 || _to.compare(0, 2, "0X") == 0) ? _to.substr(2) : _to;

    return _transactionData;
}

/**
 * @brief
 *
 * @param _cryptoType
 * @param _transactionData
 * @return crypto::HashType
 */
crypto::HashType TransactionBuilder::calculateTransactionDataHash(
    CryptoType _cryptoType, const bcostars::TransactionData& _transactionData)
{
    bcos::crypto::CryptoSuite* cryptoSuite = nullptr;
    if (_cryptoType == bcos::crypto::KeyPairType::SM2)
    {
        cryptoSuite = &*m_smCryptoSuite;
    }
    else
    {
        cryptoSuite = &*m_ecdsaCryptoSuite;
    }

    auto encodedTransactionData = encodeTransactionData(_transactionData);
    auto encodedHash = cryptoSuite->hash(
        bytesConstRef(encodedTransactionData->data(), encodedTransactionData->size()));
    return encodedHash;
}

/**
 * @brief
 *
 * @param _keyPair
 * @param _hashType
 * @return crypto::HashType
 */
bcos::bytesConstPtr TransactionBuilder::signTransactionDataHash(
    const bcos::crypto::KeyPairInterface& _keyPair, const crypto::HashType& _transactionDataHash)
{
    bcos::crypto::CryptoSuite* cryptoSuite = nullptr;
    if (_keyPair.keyPairType() == bcos::crypto::KeyPairType::SM2)
    {
        cryptoSuite = &*m_smCryptoSuite;
    }
    else
    {
        cryptoSuite = &*m_ecdsaCryptoSuite;
    }

    //  sign transaction data hash
    auto signData = cryptoSuite->signatureImpl()->sign(_keyPair, _transactionDataHash, false);
    return signData;
}

/**
 * @brief Create a Transaction object
 *
 * @param _transactionData
 * @param _signData
 * @param _attribute
 * @return bcostars::TransactionUniquePtr
 */
bcostars::TransactionUniquePtr TransactionBuilder::createTransaction(
    const bcostars::TransactionData& _transactionData, const bcos::bytes& _signData,
    const crypto::HashType& _hash, int32_t _attribute)
{
    auto transaction = std::make_unique<bcostars::Transaction>();
    transaction->data = _transactionData;
    transaction->dataHash.insert(transaction->dataHash.begin(), _hash.begin(), _hash.end());
    transaction->signature.insert(
        transaction->signature.begin(), _signData.begin(), _signData.end());
    transaction->importTime = 0;
    transaction->attribute = _attribute;
    return transaction;
}

/**
 * @brief
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
std::pair<std::string, std::string> TransactionBuilder::createSignedTransaction(
    const bcos::crypto::KeyPairInterface& _keyPair, const std::string& _groupID,
    const string& _chainID, const std::string& _to, const bcos::bytes& _data,
    const std::string& _abi, int64_t _blockLimit, int32_t _attribute)
{
    auto transactionData = createTransactionData(_groupID, _chainID, _to, _data, _abi, _blockLimit);
    auto transactionDataHash =
        calculateTransactionDataHash(_keyPair.keyPairType(), *transactionData);
    auto signData = signTransactionDataHash(_keyPair, transactionDataHash);
    auto transaction =
        createTransaction(*transactionData, *signData, transactionDataHash, _attribute);
    auto encodedTx = encodeTransaction(*transaction);

    return std::make_pair<std::string, std::string>(
        toHexStringWithPrefix(transactionDataHash), toHexStringWithPrefix(*encodedTx));
}