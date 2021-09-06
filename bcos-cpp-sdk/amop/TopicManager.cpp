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
 * @file TopicManager.cpp
 * @author: octopus
 * @date 2021-08-26
 */

#include <bcos-cpp-sdk/amop/TopicManager.h>
#include <json/json.h>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::amop;

void TopicManager::addTopic(const std::string& _topic)
{
    std::unique_lock lock(x_topics);
    m_topics.insert(_topic);
}
void TopicManager::addTopics(const std::set<std::string>& _topics)
{
    std::unique_lock lock(x_topics);
    m_topics.insert(_topics.begin(), _topics.end());
}
void TopicManager::removeTopic(const std::string& _topic)
{
    std::unique_lock lock(x_topics);
    m_topics.erase(_topic);
}
void TopicManager::removeTopics(const std::set<std::string>& _topics)
{
    for (auto& topic : _topics)
    {
        removeTopic(topic);
    }
}
std::set<std::string> TopicManager::topics() const
{
    std::shared_lock lock(x_topics);
    return m_topics;
}

std::string TopicManager::topicsToJsonString()
{
    auto totalTopics = topics();
    Json::Value jTopics(Json::arrayValue);
    for (const auto& topic : totalTopics)
    {
        jTopics.append(topic);
    }
    Json::Value jReq;
    jReq["topics"] = jTopics;
    Json::FastWriter writer;
    std::string request = writer.write(jReq);
    return request;
}