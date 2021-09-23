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
 * @brief test for EventPush
 * @file EventPushTest.cpp
 * @author: octopus
 * @date 2021-09-22
 */
#include "./WsServiceFake.h"
#include "./WsSessionFake.h"
#include <bcos-cpp-sdk/event/EventPush.h>
#include <bcos-framework/testutils/TestPromptFixture.h>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <future>
#include <memory>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::test;

BOOST_FIXTURE_TEST_SUITE(EventPushTest, TestPromptFixture)

BOOST_AUTO_TEST_CASE(test_EventPush_addTask)
{
    auto ep = std::make_shared<bcos::cppsdk::event::EventPush>();
    auto task = std::make_shared<bcos::cppsdk::event::EventPushTask>();

    {
        std::string id = "123";
        auto r = ep->addTask(id, task);
        BOOST_CHECK(r);
        r = ep->addTask(id, task);
        BOOST_CHECK(!r);

        auto task = ep->getTask(id);
        BOOST_CHECK(task);
        task = ep->getTaskAndRemove(id);
        BOOST_CHECK(task);
        task = ep->getTask(id);
        BOOST_CHECK(!task);
        task = ep->getTaskAndRemove(id);
        BOOST_CHECK(!task);
    }
}


BOOST_AUTO_TEST_CASE(test_EventPush_unsubEvent)
{
    auto ep = std::make_shared<bcos::cppsdk::event::EventPush>();
    auto task = std::make_shared<bcos::cppsdk::event::EventPushTask>();

    {
        // task not exist
        std::string id = "123";
        std::promise<bool> p;
        auto f = p.get_future();
        ep->unsubscribeEvent(
            id, [&p](bcos::Error::Ptr _error, const std::string& _id, const std::string&) {
                (void)_error;
                (void)_id;
                BOOST_CHECK(_error);
                BOOST_CHECK_EQUAL(_error->errorCode(), -1);
                p.set_value(true);
            });
        f.get();
    }

    {
        // task is suspend
        std::string id = "456";
        ep->addTask(id, task);
        std::promise<bool> p;
        auto f = p.get_future();

        ep->unsubscribeEvent(
            id, [&p](bcos::Error::Ptr _error, const std::string& _id, const std::string&) {
                (void)_error;
                (void)_id;
                BOOST_CHECK(!_error);
                p.set_value(true);
            });
        f.get();

        BOOST_CHECK(!ep->getTask(id));
    }
    /*
    {
        auto stream = std::make_shared<boost::beast::websocket::stream<boost::beast::tcp_stream>>();
        auto session = std::make_shared<bcos::cppsdk::test::WsSession>(*stream);
        task->setSession(session);

        // task is running
        std::string id = "456";
        ep->addTask(id, task);
        std::promise<bool> p;
        auto f = p.get_future();

        ep->unsubscribeEvent(
            id, [&p](bcos::Error::Ptr _error, const std::string& _id, const std::string&) {
                (void)_error;
                (void)_id;
                BOOST_CHECK(!_error);
                p.set_value(true);
            });
        f.get();

        BOOST_CHECK(!ep->getTask(id));
    }
    */
}

BOOST_AUTO_TEST_SUITE_END()