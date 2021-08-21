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
 * @file WsMessageType.h
 * @author: octopus
 * @date 2021-07-28
 */
#pragma once

namespace bcos
{
namespace ws
{
enum WsMessageType
{
    HANDESHAKE = 0x100,
    BLOCK_NOTIFY = 0x101,
    RPC_REQUEST = 0x102,

    // ------------AMOP begin ---------
    AMOP_SUBTOPIC = 0x110,
    AMOP_REQUEST = 0x111,
    AMOP_BROADCAST = 0x112,
    AMOP_RESPONSE = 0x113
    // ------------AMOP end ---------
};
}
}  // namespace bcos
