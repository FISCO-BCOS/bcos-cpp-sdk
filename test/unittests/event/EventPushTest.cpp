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
#include "libutilities/Common.h"
#include <bcos-cpp-sdk/event/EventPush.h>
#include <bcos-cpp-sdk/event/EventPushResponse.h>
#include <bcos-framework/testutils/TestPromptFixture.h>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <future>
#include <memory>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::test;

BOOST_FIXTURE_TEST_SUITE(EventPushTest, TestPromptFixture)

BOOST_AUTO_TEST_CASE(test_EventPush_suspendTask)
{
    auto ep = std::make_shared<bcos::cppsdk::event::EventPush>();
    auto task = std::make_shared<bcos::cppsdk::event::EventPushTask>();
    std::string id = "123";
    task->setId(id);

    auto r = ep->addSuspendTask(task);
    BOOST_CHECK(r);
    BOOST_CHECK_EQUAL(ep->suspendTasksCount(), 1);
    r = ep->addSuspendTask(task);
    BOOST_CHECK(!r);
    BOOST_CHECK_EQUAL(ep->suspendTasksCount(), 1);

    r = ep->removeSuspendTask(id);
    BOOST_CHECK(r);
    BOOST_CHECK_EQUAL(ep->suspendTasksCount(), 0);

    r = ep->removeSuspendTask(id);
    BOOST_CHECK(!r);
    BOOST_CHECK_EQUAL(ep->suspendTasksCount(), 0);
}

BOOST_AUTO_TEST_CASE(test_EventPush_addTask)
{
    auto ep = std::make_shared<bcos::cppsdk::event::EventPush>();
    auto task1 = std::make_shared<bcos::cppsdk::event::EventPushTask>();
    auto task2 = std::make_shared<bcos::cppsdk::event::EventPushTask>();

    std::string id1 = "123";
    std::string id2 = "456";
    task1->setId(id1);
    task2->setId(id2);

    {
        // addTask
        auto r = ep->addTask(task1);
        BOOST_CHECK(r);
        r = ep->addTask(task1);
        BOOST_CHECK(!r);

        // getAndRemove
        auto task = ep->getTask(id1);
        BOOST_CHECK(task);
        task = ep->getTaskAndRemove(id1);
        BOOST_CHECK(task);
        task = ep->getTask(id1);
        BOOST_CHECK(!task);
        task = ep->getTaskAndRemove(id1);
        BOOST_CHECK(!task);
    }

    {
        // addTask
        auto r = ep->addTask(task1);
        BOOST_CHECK(r);
        r = ep->addTask(task1);
        BOOST_CHECK(!r);

        // getAndRemove
        auto task = ep->getTask(id1);
        BOOST_CHECK(task);
        task = ep->getTaskAndRemove(id1);
        BOOST_CHECK(task);
        task = ep->getTask(id1);
        BOOST_CHECK(!task);
        task = ep->getTaskAndRemove(id1);
        BOOST_CHECK(!task);
    }

    {
        auto r = ep->addSuspendTask(task2);
        BOOST_CHECK(r);
        r = ep->addSuspendTask(task2);
        BOOST_CHECK(!r);
        BOOST_CHECK_EQUAL(ep->suspendTasksCount(), 1);

        auto task = ep->getTask(id2, false);
        BOOST_CHECK(!task);

        task = ep->getTask(id2);
        BOOST_CHECK(task);

        task = ep->getTaskAndRemove(id2, false);
        BOOST_CHECK(!task);

        task = ep->getTaskAndRemove(id2);
        BOOST_CHECK(task);

        BOOST_CHECK_EQUAL(ep->suspendTasksCount(), 0);
    }

    {
        auto r = ep->addSuspendTask(task2);
        BOOST_CHECK(r);
        BOOST_CHECK_EQUAL(ep->suspendTasksCount(), 1);

        r = ep->addTask(task2);
        BOOST_CHECK(r);

        BOOST_CHECK_EQUAL(ep->suspendTasksCount(), 0);
    }
}

BOOST_AUTO_TEST_CASE(test_EventPush_unsubscribeEvent)
{
    auto ep = std::make_shared<bcos::cppsdk::event::EventPush>();
    auto ioc = std::make_shared<boost::asio::io_context>();
    auto messageFactory = std::make_shared<bcos::ws::WsMessageFactory>();
    ep->setMessageFactory(messageFactory);
    ep->setIoc(ioc);

    auto task = std::make_shared<bcos::cppsdk::event::EventPushTask>();
    std::string id = "123";
    task->setId(id);

    {
        // task not exist
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
        ep->addSuspendTask(task);
        BOOST_CHECK_EQUAL(ep->suspendTasksCount(), 1);
        std::promise<bool> p;
        auto f = p.get_future();

        BOOST_CHECK(!ep->getTask(id, false));
        BOOST_CHECK_EQUAL(ep->suspendTasksCount(), 1);

        BOOST_CHECK(ep->getTask(id));
        BOOST_CHECK_EQUAL(ep->suspendTasksCount(), 1);

        ep->unsubscribeEvent(
            id, [&p](bcos::Error::Ptr _error, const std::string& _id, const std::string&) {
                (void)_error;
                (void)_id;
                BOOST_CHECK(!_error);
                p.set_value(true);
            });
        f.get();

        BOOST_CHECK(!ep->getTask(id));
        BOOST_CHECK_EQUAL(ep->suspendTasksCount(), 0);
    }

    {
        // task is running
        auto stream = boost::beast::websocket::stream<boost::beast::tcp_stream>(*ep->ioc());
        auto session = std::make_shared<bcos::cppsdk::test::WsSessionFake>(std::move(stream));

        task->setSession(session);
        ep->addTask(task);

        std::promise<bool> p;
        auto f = p.get_future();

        auto errorCode = -111;
        auto error = std::make_shared<Error>(errorCode, "event push task not found");
        session->setError(error);

        // callback error
        ep->unsubscribeEvent(id,
            [&p, errorCode](bcos::Error::Ptr _error, const std::string& _id, const std::string&) {
                (void)_id;
                BOOST_CHECK(_error);
                BOOST_CHECK_EQUAL(_error->errorCode(), errorCode);
                p.set_value(true);
            });
        f.get();

        BOOST_CHECK(!ep->getTask(id));
        BOOST_CHECK_EQUAL(ep->suspendTasksCount(), 0);
    }

    {
        // task is running
        auto stream = boost::beast::websocket::stream<boost::beast::tcp_stream>(*ep->ioc());
        auto session = std::make_shared<bcos::cppsdk::test::WsSessionFake>(std::move(stream));

        task->setSession(session);

        ep->addTask(task);

        auto resp = std::make_shared<bcos::cppsdk::event::EventPushResponse>();
        resp->setId(task->id());
        resp->setStatus(0);

        std::promise<bool> p;
        auto f = p.get_future();

        session->setError(nullptr);
        auto respJson = resp->generateJson();
        session->setResp(std::make_shared<bcos::bytes>(respJson.begin(), respJson.end()));

        ep->unsubscribeEvent(id, [&p, &respJson](bcos::Error::Ptr _error, const std::string& _id,
                                     const std::string& _resp) {
            (void)_id;
            BOOST_CHECK(!_error);
            BOOST_CHECK_EQUAL(respJson, _resp);
            p.set_value(true);
        });
        f.get();

        BOOST_CHECK(!ep->getTask(id));
        BOOST_CHECK_EQUAL(ep->suspendTasksCount(), 0);
    }
}

BOOST_AUTO_TEST_SUITE_END()