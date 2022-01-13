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
#include <bcos-cpp-sdk/utilities/tx/TransactionBuilder.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::utilities;

/**
 * @brief
 *
 * @param _to
 * @param _data
 * @param _chainID
 * @param _groupID
 * @param _blockLimit
 * @param _nonce
 * @return bcostars::TransactionDataPtr
 */
bcostars::TransactionDataPtr TransactionBuilder::createTransaction(const std::string& _to,
    const bcos::bytes& _data, const string& _chainID, const std::string& _groupID,
    int64_t _blockLimit, const std::string& _nonce)
{
    auto _transactionData = std::make_shared<bcostars::TransactionData>();
    _transactionData->version = 0;
    _transactionData->chainID = _chainID;
    _transactionData->groupID = _groupID;
    _transactionData->blockLimit = _blockLimit;
    _transactionData->nonce = _nonce;
    _transactionData->input.insert(_transactionData->input.begin(), _data.begin(), _data.end());
    // trim 0x prefix
    _transactionData->to =
        (_to.compare(0, 2, "0x") == 0 || _to.compare(0, 2, "0X") == 0) ? _to.substr(2) : _to;

    return _transactionData;
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
    // generator new nonce
    std::string nonce = boost::uuids::to_string(boost::uuids::random_generator()());
    nonce.erase(std::remove(nonce.begin(), nonce.end(), '-'), nonce.end());

    return createTransaction(_to, _data, _chainID, _groupID, _blockLimit, nonce);
}
