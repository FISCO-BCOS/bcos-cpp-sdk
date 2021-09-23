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
 * @brief test for EventPushTaskState
 * @file EventPushTaskStateTest.cpp
 * @author: octopus
 * @date 2021-09-22
 */
#define BOOST_TEST_MAIN

#include <bcos-cpp-sdk/event/EventPushTask.h>
#include <bcos-framework/testutils/TestPromptFixture.h>
#include <boost/test/unit_test.hpp>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::test;

BOOST_FIXTURE_TEST_SUITE(EventPushTaskStateTest, TestPromptFixture)

BOOST_AUTO_TEST_CASE(test_EventPushTaskStateTest)
{
    auto state = std::make_shared<bcos::cppsdk::event::EventPushTaskState>();
    BOOST_CHECK(state->currentBlockNumber() < 0);
    int64_t blockNumber = 10;
    state->setCurrentBlockNumber(blockNumber);
    BOOST_CHECK_EQUAL(state->currentBlockNumber(), blockNumber);
}

BOOST_AUTO_TEST_SUITE_END()