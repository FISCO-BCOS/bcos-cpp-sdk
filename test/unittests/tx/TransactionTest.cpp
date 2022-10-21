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
 * @file TransactionTest.cpp
 * @author: yujiechen
 * @date 2022-05-31
 */
#include "utilities/tx/Transaction.h"
#include <bcos-crypto/hash/SM3.h>
#include <bcos-crypto/interfaces/crypto/CryptoSuite.h>
#include <bcos-crypto/signature/sm2/SM2Crypto.h>
#include <bcos-utilities/testutils/TestPromptFixture.h>

using namespace bcostars;
using namespace bcos;
using namespace bcos::crypto;

namespace bcos::test
{
BOOST_FIXTURE_TEST_SUITE(TransactionTest, TestPromptFixture)

BOOST_AUTO_TEST_CASE(test_transaction)
{
    TransactionData txData;
    std::string to("Target");
    bcos::bytes input(bcos::asBytes("Arguments"));
    bcos::u256 nonce(800);

    txData.version = 0;
    txData.to = to;
    std::vector<tars::Char> txInput(input.begin(), input.end());
    txData.input = std::move(txInput);
    txData.nonce = boost::lexical_cast<std::string>(nonce);
    txData.blockLimit = 100;
    txData.chainID = "testChain";
    txData.groupID = "testGroup";

    auto cryptoSuite =
        std::make_shared<bcos::crypto::CryptoSuite>(std::make_shared<bcos::crypto::SM3>(),
            std::make_shared<bcos::crypto::SM2Crypto>(), nullptr);

    auto hash = txData.hash(cryptoSuite->hashImpl());
    BOOST_CHECK_EQUAL(
        hash.hex(), "a060addbd0f5b02806a48bee54fdac997ca2b3a7ff2311715f3af4d3ee727285");

    // set version to 10
    txData.version = 10;
    hash = txData.hash(cryptoSuite->hashImpl());
    BOOST_CHECK_EQUAL(
        hash.hex(), "8717ad33c7ee088d86be7594f2c4e45fecd7c6d1199ae4c11a37fca1ad11da2e");
}
BOOST_AUTO_TEST_SUITE_END()
}  // namespace bcos::test