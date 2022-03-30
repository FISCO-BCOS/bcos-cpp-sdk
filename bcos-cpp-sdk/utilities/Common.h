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
 * @file Common.h
 * @author: octopus
 * @date 2022-02-21
 */

#pragma once
#include <bcos-cpp-sdk/utilities/logger/LogInitializer.h>

#define UTILITIES_ABI_LOG(LEVEL) BCOS_LOG(LEVEL) << "[UTILITIES][ABI]"
#define UTILITIES_TX_LOG(LEVEL) BCOS_LOG(LEVEL) << "[UTILITIES][TX]"
#define UTILITIES_KEYPAIR_LOG(LEVEL) BCOS_LOG(LEVEL) << "[UTILITIES][KEYPAIR]"

namespace bcos
{
namespace cppsdk
{
namespace utilities
{
}  // namespace utilities
}  // namespace cppsdk
}  // namespace bcos