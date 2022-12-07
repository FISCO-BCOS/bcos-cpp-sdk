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
#include <bcos-cpp-sdk/utilities/receipt/ReceiptBuilder.h>
#include <bcos-cpp-sdk/utilities/tx/TransactionBuilder.h>
#include <bcos-crypto/interfaces/crypto/CryptoSuite.h>
#include <bcos-utilities/testutils/TestPromptFixture.h>

using namespace bcostars;
using namespace bcos;
using namespace bcos::crypto;
using namespace bcos::cppsdk::utilities;

namespace bcos::test
{
BOOST_FIXTURE_TEST_SUITE(TransactionTest, TestPromptFixture)

BOOST_AUTO_TEST_CASE(test_transaction)
{
    auto txBuilder = std::make_unique<TransactionBuilder>();
    auto cryptoSuite =
        std::make_shared<bcos::crypto::CryptoSuite>(std::make_shared<bcos::crypto::Keccak256>(),
            std::make_shared<bcos::crypto::Secp256k1Crypto>(), nullptr);
    bcos::bytes input = fromHex(
        std::string("4ed3885e00000000000000000000000000000000000000000000000000000000000000200000"
                    "0000000000000000000000000000000000000000000000000000000000033132330000000000"
                    "000000000000000000000000000000000000000000000000"));
    auto txData = txBuilder->createTransactionData(
        "group0", "chain0", "0x0102e8b6fc8cdf9626fddc1c3ea8c1e79b3fce94", input, "", 509);
    txData->nonce = "77972073868959774439218594078575551136443541590072266626762998244779252502750";
    auto hash = txData->hash(cryptoSuite->hashImpl()).hex();
    BOOST_CHECK_EQUAL(hash, "db2ad0125c0a15b165016af8dfdb24d059075c2a82dc7bc3458e68d3f6bf1aee");

    auto txDataBytes = txBuilder->encodeTransactionData(*txData);
    auto txHash = txBuilder->calculateTransactionDataHash(CryptoType::Secp256K1, *txData).hex();

    BOOST_CHECK_EQUAL(txHash, "db2ad0125c0a15b165016af8dfdb24d059075c2a82dc7bc3458e68d3f6bf1aee");

    auto json = txBuilder->decodeTransactionDataToJsonObj(*txDataBytes);
    std::cout << "json: " << json << std::endl;
    BOOST_CHECK(json.find("0x0102e8b6fc8cdf9626fddc1c3ea8c1e79b3fce94") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(test_receipt)
{
    auto receiptBuilder = std::make_unique<ReceiptBuilder>();
    bcos::bytes output(bcos::asBytes(""));

    auto receiptData = receiptBuilder->createReceiptData(
        "24363", "0102e8b6fc8cdf9626fddc1c3ea8c1e79b3fce94", output, 9);

    auto cryptoSuite =
        std::make_shared<bcos::crypto::CryptoSuite>(std::make_shared<bcos::crypto::Keccak256>(),
            std::make_shared<bcos::crypto::Secp256k1Crypto>(), nullptr);

    auto hash = receiptData->hash(cryptoSuite->hashImpl()).hex();
    BOOST_CHECK_EQUAL(
        hash, "296b4598a56d6e2a295e5fe913e6c55459bef0c290f0e713744be8ade2ceec51");
    auto hash2 = receiptBuilder->calculateReceiptDataHash(CryptoType::Secp256K1, *receiptData).hex();
    BOOST_CHECK_EQUAL(hash2, "296b4598a56d6e2a295e5fe913e6c55459bef0c290f0e713744be8ade2ceec51");

    auto receiptDataByte = receiptBuilder->encodeReceipt(*receiptData);
    auto json = receiptBuilder->decodeReceiptDataToJsonObj(*receiptDataByte);
    std::cout << "json: " << json << std::endl;
    BOOST_CHECK(json.find("0102e8b6fc8cdf9626fddc1c3ea8c1e79b3fce94") != std::string::npos);
}
BOOST_AUTO_TEST_SUITE_END()
}  // namespace bcos::test