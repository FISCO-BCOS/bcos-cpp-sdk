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
#include <bcos-cpp-sdk/utilities/crypto/CryptoSuite.h>
#include <bcos-cpp-sdk/utilities/crypto/KeyPairBuilder.h>
#include <bcos-utilities/BoostLog.h>
#include <bcos-utilities/Common.h>
#include <bcos-utilities/DataConvertUtility.h>
#include <bcos-utilities/Exceptions.h>
#include <bcos-utilities/FileUtility.h>
#include <json/json.h>
#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/ossl_typ.h>
#include <openssl/pem.h>
#include <wedpr-crypto/WedprCrypto.h>
#include <boost/algorithm/string/predicate.hpp>
#include <fstream>
#include <memory>
#include <utility>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::utilities;

KeyPair::UniquePtr KeyPairBuilder::genKeyPair(CryptoSuiteType _cryptoSuiteType)
{
    auto pubKeyBytes = std::make_shared<bcos::bytes>(PUBLIC_KEY_LEN);
    auto priKeyBytes = std::make_shared<bcos::bytes>(PRIVATE_KEY_LEN);

    COutputBuffer publicKey{(char*)pubKeyBytes->data(), pubKeyBytes->size()};
    COutputBuffer privateKey{(char*)priKeyBytes->data(), priKeyBytes->size()};

    int8_t (*genKeyPairFunc)(COutputBuffer*, COutputBuffer*) =
        _cryptoSuiteType == CryptoSuiteType::ECDSA_TYPE ? wedpr_secp256k1_gen_key_pair :
                                                          wedpr_sm2_gen_key_pair;

    int8_t retCode = genKeyPairFunc(&publicKey, &privateKey);
    if (retCode != WEDPR_SUCCESS)
    {
        BCOS_LOG(ERROR) << LOG_BADGE("genKeyPair") << LOG_DESC("generator key pair error")
                        << LOG_KV("cryptoSuiteType", (int)_cryptoSuiteType)
                        << LOG_KV("retCode", (int32_t)retCode);
        BOOST_THROW_EXCEPTION(InvalidParameter() << errinfo_comment(
                                  "KeyPairBuilder::genKeyPair wedpr_xxx_gen_key_pair error"));
    }

    BCOS_LOG(DEBUG) << LOG_BADGE("genKeyPair") << LOG_DESC("generator key pair success")
                    << LOG_KV("pub len", publicKey.len) << LOG_KV("pri len", privateKey.len)
                    << LOG_KV("retCode", (int32_t)retCode);

    auto keyPair = std::make_unique<KeyPair>(_cryptoSuiteType, priKeyBytes, pubKeyBytes);

    BCOS_LOG(INFO) << LOG_BADGE("genKeyPair") << LOG_DESC("generator key pair success")
                   << LOG_KV("cryptoSuiteType", (int)_cryptoSuiteType)
                   << LOG_KV("public key", keyPair->hexPrivateKey())
                   << LOG_KV("private key", keyPair->hexPrivateKey());

    return keyPair;
}

KeyPair::UniquePtr KeyPairBuilder::genKeyPair(
    CryptoSuiteType _cryptoSuiteType, bytesConstPtr _priKeyBytes)
{
    int8_t (*pubFunc)(const CInputBuffer*, COutputBuffer*) =
        _cryptoSuiteType == CryptoSuiteType::ECDSA_TYPE ? wedpr_secp256k1_derive_public_key :
                                                          wedpr_sm2_derive_public_key;

    bytesConstPtr pubKeyBytes = std::make_shared<bcos::bytes>(PUBLIC_KEY_LEN);
    COutputBuffer publicKey{(char*)pubKeyBytes->data(), pubKeyBytes->size()};
    CInputBuffer privateKey{(char*)_priKeyBytes->data(), _priKeyBytes->size()};

    auto retCode = pubFunc(&privateKey, &publicKey);
    if (retCode != WEDPR_SUCCESS)
    {
        BCOS_LOG(ERROR) << LOG_BADGE("genKeyPair") << LOG_DESC("gen key pair by private key error")
                        << LOG_KV("cryptoSuiteType", (int)_cryptoSuiteType)
                        << LOG_KV("retCode", (int32_t)retCode);
        BOOST_THROW_EXCEPTION(InvalidParameter() << errinfo_comment(
                                  "KeyPairBuilder::genKeyPair wedpr_xx_derive_public_key error"));
    }

    auto keyPair = std::make_unique<KeyPair>(_cryptoSuiteType, _priKeyBytes, pubKeyBytes);

    BCOS_LOG(INFO) << LOG_BADGE("genKeyPair") << LOG_DESC("gen key pair by private key")
                   << LOG_KV("cryptoSuiteType", (int)keyPair->cryptoSuiteType())
                   << LOG_KV("pub", keyPair->hexPrivateKey());

    return keyPair;
}

