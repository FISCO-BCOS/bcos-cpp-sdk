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
 * @file TransactionCodec.cpp
 * @author: octopus
 * @date 2022-01-12
 */
#include <bcos-cpp-sdk/utilities/tx/TransactionCodec.h>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::utilities;

void TransactionCodec::encode(
    const bcostars::TransactionData& _transactionData, bcos::bytes& _buffer)
{
    tars::TarsOutputStream<tars::BufferWriter> output;
    _transactionData.writeTo(output);
    _buffer.assign(output.getByteBuffer().begin(), output.getByteBuffer().end());
}

void TransactionCodec::encodeAndSign(
    const bcostars::TransactionData& _transactionData, KeyPair _keyPair, bcos::bytes& _buffer)
{
    std::ignore = _transactionData;
    std::ignore = _buffer;
    std::ignore = _keyPair;
}

void TransactionCodec::encodeAndSign(
    const bcostars::TransactionData& _transactionData, KeyPair _keyPair, std::string& _hexBuffer)
{
    std::ignore = _transactionData;
    std::ignore = _hexBuffer;
    std::ignore = _keyPair;
}