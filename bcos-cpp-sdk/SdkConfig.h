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
 * @file Config.h
 * @author: octopus
 * @date 2021-08-23
 */
#pragma once

#include <bcos-framework/libutilities/Log.h>
#include <boost/asio/ip/tcp.hpp>
#include <set>

#include <bcos-boostssl/network/Common.h>
namespace bcos
{
namespace cppsdk
{
struct EndPoint
{
    std::string host;
    uint16_t port;

    bool operator<(const EndPoint& rhs) const
    {
        if (host + std::to_string(port) < rhs.host + std::to_string(rhs.port))
        {
            return true;
        }
        return false;
    }
};
class SdkConfig
{
private:
    std::set<EndPoint> m_peers;
    uint32_t m_reconnectPeriod{10000};
    // uint32_t m_threadPoolSize{4};

public:
    uint32_t reconnectPeriod() const { return m_reconnectPeriod; }
    void setReconnectPeriod(uint32_t _reconnectPeriod) { m_reconnectPeriod = _reconnectPeriod; }
    const std::set<EndPoint>& peers() { return m_peers; }
    void setPeers(const std::set<EndPoint> _peers) { m_peers = _peers; }

public:
    void initConfig(const std::string& _configPath);
};

}  // namespace cppsdk
}  // namespace bcos