void KeyPairBuilder::storeKeyPair(KeyPair::Ptr _keyPair, const std::string& _keyPairPath)
{
    std::string path = _keyPairPath;
    if (path.empty())
    {
        auto cryptoSuite = std::make_shared<CryptoSuite>(*_keyPair);
        path = cryptoSuite->address().hexPrefixed() + ".account";
    }

    Json::Value jResult;
    // type
    jResult["type"] = _keyPair->cryptoSuiteType() == CryptoSuiteType::ECDSA_TYPE ? "ecdsa" : "sm";
    // private key
    jResult["priKey"] = _keyPair->hexPrivateKey();
    // public key
    jResult["pubKey"] = _keyPair->hexPublicKey();

    Json::FastWriter writer;
    std::string result = writer.write(jResult);

    std::ofstream ofs(path, std::ofstream::out);
    ofs << result;
    ofs.close();

    BCOS_LOG(INFO) << LOG_BADGE("storeKeyPair") << LOG_DESC("store key pair success")
                   << LOG_KV("result", result) << LOG_KV("_keyPairPath", _keyPairPath);
}

KeyPair::UniquePtr KeyPairBuilder::loadKeyPair(const std::string& _pemPath)
{
    BCOS_LOG(DEBUG) << LOG_BADGE("KeyPairBuilder::loadKeyPair") << LOG_DESC("read pem content")
                    << LOG_KV("pemPath", _pemPath);

    auto keyContent = readContents(boost::filesystem::path(_pemPath));
    if (keyContent->empty())
    {
        BOOST_THROW_EXCEPTION(
            InvalidParameter() << errinfo_comment(
                "KeyPairBuilder::loadKeyPair the pem file not exist, pem path: " + _pemPath));
    }

    std::shared_ptr<BIO> bioMem(BIO_new(BIO_s_mem()), [&](BIO* p) { BIO_free(p); });
    BIO_write(bioMem.get(), keyContent->data(), keyContent->size());

    std::shared_ptr<EVP_PKEY> evpPKey(PEM_read_bio_PrivateKey(bioMem.get(), NULL, NULL, NULL),
        [](EVP_PKEY* p) { EVP_PKEY_free(p); });
    if (!evpPKey)
    {
        BOOST_THROW_EXCEPTION(
            InvalidParameter() << errinfo_comment(
                "KeyPairBuilder::loadKeyPair PEM_read_bio_PrivateKey error, pem: " + _pemPath));
    }

    std::shared_ptr<EC_KEY> ecKey(
        EVP_PKEY_get1_EC_KEY(evpPKey.get()), [](EC_KEY* p) { EC_KEY_free(p); });
    if (!ecKey)
    {
        BOOST_THROW_EXCEPTION(
            InvalidParameter() << errinfo_comment(
                "KeyPairBuilder::loadKeyPair EVP_PKEY_get1_EC_KEY error, pem: " + _pemPath));
    }

    std::shared_ptr<const BIGNUM> ecPrivateKey(
        EC_KEY_get0_private_key(ecKey.get()), [](const BIGNUM*) {});
    std::shared_ptr<char> privateKeyData(
        BN_bn2hex(ecPrivateKey.get()), [](char* p) { OPENSSL_free(p); });

    std::string hexPriKey(privateKeyData.get());

    if (hexPriKey.size() < HEX_PRIVATE_KEY_LEN)
    {
        hexPriKey = std::string(HEX_PRIVATE_KEY_LEN - hexPriKey.size(), '0') + hexPriKey;
    }

    auto priKeyBytes = fromHexString(hexPriKey);

    const EC_GROUP* ecGroup = EC_KEY_get0_group(ecKey.get());
    int nid = EC_GROUP_get_curve_name(ecGroup);
    const char* cname = EC_curve_nid2nist(nid);
    if (cname == NULL)
    {
        cname = OBJ_nid2sn(nid);
        cname = (cname ? cname : "");
    }

    if (!(boost::iequals("sm2", cname) || boost::iequals("secp256k1", cname)))
    {
        BCOS_LOG(ERROR) << LOG_BADGE("KeyPairBuilder::loadKeyPair")
                        << LOG_DESC("unsupported private key format") << LOG_KV("pemPath", _pemPath)
                        << LOG_KV("cname", cname);

        // unrecognized private key
        BOOST_THROW_EXCEPTION(
            InvalidParameter() << errinfo_comment(
                "KeyPairBuilder::loadKeyPair unsupported privatekey format , cname: " +
                std::string(cname)));
    }

    auto cryptoSuiteType =
        boost::equals("secp256k1", cname) ? CryptoSuiteType::ECDSA_TYPE : CryptoSuiteType::SM_TYPE;

    auto keyPairBuilder = std::make_unique<KeyPairBuilder>();
    auto keyPair = keyPairBuilder->genKeyPair(cryptoSuiteType, priKeyBytes);
    auto cryptoSuite = std::make_shared<CryptoSuite>(*keyPair);

    BCOS_LOG(INFO) << LOG_BADGE("KeyPairBuilder") << LOG_DESC("loadKeyPair success")
                   << LOG_KV("pemPath", _pemPath) << LOG_KV("cname", cname)
                   << LOG_KV("pubKey", keyPair->hexPublicKey())
                   << LOG_KV("address", cryptoSuite->address().hexPrefixed());

    return keyPair;
}