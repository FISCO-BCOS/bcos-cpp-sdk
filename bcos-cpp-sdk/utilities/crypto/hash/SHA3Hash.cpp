/**
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
 * @brief interfaces for Hash
 * @file SM3Hash.cpp
 * @author: octopuswang
 * @date 2022-01-13
 */

#include <bcos-cpp-sdk/utilities/crypto/hash/SHA3Hash.h>
#include <bcos-utilities/Common.h>
#include <wedpr-crypto/WedprCrypto.h>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::utilities;

HashResult SHA3Hash::hash(bytesConstRef _data)
{
    HashResult hashResult;
    CInputBuffer hashInput{(const char*)_data.data(), _data.size()};
    COutputBuffer hashOutput{(char*)hashResult.data(), HashResult::size};

    int8_t retCode = wedpr_sha3_hash(&hashInput, &hashOutput);
    if (retCode != WEDPR_SUCCESS)
    {
        // TODO: how to handle error, throw exception???
        BCOS_LOG(ERROR) << LOG_BADGE("SHA3Hash") << LOG_DESC("wedpr_sha3_hash error")
                        << LOG_KV("data len", _data.size()) << LOG_KV("retCode", (int32_t)retCode);
        return hashResult;
    }

    // Note: Due to the return value optimize of the C++ compiler, there will be no additional copy
    // overhead
    return hashResult;
}