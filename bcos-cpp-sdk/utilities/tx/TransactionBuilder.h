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
 * @file TransactionBuilder.h
 * @author: octopus
 * @date 2022-01-13
 */
#pragma once
#include <bcos-cpp-sdk/utilities/tx/Transaction.h>
#include <bcos-cpp-sdk/utilities/tx/TransactionBuilderInterface.h>
#include <bcos-utilities/Common.h>

namespace bcos
{
namespace cppsdk
{
namespace utilities
{
class TransactionBuilder : public TransactionBuilderInterface
{
public:
    /**
     * @brief Create a Transaction object
     *
     * @param _to
     * @param _data
     * @param _chainID
     * @param _groupID
     * @param _blockLimit
     * @param _nonce
     * @return bcostars::TransactionDataPtr
     */
    virtual bcostars::TransactionDataPtr createTransaction(const std::string& _to,
        const bcos::bytes& _data, const string& _chainID, const std::string& _groupID,
        int64_t _blockLimit, const std::string& _nonce) override;

    /**
     * @brief Create a Transaction object
     *
     * @param _to
     * @param _data
     * @param _chainID
     * @param _groupID
     * @param _blockLimit
     * @return bcostars::TransactionDataPtr
     */
    virtual bcostars::TransactionDataPtr createTransaction(const std::string& _to,
        const bcos::bytes& _data, const string& _chainID, const std::string& _groupID,
        int64_t _blockLimit) override;
};
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos