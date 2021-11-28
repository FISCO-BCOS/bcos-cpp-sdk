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
 * @file EvenPushParams.cpp
 * @author: octopus
 * @date 2021-09-01
 */

#include <bcos-cpp-sdk/event/EventSubParams.h>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::event;

bool EventSubParams::fromJson(const std::string &_jsonString) {

  try {
    Json::Value root;
    Json::Reader jsonReader;

    if (!jsonReader.parse(_jsonString, root)) {

      break;
    }

  } catch (const std::exception &e) {
    //
  }

  return false;
}

bool EventSubParams::fromJson(const Json::Value &jParams) {

  if (jParams.isMember("fromBlock")) {
    setFromBlock(jParams["fromBlock"].asInt64());
  }

  if (jParams.isMember("toBlock")) {
    params->setToBlock(jParams["toBlock"].asInt64());
  }

  if (jParams.isMember("addresses")) {
    auto &jAddresses = jParams["addresses"];
    for (Json::Value::ArrayIndex index = 0; index < jAddresses.size();
         ++index) {
      params->addAddress(jAddresses[index].asString());
    }
  }

  if (jParams.isMember("topics")) {
    auto &jTopics = jParams["topics"];

    for (Json::Value::ArrayIndex index = 0; index < jTopics.size(); ++index) {
      auto &jIndex = jTopics[index];
      if (jIndex.isNull()) {
        continue;
      }

      if (jIndex.isArray()) { // array topics
        for (Json::Value::ArrayIndex innerIndex = 0; innerIndex < jIndex.size();
             ++innerIndex) {
          params->addTopic(index, jIndex[innerIndex].asString());
        }
      } else { // single topic, string value
        params->addTopic(index, jIndex.asString());
      }
    }
  }
}