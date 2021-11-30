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
 * @file ProtocolVersionTest.cpp
 * @author: octopus
 * @date 2021-10-26
 */

#include <bcos-boostssl/utilities/Common.h>
#include <bcos-cpp-sdk/ws/Common.h>
#include <bcos-cpp-sdk/ws/ProtocolVersion.h>
#include <bcos-framework/testutils/TestPromptFixture.h>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <future>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::service;

using namespace bcos;
using namespace bcos::test;

BOOST_FIXTURE_TEST_SUITE(ProtocolVersionTest, TestPromptFixture)

BOOST_AUTO_TEST_CASE(test_ProtocolVersion)
{
    {
        int protocolVersion = 111;
        auto pv = std::make_shared<bcos::cppsdk::service::ProtocolVersion>();
        pv->setProtocolVersion(protocolVersion);
        BOOST_CHECK_EQUAL(pv->protocolVersion(), protocolVersion);
    }

    {
        int protocolVersion = 111;
        auto pv = std::make_shared<bcos::cppsdk::service::ProtocolVersion>();
        pv->setProtocolVersion(protocolVersion);
        auto s = pv->toJsonString();

        auto pv0 = std::make_shared<bcos::cppsdk::service::ProtocolVersion>();
        auto r = pv0->fromJson(s);

        BOOST_CHECK_EQUAL(r, true);
        BOOST_CHECK_EQUAL(pv0->protocolVersion(), protocolVersion);
    }

    {
        auto pv = std::make_shared<bcos::cppsdk::service::ProtocolVersion>();
        BOOST_CHECK_EQUAL(pv->fromJson("1adf"), false);
    }
}

BOOST_AUTO_TEST_SUITE_END()