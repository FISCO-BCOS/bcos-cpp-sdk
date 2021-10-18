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
 * @brief test for BlockNotifier
 * @file BlockNotifierTest.cpp
 * @author: octopus
 * @date 2021-10-04
 */
#include "libutilities/Common.h"
#include <bcos-cpp-sdk/group/BlockNotifier.h>
#include <bcos-framework/testutils/TestPromptFixture.h>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <future>

using namespace bcos;
using namespace bcos::cppsdk::group;
using namespace bcos::test;

BOOST_FIXTURE_TEST_SUITE(BlockNotifierTest, TestPromptFixture)

BOOST_AUTO_TEST_CASE(test_BlockInfo)
{
    std::string group = "group";
    int64_t blockNumber = 111;

    auto bi = std::make_shared<bcos::cppsdk::group::BlockInfo>();
    bi->setGroup(group);
    bi->setBlockNumber(blockNumber);

    std::string s = bi->toJson();
    bi->setGroup("");
    bi->setBlockNumber(-111);

    auto r = bi->fromJson(s);
    BOOST_CHECK_EQUAL(r, true);
    BOOST_CHECK_EQUAL(bi->group(), group);
    BOOST_CHECK_EQUAL(bi->blockNumber(), blockNumber);

    BOOST_CHECK_EQUAL(bi->fromJson("1adf"), false);
}

BOOST_AUTO_TEST_CASE(test_BlockNotifier)
{
    auto blockNotifier = std::make_shared<bcos::cppsdk::group::BlockNotifier>();

    std::string group = "group";
    int64_t globalBlockNumber1 = -1;
    int64_t globalBlockNumber2 = -1;

    blockNotifier->registerCallback(
        group, [&globalBlockNumber1](int64_t _blockNumber) { globalBlockNumber1 = _blockNumber; });
    blockNotifier->registerCallback(
        group, [&globalBlockNumber2](int64_t _blockNumber) { globalBlockNumber2 = _blockNumber; });

    int64_t globalBlockNumber = -1;
    auto r = blockNotifier->getBlockNumberByGroup(group, globalBlockNumber);
    BOOST_CHECK(!r);

    {
        std::string group = "group";
        int64_t blockNumber1 = 111;

        auto bi = std::make_shared<bcos::cppsdk::group::BlockInfo>();
        bi->setGroup(group);
        bi->setBlockNumber(blockNumber1);

        blockNotifier->onRecvBlockNotifier(bi->toJson());

        int64_t blockNumber2 = 111;
        auto r = blockNotifier->getBlockNumberByGroup(group, blockNumber2);
        BOOST_CHECK(r);
        BOOST_CHECK_EQUAL(globalBlockNumber1, 111);
        BOOST_CHECK_EQUAL(globalBlockNumber2, 111);
        BOOST_CHECK_EQUAL(blockNumber1, blockNumber2);
    }

    {
        std::string group = "group1";
        int64_t blockNumber1 = 12345;


        auto bi = std::make_shared<bcos::cppsdk::group::BlockInfo>();
        bi->setGroup(group);
        bi->setBlockNumber(blockNumber1);

        blockNotifier->onRecvBlockNotifier(bi->toJson());

        int64_t blockNumber2 = 111;
        auto r = blockNotifier->getBlockNumberByGroup(group, blockNumber2);
        BOOST_CHECK(r);
        BOOST_CHECK_EQUAL(globalBlockNumber1, 111);
        BOOST_CHECK_EQUAL(globalBlockNumber2, 111);
        BOOST_CHECK_EQUAL(blockNumber1, blockNumber2);
    }
}

BOOST_AUTO_TEST_SUITE_END()