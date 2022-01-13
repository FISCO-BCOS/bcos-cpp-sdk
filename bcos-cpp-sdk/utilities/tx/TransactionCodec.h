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
 * @file TransactionCodec.h
 * @author: octopus
 * @date 2022-01-12
 */
#pragma once
#include <bcos-cpp-sdk/utilities/tx/TransactionCodecInterface.h>

namespace bcos
{
namespace cppsdk
{
namespace utilities
{
class TransactionCodec : public TransactionCodecInterface,
                         std::enable_shared_from_this<TransactionCodec>
{
public:
    using Ptr = std::shared_ptr<TransactionCodec>;

public:
    virtual void encode(
        const bcostars::TransactionData& _transactionData, bcos::bytes& _buffer) override;

    virtual void encodeAndSign(const bcostars::TransactionData& _transactionData, KeyPair _keyPair,
        bcos::bytes& _buffer) override;
    virtual void encodeAndSign(const bcostars::TransactionData& _transactionData, KeyPair _keyPair,
        std::string& _hexBuffer) override;
};
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos