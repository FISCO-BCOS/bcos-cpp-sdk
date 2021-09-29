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
 * @file NodeInfo.cpp
 * @author: octopus
 * @date 2021-08-25
 */

#include <bcos-boostssl/websocket/Common.h>
#include <bcos-boostssl/websocket/NodeInfo.h>
#include <json/json.h>
#include <exception>

using namespace bcos;
using namespace bcos::ws;

bool NodeInfo::init(const std::string& _json)
{
    /*
        {
      "agency": "",
      "blockNumber": 504,
      "buildTime": "20210823 15:22:15",
      "chainID": "test_chain",
      "gitCommit": "bc5378bd399dd47c07d7d5475a7b091729833b3d",
      "groupID": "test_group",
      "nodeID":
    "147dd1ffc92644ca244f36e679a9557a02ba78ca2ba223b98077a9470162fc9f53f45589cf2389750358e252389791aa3cd1ea7a70c7b04d8e33015f55f37be1",
      "smCrypto": false,
      "supportedVersion": "",
      "version": "3.0.0",
      "wasm": false,
      "wsProtocolVersion": 1
    }*/
    try
    {
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(_json, root))
        {
            WEBSOCKET_NODEINFO(ERROR)
                << LOG_BADGE("init") << LOG_DESC("invalid json object") << LOG_KV("json", _json);
            return false;
        }

        if (root.isMember("agency"))
        {
            m_agency = root["agency"].asString();
        }

        if (root.isMember("blockNumber"))
        {
            m_blockNumber = root["blockNumber"].asInt64();
        }

        if (root.isMember("buildTime"))
        {
            m_buildTime = root["buildTime"].asString();
        }

        if (root.isMember("gitCommit"))
        {
            m_gitCommit = root["gitCommit"].asString();
        }

        if (root.isMember("chainID"))
        {
            m_chainID = root["chainID"].asString();
        }

        if (root.isMember("groupID"))
        {
            m_groupID = root["groupID"].asString();
        }

        if (root.isMember("nodeID"))
        {
            m_nodeID = root["nodeID"].asString();
        }

        if (root.isMember("smCrypto"))
        {
            m_smCrypto = root["smCrypto"].asBool();
        }

        if (root.isMember("supportedVersion"))
        {
            m_supportedVersion = root["supportedVersion"].asString();
        }

        if (root.isMember("version"))
        {
            m_version = root["version"].asString();
        }

        if (root.isMember("wasm"))
        {
            m_wasm = root["wasm"].asBool();
        }

        if (root.isMember("wsProtocolVersion"))
        {
            m_wsProtocolVersion = root["wsProtocolVersion"].asUInt();
        }

        WEBSOCKET_NODEINFO(INFO) << LOG_BADGE("init") << LOG_DESC("parse node info successfully")
                                 << LOG_KV("blockNumber", m_blockNumber)
                                 << LOG_KV("chainID", m_chainID) << LOG_KV("groupID", m_groupID)
                                 << LOG_KV("nodeID", m_nodeID) << LOG_KV("smCrypto", m_smCrypto)
                                 << LOG_KV("version", m_version)
                                 << LOG_KV("wsProtocolVersion", m_wsProtocolVersion)
                                 << LOG_KV("gitCommit", m_gitCommit)
                                 << LOG_KV("buildTime", m_buildTime);

        return true;
    }
    catch (const std::exception& e)
    {
        WEBSOCKET_NODEINFO(ERROR) << LOG_BADGE("init") << LOG_DESC("invalid json object")
                                  << LOG_KV("what", std::string(e.what())) << LOG_KV("json", _json);
        return false;
    }
}