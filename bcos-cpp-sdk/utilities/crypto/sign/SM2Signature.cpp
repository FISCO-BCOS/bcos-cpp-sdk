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
 * @file SM2Signature.cpp
 * @author: octopus
 * @date 2022-01-13
 */

#include <bcos-cpp-sdk/utilities/crypto/sign/SM2Signature.h>
#include <wedpr-crypto/WedprCrypto.h>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::utilities;

bytesPointer SM2Signature::sign(HashResult _hash, KeyPair::Ptr _keyPair)
{
    CInputBuffer priKeyInput{(char*)_keyPair->privateKey()->data(), _keyPair->privateKey()->size()};
    CInputBuffer pubKeyInput{(char*)_keyPair->publicKey()->data(), _keyPair->publicKey()->size()};

    CInputBuffer hashInput{(const char*)_hash.data(), HashResult::size};

    FixedBytes<64> signResult;
    COutputBuffer signOutput{(char*)signResult.data(), signResult.size};

    auto retCode = wedpr_sm2_sign_fast(&priKeyInput, &pubKeyInput, &hashInput, &signOutput);
    if (retCode != WEDPR_SUCCESS)
    {
        // TODO: how to handle error, throw exception???
        BCOS_LOG(ERROR) << LOG_BADGE("SM2Sign") << LOG_DESC("wedpr_sm2_sign_fast error")
                        << LOG_KV("hash", _hash.hex()) << LOG_KV("retCode", (int32_t)retCode);
        return nullptr;
    }

    std::shared_ptr<bytes> sigData = std::make_shared<bytes>();
    *sigData = signResult.asBytes();
    return sigData;
}

bool SM2Signature::verify(
    bytesConstPtr _publicKey, HashResult _hashResult, bytesConstPtr _signature)
{
    CInputBuffer pubInput{(char*)_publicKey->data(), _publicKey->size()};
    CInputBuffer hashInput{(const char*)_hashResult.data(), HashResult::size};
    CInputBuffer sigInput{(const char*)_signature->data(), _signature->size()};

    auto verifyResult = wedpr_sm2_verify(&pubInput, &hashInput, &sigInput);
    if (verifyResult == WEDPR_SUCCESS)
    {
        return true;
    }
    return false;
}