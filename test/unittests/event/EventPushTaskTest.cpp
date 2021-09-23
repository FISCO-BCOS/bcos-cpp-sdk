/**
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
 * @brief test for EventPushTask
 * @file EventPushTaskTest.cpp
 * @author: octopus
 * @date 2021-09-22
 */

#include <bcos-cpp-sdk/event/EventPushTask.h>
#include <bcos-framework/testutils/TestPromptFixture.h>
#include <boost/core/ignore_unused.hpp>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <memory>
#include <string>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::test;

BOOST_FIXTURE_TEST_SUITE(EventPushTaskTest, TestPromptFixture)

BOOST_AUTO_TEST_CASE(test_EventPushTask)
{
    auto id = std::string("123");
    auto group = std::string("321");
    auto params = std::shared_ptr<bcos::cppsdk::event::EventPushParams>();
    auto state = std::shared_ptr<bcos::cppsdk::event::EventPushTaskState>();

    auto task = std::make_shared<bcos::cppsdk::event::EventPushTask>();
    task->setId(id);
    task->setGroup(group);
    task->setParams(params);
    task->setState(state);
    task->setCallback([](bcos::Error::Ptr _error, const std::string& _id,
                          const std::string& _resp) { boost::ignore_unused(_error, _id, _resp); });

    BOOST_CHECK_EQUAL(id, task->id());
    BOOST_CHECK_EQUAL(group, task->group());
    BOOST_CHECK_EQUAL(params, task->params());
    BOOST_CHECK_EQUAL(state, task->state());
    BOOST_CHECK(task->callback());
}


BOOST_AUTO_TEST_SUITE_END()