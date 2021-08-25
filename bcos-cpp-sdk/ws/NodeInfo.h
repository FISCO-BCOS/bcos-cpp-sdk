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
 * @file NodeInfo.h
 * @author: octopus
 * @date 2021-08-25
 */

#pragma once

#include <memory>
#include <string>
namespace bcos
{
namespace ws
{
class NodeInfo : std::enable_shared_from_this<NodeInfo>
{
public:
    using Ptr = std::shared_ptr<NodeInfo>;

    bool init(const std::string& _json);

public:
    std::string agency() const { return m_agency; }
    void setAgency(const std::string& _agency) { m_agency = _agency; }

    int64_t blockNumber() const { return m_blockNumber; }
    void setBlockNumber(int64_t _blockNumber) { m_blockNumber = _blockNumber; }

    std::string chainID() const { return m_chainID; }
    void setChainID(const std::string& _chainID) { m_chainID = _chainID; }

    std::string groupID() const { return m_groupID; }
    void setGroupID(const std::string& _groupID) { m_groupID = _groupID; }

    bool smCrypto() const { return m_smCrypto; }
    void setSmCrypto(bool _smCrypto) { m_smCrypto = _smCrypto; }

    bool wasm() const { return m_wasm; }
    void setWasm(bool _wasm) { m_wasm = _wasm; }

    uint16_t wsProtocolVersion() const { return m_wsProtocolVersion; }
    void setWsProtocolVersion(uint16_t _wsProtocolVersion)
    {
        m_wsProtocolVersion = _wsProtocolVersion;
    }

    std::string gitCommit() const { return m_gitCommit; }
    void setGitCommit(const std::string& _gitCommit) { m_gitCommit = _gitCommit; }

    std::string buildTime() const { return m_buildTime; }
    void setBuildTime(const std::string& _buildTime) { m_buildTime = _buildTime; }

    std::string version() const { return m_version; }
    void setVersion(const std::string& _version) { m_version = _version; }

    std::string supportedVersion() const { return m_supportedVersion; }
    void setSupportedVersion(const std::string& _supportedVersion)
    {
        m_supportedVersion = _supportedVersion;
    }

private:
    std::string m_agency;
    int64_t m_blockNumber;

    std::string m_chainID;
    std::string m_groupID;
    std::string m_nodeID;

    bool m_smCrypto;
    bool m_wasm;
    uint16_t m_wsProtocolVersion;

    std::string m_gitCommit;
    std::string m_buildTime;
    std::string m_version;
    std::string m_supportedVersion;
};

}  // namespace ws
}  // namespace bcos
