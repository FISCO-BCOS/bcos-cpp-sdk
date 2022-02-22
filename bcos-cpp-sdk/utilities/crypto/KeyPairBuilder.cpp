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
 * @file KeyPairBuilder.cpp
 * @author: octopus
 * @date 2022-01-13
 */
#include <bcos-cpp-sdk/utilities/Common.h>
#include <bcos-cpp-sdk/utilities/crypto/KeyPairBuilder.h>
#include <bcos-crypto/hash/Keccak256.h>
#include <bcos-crypto/hash/SM3.h>
#include <bcos-crypto/signature/fastsm2/FastSM2KeyPairFactory.h>
#include <bcos-crypto/signature/key/KeyFactoryImpl.h>
#include <bcos-crypto/signature/secp256k1/Secp256k1Crypto.h>
#include <bcos-utilities/BoostLog.h>
#include <bcos-utilities/DataConvertUtility.h>
#include <bcos-utilities/Exceptions.h>
#include <bcos-utilities/FileUtility.h>
#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/ossl_typ.h>
#include <openssl/pem.h>
#include <fstream>
#include <memory>
#include <utility>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::utilities;

bcos::crypto::KeyInterface::Ptr KeyPairBuilder::loadPem(
    const std::string& _pemPath, std::size_t _hexedPrivateKeySize)
{
    UTILITIES_KEYPAIR_LOG(DEBUG) << LOG_BADGE("KeyPairBuilder") << LOG_DESC("load pem")
                                 << LOG_KV("pemPath", _pemPath);

    auto keyContent = readContents(boost::filesystem::path(_pemPath));
    if (keyContent->empty())
    {
        BOOST_THROW_EXCEPTION(
            InvalidParameter() << errinfo_comment(
                "KeyPairBuilder::loadPem the pem file not exist, pem path: " + _pemPath));
    }

    std::shared_ptr<BIO> bioMem(BIO_new(BIO_s_mem()), [&](BIO* p) { BIO_free(p); });
    BIO_write(bioMem.get(), keyContent->data(), keyContent->size());

    std::shared_ptr<EVP_PKEY> evpPKey(PEM_read_bio_PrivateKey(bioMem.get(), NULL, NULL, NULL),
        [](EVP_PKEY* p) { EVP_PKEY_free(p); });
    if (!evpPKey)
    {
        BOOST_THROW_EXCEPTION(
            InvalidParameter() << errinfo_comment(
                "KeyPairBuilder::loadPem PEM_read_bio_PrivateKey error, pem: " + _pemPath));
    }

    std::shared_ptr<EC_KEY> ecKey(
        EVP_PKEY_get1_EC_KEY(evpPKey.get()), [](EC_KEY* p) { EC_KEY_free(p); });
    if (!ecKey)
    {
        BOOST_THROW_EXCEPTION(
            InvalidParameter() << errinfo_comment(
                "KeyPairBuilder::loadPem EVP_PKEY_get1_EC_KEY error, pem: " + _pemPath));
    }

    std::shared_ptr<const BIGNUM> ecPrivateKey(
        EC_KEY_get0_private_key(ecKey.get()), [](const BIGNUM*) {});
    std::shared_ptr<char> privateKeyData(
        BN_bn2hex(ecPrivateKey.get()), [](char* p) { OPENSSL_free(p); });

    std::string hexPriKey(privateKeyData.get());
    if (hexPriKey.length() < _hexedPrivateKeySize)
    {
        hexPriKey = std::string('0', _hexedPrivateKeySize - hexPriKey.length()) + hexPriKey;
    }
    auto priKeyBytes = fromHexString(hexPriKey);

    BCOS_LOG(DEBUG) << LOG_BADGE("KeyPairBuilder") << LOG_DESC("load pem private key")
                    << LOG_KV("length", priKeyBytes->size());

    auto keyFactory = std::make_shared<bcos::crypto::KeyFactoryImpl>();
    return keyFactory->createKey(*priKeyBytes);
}

/**
 * @brief
 *
 * @param _cryptoType
 * @return bcos::crypto::KeyPair::UniquePtr
 */
bcos::crypto::KeyPairInterface::UniquePtr KeyPairBuilder::genKeyPair(CryptoType _cryptoType)
{
    auto fixBytes = FixedBytes<32>().generateRandomFixedBytes();
    return genKeyPair(_cryptoType, bytesConstRef(fixBytes.data(), fixBytes.size));
}

bcos::crypto::KeyPair::UniquePtr KeyPairBuilder::genKeyPair(
    CryptoType _cryptoType, bytesConstRef _privateKey)
{
    auto keyImpl = std::make_shared<bcos::crypto::KeyImpl>(_privateKey);
    if (_cryptoType == CryptoType::Secp256K1)
    {
        bcos::crypto::Secp256k1Crypto secp256k1Crypto;
        auto keyPair = secp256k1Crypto.createKeyPair(keyImpl);

        UTILITIES_KEYPAIR_LOG(TRACE)
            << LOG_BADGE("genKeyPair") << LOG_DESC("generate new ecdsa keypair")
            << LOG_KV("address", keyPair->address(std::make_shared<bcos::crypto::Keccak256>()));

        return keyPair;
    }
    else
    {
        bcos::crypto::FastSM2KeyPairFactory fastSM2KeyPairFactory;
        auto keyPair = fastSM2KeyPairFactory.createKeyPair(keyImpl);

        UTILITIES_KEYPAIR_LOG(TRACE)
            << LOG_BADGE("genKeyPair") << LOG_DESC("generate new sm keypair")
            << LOG_KV("address", keyPair->address(std::make_shared<bcos::crypto::SM3>()));

        return keyPair;
    }
}

bcos::crypto::KeyPair::UniquePtr KeyPairBuilder::genKeyPair(
    CryptoType _cryptoType, const std::string& _pemPath)
{
    auto key = loadPem(_pemPath);
    return genKeyPair(_cryptoType, bytesConstRef((byte*)key->mutableData(), key->size()));
}