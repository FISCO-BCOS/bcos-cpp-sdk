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

#include <bcos-cpp-sdk/utilities/crypto/hash/SM3Hash.h>
#include <bcos-utilities/Common.h>
#include <wedpr-crypto/WedprCrypto.h>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::utilities;

HashResult SM3Hash::hash(bytesConstRef _data)
{
    HashResult hashResult;
    CInputBuffer hashInput{(const char*)_data.data(), _data.size()};
    COutputBuffer hashOutput{(char*)hashResult.data(), HashResult::size};

    int8_t retCode = wedpr_sm3_hash(&hashInput, &hashOutput);
    if (retCode != WEDPR_SUCCESS)
    {
        BCOS_LOG(ERROR) << LOG_BADGE("SM3Hash") << LOG_DESC("wedpr_sm3_hash error")
                        << LOG_KV("data len", _data.size()) << LOG_KV("retCode", (int32_t)retCode);
        BOOST_THROW_EXCEPTION(
            InvalidParameter() << errinfo_comment("SM3Hash::sign wedpr_sm3_hash error"));
    }

    // Note: Due to the return value optimize of the C++ compiler, there will be no additional copy
    // overhead
    return hashResult;
}