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
 * @brief test for AMOPRequest
 * @file AMOPRequestTest.cpp
 * @author: octopus
 * @date 2021-09-22
 */
#include <bcos-framework/libprotocol/amop/AMOPRequest.h>
#include <bcos-framework/testutils/TestPromptFixture.h>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <memory>

using namespace bcos;
using namespace bcos::test;


BOOST_FIXTURE_TEST_SUITE(AMOPRequestTest, TestPromptFixture)

BOOST_AUTO_TEST_CASE(test_AMOPRequest)
{
    auto requestFactory = std::make_shared<bcos::protocol::AMOPRequestFactory>();
    {
        auto request = requestFactory->buildRequest();

        auto buffer = std::make_shared<bytes>();
        auto r = request->encode(*buffer);
        BOOST_CHECK(r);

        auto decodeRequest = requestFactory->buildRequest();
        r = decodeRequest->decode(bytesConstRef(buffer->data(), buffer->size()));
        BOOST_CHECK(r);

        BOOST_CHECK_EQUAL(request->topic(), decodeRequest->topic());
        BOOST_CHECK_EQUAL(0, decodeRequest->data().size());
    }
    {
        auto request = requestFactory->buildRequest();
        std::string topic = "topic";
        std::string data(1000, 'a');

        request->setTopic(topic);
        request->setData(bytesConstRef((byte*)data.data(), data.size()));

        auto buffer = std::make_shared<bytes>();
        auto r = request->encode(*buffer);
        BOOST_CHECK(r);

        auto decodeRequest = requestFactory->buildRequest();
        auto size = decodeRequest->decode(bytesConstRef(buffer->data(), buffer->size()));
        BOOST_CHECK(size > 0);

        BOOST_CHECK_EQUAL(topic, decodeRequest->topic());
        BOOST_CHECK_EQUAL(
            data, std::string(decodeRequest->data().begin(), decodeRequest->data().end()));
    }

    {
        auto request = requestFactory->buildRequest();
        std::string topic(bcos::protocol::AMOPRequest::TOPIC_MAX_LENGTH + 1, 'a');
        std::string data(1000, 'a');

        request->setTopic(topic);
        request->setData(bytesConstRef((byte*)data.data(), data.size()));

        auto buffer = std::make_shared<bytes>();
        auto r = request->encode(*buffer);
        BOOST_CHECK(!r);
    }

    {
        auto request = requestFactory->buildRequest();
        std::string topic = "topic";
        std::string data(1000, 'a');

        request->setTopic(topic);
        request->setData(bytesConstRef((byte*)data.data(), data.size()));

        auto buffer = std::make_shared<bytes>();
        auto r = request->encode(*buffer);
        BOOST_CHECK(r);

        auto decodeRequest = requestFactory->buildRequest();
        auto size = decodeRequest->decode(
            bytesConstRef(buffer->data(), bcos::protocol::AMOPRequest::MESSAGE_MIN_LENGTH - 1));
        BOOST_CHECK(size < 0);
    }
}

BOOST_AUTO_TEST_SUITE_END()
