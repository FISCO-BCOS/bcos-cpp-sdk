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
 * @date 2021-08-10
 */

#pragma once
#include <bcos-boostssl/utilities/Common.h>
#include <json/json.h>

#define RPCREQ_LOG(LEVEL) BCOS_LOG(LEVEL) << "[RPC][REQUEST]"
#define RPCIMPL_LOG(LEVEL) BCOS_LOG(LEVEL) << "[RPC][IMPL]"

namespace bcos
{
namespace cppsdk
{
namespace jsonrpc
{
/**
 * @brief: jsonrpc message types
 */
enum MessageType
{
    // ------------jsonrpc begin ----------
    HANDESHAKE = 0x100,    // 256
    BLOCK_NOTIFY = 0x101,  // 257
    RPC_REQUEST = 0x102,   // 258
    GROUP_NOTIFY = 0x103,  // 259

    // ------------jsonrpc end ---------
};

struct JsonResponse
{
    struct Error
    {
        int32_t code{0};
        std::string message{"success"};

        std::string toString() const
        {
            return "{\"code\":" + std::to_string(code) + "\"message\":" + message + "}";
        }
    };
    std::string jsonrpc;
    int64_t id;
    Error error;
    Json::Value result;
};

inline Json::Value toJsonResponse(const JsonResponse& _jsonResponse)
{
    Json::Value jResp;
    jResp["jsonrpc"] = _jsonResponse.jsonrpc;
    jResp["id"] = _jsonResponse.id;

    if (_jsonResponse.error.code == 0)
    {  // success
        jResp["result"] = _jsonResponse.result;
    }
    else
    {  // error
        Json::Value jError;
        jError["code"] = _jsonResponse.error.code;
        jError["message"] = _jsonResponse.error.message;
        jResp["error"] = jError;
    }

    return jResp;
}

inline std::string toStringResponse(const JsonResponse& _jsonResponse)
{
    auto jResp = toJsonResponse(_jsonResponse);
    Json::FastWriter writer;
    std::string resp = writer.write(jResp);
    return resp;
}

}  // namespace jsonrpc
}  // namespace cppsdk
}  // namespace bcos