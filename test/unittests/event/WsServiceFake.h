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
 * @file WsServiceFake.h
 * @author: octopus
 * @date 2021-09-24
 */
#pragma once
#include "libutilities/Common.h"
#include <bcos-cpp-sdk/ws/WsService.h>
#include <bcos-cpp-sdk/ws/WsSession.h>
#include <memory>

namespace bcos
{
namespace cppsdk
{
namespace test
{
class WsServiceFake : public ws::WsService
{
public:
    using Ptr = std::shared_ptr<WsServiceFake>;

public:
    virtual void asyncSendMessage(std::shared_ptr<ws::WsMessage> _msg,
        ws::Options _options = ws::Options(-1),
        ws::RespCallBack _respFunc = ws::RespCallBack()) override
    {
        (void)_msg;
        (void)_options;

        auto msg = std::make_shared<ws::WsMessage>();
        msg->setData(m_resp);
        auto session = shared_from_this();
        _respFunc(m_error, _msg, m_session);
    }

public:
    void setError(bcos::Error::Ptr _error) { m_error = _error; }
    void setResp(std::shared_ptr<bcos::bytes> _resp) { m_resp = _resp; }
    void setSession(std::shared_ptr<ws::WsSession> _session) { m_session = _session; }

private:
    bcos::Error::Ptr m_error;
    std::shared_ptr<bcos::bytes> m_resp;
    std::shared_ptr<ws::WsSession> m_session;
};
}  // namespace test
}  // namespace cppsdk
}  // namespace bcos