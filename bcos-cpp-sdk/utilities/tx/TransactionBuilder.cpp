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
#include <bcos-cpp-sdk/utilities/crypto/CryptoSuite.h>
#include <bcos-cpp-sdk/utilities/crypto/KeyPair.h>
#include <bcos-cpp-sdk/utilities/tx/Transaction.h>
#include <bcos-cpp-sdk/utilities/tx/TransactionBuilder.h>
#include <bcos-utilities/Common.h>
#include <bcos-utilities/FixedBytes.h>
using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::utilities;

/**
 * @brief
 *
 * @param _transactionData
 * @return bytesConstPtr
 */
bytesConstPtr TransactionBuilder::encodeTxData(bcostars::TransactionDataConstPtr _transactionData)
{
    tars::TarsOutputStream<tars::BufferWriter> output;
    _transactionData->writeTo(output);

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
bytesConstPtr TransactionBuilder::encodeTx(bcostars::TransactionConstPtr _transaction)
{
    tars::TarsOutputStream<tars::BufferWriter> output;
    _transaction->writeTo(output);

    auto buffer = std::make_shared<bcos::bytes>();
    buffer->assign(output.getBuffer(), output.getBuffer() + output.getLength());
    return buffer;
}

/**
 * @brief
 *
 * @param _to
 * @param _data
 * @param _chainID
 * @param _groupID
 * @param _blockLimit
 * @return bcostars::TransactionDataPtr
 */
bcostars::TransactionDataPtr TransactionBuilder::createTransaction(const std::string& _to,
    const bcos::bytes& _data, const string& _chainID, const std::string& _groupID,
    int64_t _blockLimit)
{
    auto fixBytes256 = FixedBytes<256>().generateRandomFixedBytes();

    auto _transactionData = std::make_shared<bcostars::TransactionData>();
    _transactionData->version = 0;
    _transactionData->chainID = _chainID;
    _transactionData->groupID = _groupID;
    _transactionData->blockLimit = _blockLimit;
    _transactionData->nonce = u256(fixBytes256.hexPrefixed()).str(10);

    BCOS_LOG(INFO) << LOG_BADGE("TransactionBuilder::createTransaction")
                   << LOG_KV("hex", fixBytes256.hexPrefixed())
                   << LOG_KV("nonce", _transactionData->nonce);

    _transactionData->input.insert(_transactionData->input.begin(), _data.begin(), _data.end());
    // trim 0x prefix
    _transactionData->to =
        (_to.compare(0, 2, "0x") == 0 || _to.compare(0, 2, "0X") == 0) ? _to.substr(2) : _to;

    return _transactionData;
}

/**
 * @brief
 *
 * @param _transactionData
 * @param _keyPair
 * @return bytesConstPtr
 */
bytesConstPtr TransactionBuilder::encodeAndSign(
    bcostars::TransactionDataConstPtr _transactionData, const KeyPair& _keyPair)
{
    auto cryptoSuite = std::make_shared<CryptoSuite>(_keyPair);

    // hash and sign transaction data
    auto encoded = encodeTxData(_transactionData);
    auto encodedHash = cryptoSuite->hash(bytesConstRef(encoded->data(), encoded->size()));
    auto signData = cryptoSuite->sign(encodedHash.ref());

    auto transaction = std::make_shared<bcostars::Transaction>();
    transaction->data = *_transactionData;
    transaction->dataHash.insert(
        transaction->dataHash.begin(), encodedHash.begin(), encodedHash.end());
    transaction->signature.insert(
        transaction->signature.begin(), signData->begin(), signData->end());
    transaction->importTime = 0;
    // TODO: add attribute value
    transaction->attribute = 0;

    return encodeTx(transaction);
}

/**
 * @brief Create a Signed Transaction object
 *
 * @param _to
 * @param _data
 * @param _chainID
 * @param _groupID
 * @param _blockLimit
 * @param _nonce
 * @return std::string
 */

std::string TransactionBuilder::createSignedTransaction(const std::string& _to,
    const bcos::bytes& _data, const string& _chainID, const std::string& _groupID,
    int64_t _blockLimit, const KeyPair& _keyPair)
{
    auto transactionData = createTransaction(_to, _data, _chainID, _groupID, _blockLimit);
    auto encoded = encodeAndSign(transactionData, _keyPair);
    return toHexStringWithPrefix(*encoded);
}