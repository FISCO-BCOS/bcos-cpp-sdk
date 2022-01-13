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
#include <json/json.h>
#include <wedpr-crypto/WedprCrypto.h>
#include <fstream>
#include <memory>
#include <utility>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::utilities;

KeyPair::Ptr KeyPairBuilder::genKeyPair(CryptoSuiteType _cryptoSuiteType)
{
    auto publicBytes = std::make_shared<bcos::bytes>(PUBLIC_KEY_LEN);
    auto privateBytes = std::make_shared<bcos::bytes>(PRIVATE_KEY_LEN);

    COutputBuffer publicKey{(char*)publicBytes->data(), publicBytes->size()};
    COutputBuffer privateKey{(char*)privateBytes->data(), privateBytes->size()};

    int8_t (*genKeyPairFunc)(COutputBuffer*, COutputBuffer*) =
        _cryptoSuiteType == CryptoSuiteType::ECDSA_TYPE ? wedpr_secp256k1_gen_key_pair :
                                                          wedpr_sm2_gen_key_pair;

    int8_t retCode = genKeyPairFunc(&publicKey, &privateKey);
    if (retCode != WEDPR_SUCCESS)
    {
        // TODO: how to handle error, throw exception???
        BCOS_LOG(ERROR) << LOG_BADGE("genKeyPair") << LOG_DESC("generator key pair error")
                        << LOG_KV("cryptoSuiteType", (int)_cryptoSuiteType)
                        << LOG_KV("retCode", (int32_t)retCode);
        return nullptr;
    }

    BCOS_LOG(DEBUG) << LOG_BADGE("genKeyPair") << LOG_DESC("generator key pair success")
                    << LOG_KV("pub len", publicKey.len) << LOG_KV("pri len", privateKey.len)
                    << LOG_KV("retCode", (int32_t)retCode);

    auto keyPair = std::make_shared<KeyPair>(_cryptoSuiteType);
    keyPair->setPrivateKey(privateBytes);
    keyPair->setPublicKey(publicBytes);

    BCOS_LOG(INFO) << LOG_BADGE("genKeyPair") << LOG_DESC("generator key pair success")
                   << LOG_KV("cryptoSuiteType", (int)_cryptoSuiteType)
                   << LOG_KV("public key", keyPair->hexPrivateKey())
                   << LOG_KV("private key", keyPair->hexPrivateKey());

    return keyPair;
}

KeyPair::Ptr KeyPairBuilder::loadKeyPair(const std::string& _keyPairPath)
{
    std::ifstream ifs(_keyPairPath);
    std::stringstream buffer;
    buffer << ifs.rdbuf();

    BCOS_LOG(INFO) << LOG_BADGE("loadKeyPair") << LOG_DESC("read file success")
                   << LOG_KV("keyPairPath", _keyPairPath) << LOG_KV("content", buffer.str());

    return loadKeyPair1(buffer.str());
}

KeyPair::Ptr KeyPairBuilder::loadKeyPair1(const std::string& _keyPairContent)
{
    Json::Value root;
    Json::Reader jsonReader;

    try
    {
        if (!jsonReader.parse(_keyPairContent, root))
        {
            BCOS_LOG(ERROR) << LOG_BADGE("loadKeyPair1") << LOG_DESC("Invalid Json Object")
                            << LOG_KV("keyPairContent", _keyPairContent);
            return nullptr;
        }

        std::string type = root["type"].asString();
        std::string priKey = root["priKey"].asString();
        std::string pubKey = root["pubKey"].asString();

        auto keyPairBuilder = std::make_shared<KeyPairBuilder>();
        auto keyPair = keyPairBuilder->genEmptyKeyPair();
        keyPair->setCryptoSuiteType(
            type == "ecdsa" ? CryptoSuiteType::ECDSA_TYPE : CryptoSuiteType::SM_TYPE);
        keyPair->setPrivateKey(fromHexString(priKey));
        keyPair->setPublicKey(fromHexString(pubKey));

        BCOS_LOG(INFO) << LOG_BADGE("loadKeyPair1") << LOG_DESC("load key pair success")
                       << LOG_KV("cryptoSuiteType", (int)keyPair->cryptoSuiteType())
                       << LOG_KV("pri", keyPair->hexPrivateKey())
                       << LOG_KV("pub", keyPair->hexPrivateKey())
                       << LOG_KV("keyPairContent", _keyPairContent);
        return keyPair;
    }
    catch (const std::exception& _e)
    {
        BCOS_LOG(ERROR) << LOG_BADGE("loadKeyPair1") << LOG_DESC("invalid json params")
                        << LOG_KV("keyPairContent", _keyPairContent)
                        << LOG_KV("error", boost::diagnostic_information(_e));
        return nullptr;
    }
}

void KeyPairBuilder::storeKeyPair(KeyPair::Ptr _keyPair, const std::string& _keyPairPath)
{
    /*
    {
    "type": "ecdsa",
    "priKey": "a",
    "pubKey": "b"
    }
    or
    {
    "type": "sm",
    "priKey": "a",
    "pubKey": "b"
    }
     */

    Json::Value jResult;
    // type
    jResult["type"] = _keyPair->cryptoSuiteType() == CryptoSuiteType::ECDSA_TYPE ? "ecdsa" : "sm";
    // private key
    jResult["priKey"] = _keyPair->hexPrivateKey();
    // public key
    jResult["pubKey"] = _keyPair->hexPublicKey();

    Json::FastWriter writer;
    std::string result = writer.write(jResult);

    std::ofstream ofs(_keyPairPath, std::ofstream::out);
    ofs << result;
    ofs.close();

    BCOS_LOG(INFO) << LOG_BADGE("storeKeyPair") << LOG_DESC("store key pair success")
                   << LOG_KV("result", result) << LOG_KV("_keyPairPath", _keyPairPath);
}