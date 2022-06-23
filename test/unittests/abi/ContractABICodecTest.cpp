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
 * @file ContractABICodecTest.cpp
 * @author: octopus
 * @date 2022-05-30
 */

#include <bcos-cpp-sdk/utilities/abi/ContractABICodec.h>
#include <bcos-cpp-sdk/utilities/abi/ContractABIDefinitionFactory.h>
#include <bcos-cpp-sdk/utilities/abi/ContractABIType.h>
#include <bcos-crypto/hash/Keccak256.h>
#include <bcos-crypto/interfaces/crypto/Hash.h>
#include <bcos-utilities/Base64.h>
#include <bcos-utilities/Common.h>
#include <bcos-utilities/DataConvertUtility.h>
#include <bcos-utilities/FixedBytes.h>
#include <bcos-utilities/testutils/TestPromptFixture.h>
#include <boost/test/tools/old/interface.hpp>
#include <cstddef>
#include <exception>
#include <stdexcept>
#include <utility>

using namespace bcos;
using namespace bcos::cppsdk;
using namespace bcos::cppsdk::abi;
using namespace bcos::test;

BOOST_FIXTURE_TEST_SUITE(ContractABICodecTest, TestPromptFixture)

BOOST_AUTO_TEST_CASE(test_initAbstractTypeWithJsonParams)
{
    auto solcImpl = std::make_shared<ContractABITypeCodecSolImpl>();
    ContractABICodec codec(nullptr, solcImpl);

    // uint
    {
        std::string params = "[\"111111111111111111111111111111111111111111\"]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto u = Uint::newValue();
        sPtr->addMember(std::move(u));

        BOOST_CHECK_EQUAL(sPtr->getTypeAsString(), "(uint256)");

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::UINT);
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Uint>(sPtr->value()[0])->value(),
            u256("111111111111111111111111111111111111111111"));

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL("0000000000000000000000000000014686b59ab3939851acf5c2e071c71c71c7",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // uint[3]
    {
        std::string params = "[[\"1\",2,\"3\"]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto fixedList = bcos::cppsdk::abi::FixedList::newValue(3);

        auto u0 = Uint::newValue();
        auto u1 = Uint::newValue();
        auto u2 = Uint::newValue();

        fixedList->addMember(std::move(u0));
        fixedList->addMember(std::move(u1));
        fixedList->addMember(std::move(u2));

        BOOST_CHECK_EQUAL(fixedList->getTypeAsString(), "uint256[3]");

        sPtr->addMember(std::move(fixedList));
        BOOST_CHECK_EQUAL(sPtr->getTypeAsString(), "(uint256[3])");

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::FIXED_LIST);

        auto p = std::dynamic_pointer_cast<FixedList>(sPtr->value()[0]);
        BOOST_CHECK_EQUAL(p->dimension(), 3);
        BOOST_CHECK_EQUAL(p->value().size(), 3);

        BOOST_CHECK(p->value()[0]->type() == Type::UINT);
        BOOST_CHECK(p->value()[1]->type() == Type::UINT);
        BOOST_CHECK(p->value()[2]->type() == Type::UINT);

        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Uint>(p->value()[0])->value(), u256(1));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Uint>(p->value()[1])->value(), u256(2));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Uint>(p->value()[2])->value(), u256(3));

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000010000000000000000000000"
            "00000000000000000000000000000000000000000200000000000000000000000000000000000000000000"
            "00000000000000000003",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);

        BOOST_CHECK_EQUAL(sPtr->toJsonString(), "[[\"1\",\"2\",\"3\"]]");
    }

    {
        // uint[] empty list
        std::string params = "[[]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto dyList = bcos::cppsdk::abi::DynamicList::newValue();
        sPtr->addMember(std::move(dyList));

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000"
            "000000000000000000000000000000000000000000",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    {  // uint[]
        std::string params = "[[\"1\",2,\"3\",4,\"5\",6,\"7\",8,\"9\",0]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto dyList = bcos::cppsdk::abi::DynamicList::newValue();

        auto u0 = Uint::newValue();
        dyList->addMember(std::move(u0));

        sPtr->addMember(std::move(dyList));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::DYNAMIC_LIST);

        auto p = std::dynamic_pointer_cast<DynamicList>(sPtr->value()[0]);
        BOOST_CHECK_EQUAL(p->value().size(), 10);

        BOOST_CHECK(p->value()[0]->type() == Type::UINT);
        BOOST_CHECK(p->value()[1]->type() == Type::UINT);
        BOOST_CHECK(p->value()[2]->type() == Type::UINT);
        BOOST_CHECK(p->value()[3]->type() == Type::UINT);
        BOOST_CHECK(p->value()[4]->type() == Type::UINT);
        BOOST_CHECK(p->value()[5]->type() == Type::UINT);
        BOOST_CHECK(p->value()[6]->type() == Type::UINT);
        BOOST_CHECK(p->value()[7]->type() == Type::UINT);
        BOOST_CHECK(p->value()[8]->type() == Type::UINT);
        BOOST_CHECK(p->value()[9]->type() == Type::UINT);

        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Uint>(p->value()[0])->value(), u256(1));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Uint>(p->value()[1])->value(), u256(2));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Uint>(p->value()[2])->value(), u256(3));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Uint>(p->value()[3])->value(), u256(4));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Uint>(p->value()[4])->value(), u256(5));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Uint>(p->value()[5])->value(), u256(6));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Uint>(p->value()[6])->value(), u256(7));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Uint>(p->value()[7])->value(), u256(8));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Uint>(p->value()[8])->value(), u256(9));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Uint>(p->value()[9])->value(), u256(0));

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000"
            "00000000000000000000000000000000000000000a00000000000000000000000000000000000000000000"
            "00000000000000000001000000000000000000000000000000000000000000000000000000000000000200"
            "00000000000000000000000000000000000000000000000000000000000003000000000000000000000000"
            "00000000000000000000000000000000000000040000000000000000000000000000000000000000000000"
            "00000000000000000500000000000000000000000000000000000000000000000000000000000000060000"
            "00000000000000000000000000000000000000000000000000000000000700000000000000000000000000"
            "00000000000000000000000000000000000008000000000000000000000000000000000000000000000000"
            "00000000000000090000000000000000000000000000000000000000000000000000000000000000",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(),
            "[[\"1\",\"2\",\"3\",\"4\",\"5\",\"6\",\"7\",\"8\",\"9\",\"0\"]]");
    }

    // int
    {
        std::string params = "[\"-11111111111111111111111\"]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto i = Int::newValue();
        sPtr->addMember(std::move(i));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::INT);
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Int>(sPtr->value()[0])->value(),
            s256("-11111111111111111111111"));

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL("fffffffffffffffffffffffffffffffffffffffffffffda5aa5b91a256638e39",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // int[3]
    {
        std::string params = "[[-1,\"-111111111111111111111111111111111\",3]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto fixedList = bcos::cppsdk::abi::FixedList::newValue(3);

        auto u0 = Int::newValue();
        auto u1 = Int::newValue();
        auto u2 = Int::newValue();

        fixedList->addMember(std::move(u0));
        fixedList->addMember(std::move(u1));
        fixedList->addMember(std::move(u2));

        sPtr->addMember(std::move(fixedList));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::FIXED_LIST);

        auto p = std::dynamic_pointer_cast<FixedList>(sPtr->value()[0]);
        BOOST_CHECK_EQUAL(p->dimension(), 3);
        BOOST_CHECK_EQUAL(p->value().size(), 3);

        BOOST_CHECK(p->value()[0]->type() == Type::INT);
        BOOST_CHECK(p->value()[1]->type() == Type::INT);
        BOOST_CHECK(p->value()[2]->type() == Type::INT);

        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Int>(p->value()[0])->value(), s256(-1));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Int>(p->value()[1])->value(),
            s256("-111111111111111111111111111111111"));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Int>(p->value()[2])->value(), s256(3));

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
            "fffffffffffffffa8594a30cb6c0ce125438e38e3900000000000000000000000000000000000000000000"
            "00000000000000000003",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(
            sPtr->toJsonString(), "[[\"-1\",\"-111111111111111111111111111111111\",\"3\"]]");
    }

    // int[]
    {
        std::string params = "[[\"-1\",\"-2\",\"-3\",\"-4\",\"-5\",\"6\",\"7\",\"8\",\"9\",\"0\"]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto dyList = bcos::cppsdk::abi::DynamicList::newValue();

        auto u0 = Int::newValue();
        dyList->addMember(std::move(u0));

        sPtr->addMember(std::move(dyList));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::DYNAMIC_LIST);

        auto p = std::dynamic_pointer_cast<DynamicList>(sPtr->value()[0]);
        BOOST_CHECK_EQUAL(p->value().size(), 10);

        BOOST_CHECK(p->value()[0]->type() == Type::INT);
        BOOST_CHECK(p->value()[1]->type() == Type::INT);
        BOOST_CHECK(p->value()[2]->type() == Type::INT);
        BOOST_CHECK(p->value()[3]->type() == Type::INT);
        BOOST_CHECK(p->value()[4]->type() == Type::INT);
        BOOST_CHECK(p->value()[5]->type() == Type::INT);
        BOOST_CHECK(p->value()[6]->type() == Type::INT);
        BOOST_CHECK(p->value()[7]->type() == Type::INT);
        BOOST_CHECK(p->value()[8]->type() == Type::INT);
        BOOST_CHECK(p->value()[9]->type() == Type::INT);

        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Int>(p->value()[0])->value(), s256(-1));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Int>(p->value()[1])->value(), s256(-2));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Int>(p->value()[2])->value(), s256(-3));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Int>(p->value()[3])->value(), s256(-4));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Int>(p->value()[4])->value(), s256(-5));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Int>(p->value()[5])->value(), s256(6));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Int>(p->value()[6])->value(), s256(7));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Int>(p->value()[7])->value(), s256(8));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Int>(p->value()[8])->value(), s256(9));
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Int>(p->value()[9])->value(), s256(0));

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000"
            "00000000000000000000000000000000000000000affffffffffffffffffffffffffffffffffffffffffff"
            "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffeff"
            "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffdffffffffffffffffffffffff"
            "fffffffffffffffffffffffffffffffffffffffcffffffffffffffffffffffffffffffffffffffffffffff"
            "fffffffffffffffffb00000000000000000000000000000000000000000000000000000000000000060000"
            "00000000000000000000000000000000000000000000000000000000000700000000000000000000000000"
            "00000000000000000000000000000000000008000000000000000000000000000000000000000000000000"
            "00000000000000090000000000000000000000000000000000000000000000000000000000000000",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // bool
    {
        std::string params = "[false]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto b = Boolean::newValue();
        sPtr->addMember(std::move(b));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::BOOL);
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Boolean>(sPtr->value()[0])->value(), false);

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL("0000000000000000000000000000000000000000000000000000000000000000",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // bool[3]
    {
        std::string params = "[[true,false,true]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto fixedList = bcos::cppsdk::abi::FixedList::newValue(3);

        auto u0 = Boolean::newValue();
        auto u1 = Boolean::newValue();
        auto u2 = Boolean::newValue();

        fixedList->addMember(std::move(u0));
        fixedList->addMember(std::move(u1));
        fixedList->addMember(std::move(u2));

        sPtr->addMember(std::move(fixedList));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::FIXED_LIST);

        auto p = std::dynamic_pointer_cast<FixedList>(sPtr->value()[0]);
        BOOST_CHECK_EQUAL(p->dimension(), 3);
        BOOST_CHECK_EQUAL(p->value().size(), 3);

        BOOST_CHECK(p->value()[0]->type() == Type::BOOL);
        BOOST_CHECK(p->value()[1]->type() == Type::BOOL);
        BOOST_CHECK(p->value()[2]->type() == Type::BOOL);

        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Boolean>(p->value()[0])->value(), true);
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Boolean>(p->value()[1])->value(), false);
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Boolean>(p->value()[2])->value(), true);

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000010000000000000000000000"
            "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
            "00000000000000000001",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // bool[]
    {
        std::string params = "[[true,false,true]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto dyList = bcos::cppsdk::abi::DynamicList::newValue();

        auto u0 = Boolean::newValue();
        dyList->addMember(std::move(u0));

        sPtr->addMember(std::move(dyList));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::DYNAMIC_LIST);

        auto p = std::dynamic_pointer_cast<DynamicList>(sPtr->value()[0]);
        BOOST_CHECK_EQUAL(p->value().size(), 3);

        BOOST_CHECK(p->value()[0]->type() == Type::BOOL);
        BOOST_CHECK(p->value()[1]->type() == Type::BOOL);
        BOOST_CHECK(p->value()[2]->type() == Type::BOOL);


        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Boolean>(p->value()[0])->value(), true);
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Boolean>(p->value()[1])->value(), false);
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Boolean>(p->value()[2])->value(), true);

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000"
            "00000000000000000000000000000000000000000300000000000000000000000000000000000000000000"
            "00000000000000000001000000000000000000000000000000000000000000000000000000000000000000"
            "00000000000000000000000000000000000000000000000000000000000001",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // address
    {
        std::string params = "[\"0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338\"]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto addr = Addr::newValue();
        sPtr->addMember(std::move(addr));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::ADDRESS);
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Addr>(sPtr->value()[0])->value(),
            "0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL("000000000000000000000000be5422d15f39373eb0a97ff8c10fbd0e40e29338",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // address[3]
    {
        std::string params =
            "[[\"0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338\","
            "\"0x0000000000000000000000000000000000000001\","
            "\"0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338\"]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto fixedList = bcos::cppsdk::abi::FixedList::newValue(3);

        auto u0 = Addr::newValue("0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");
        auto u1 = Addr::newValue("0x0000000000000000000000000000000000000001");
        auto u2 = Addr::newValue("0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");

        fixedList->addMember(std::move(u0));
        fixedList->addMember(std::move(u1));
        fixedList->addMember(std::move(u2));

        sPtr->addMember(std::move(fixedList));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::FIXED_LIST);

        auto p = std::dynamic_pointer_cast<FixedList>(sPtr->value()[0]);
        BOOST_CHECK_EQUAL(p->dimension(), 3);
        BOOST_CHECK_EQUAL(p->value().size(), 3);

        BOOST_CHECK(p->value()[0]->type() == Type::ADDRESS);
        BOOST_CHECK(p->value()[1]->type() == Type::ADDRESS);
        BOOST_CHECK(p->value()[2]->type() == Type::ADDRESS);

        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Addr>(p->value()[0])->value(),
            "0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Addr>(p->value()[1])->value(),
            "0x0000000000000000000000000000000000000001");
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Addr>(p->value()[2])->value(),
            "0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "000000000000000000000000be5422d15f39373eb0a97ff8c10fbd0e40e293380000000000000000000000"
            "000000000000000000000000000000000000000001000000000000000000000000be5422d15f39373eb0a9"
            "7ff8c10fbd0e40e29338",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // address[]
    {
        std::string params =
            "[[\"0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338\","
            "\"0x0000000000000000000000000000000000000001\","
            "\"0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338\"]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto fixedList = bcos::cppsdk::abi::DynamicList::newValue();

        auto u0 = Addr::newValue("0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");
        auto u1 = Addr::newValue("0x0000000000000000000000000000000000000001");
        auto u2 = Addr::newValue("0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");

        fixedList->addMember(std::move(u0));

        sPtr->addMember(std::move(fixedList));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::DYNAMIC_LIST);

        auto p = std::dynamic_pointer_cast<DynamicList>(sPtr->value()[0]);
        BOOST_CHECK_EQUAL(p->value().size(), 3);

        BOOST_CHECK(p->value()[0]->type() == Type::ADDRESS);
        BOOST_CHECK(p->value()[1]->type() == Type::ADDRESS);
        BOOST_CHECK(p->value()[2]->type() == Type::ADDRESS);

        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Addr>(p->value()[0])->value(),
            "0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Addr>(p->value()[1])->value(),
            "0x0000000000000000000000000000000000000001");
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<Addr>(p->value()[2])->value(),
            "0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000"
            "000000000000000000000000000000000000000003000000000000000000000000be5422d15f39373eb0a9"
            "7ff8c10fbd0e40e29338000000000000000000000000000000000000000000000000000000000000000100"
            "0000000000000000000000be5422d15f39373eb0a97ff8c10fbd0e40e29338",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // bytes
    {
        std::string s = "0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338";
        auto hex = *toHexString(s);
        std::string params = "[\"hex://" + hex + "\"]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto dyBytes = DynamicBytes::newValue();
        sPtr->addMember(std::move(dyBytes));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::DYNAMIC_BYTES);
        // BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<DynamicBytes>(sPtr->value()[0])->value(),
        //     "0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000"
            "00000000000000000000000000000000000000002a30786265353432326431356633393337336562306139"
            "376666386331306662643065343065323933333800000000000000000000000000000000000000000000",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // bytes[3]
    {
        std::string s0 = "";
        std::string s1 = "0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338";
        std::string s2 = std::string(100, 'a');
        auto hex0 = *toHexString(s0);
        auto hex1 = *toHexString(s1);
        auto hex2 = *toHexString(s2);

        std::string params =
            "[[\"hex://" + hex0 + "\",\"hex://" + hex1 + "\",\"hex://" + hex2 + "\"]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();

        auto fixedList = bcos::cppsdk::abi::FixedList::newValue(3);
        auto dyBytes0 = DynamicBytes::newValue();
        auto dyBytes1 = DynamicBytes::newValue();
        auto dyBytes2 = DynamicBytes::newValue();
        fixedList->addMember(std::move(dyBytes0));
        fixedList->addMember(std::move(dyBytes1));
        fixedList->addMember(std::move(dyBytes2));

        sPtr->addMember(std::move(fixedList));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::FIXED_LIST);
        // BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<DynamicBytes>(sPtr->value()[0])->value(),
        //     "0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000"
            "00000000000000000000000000000000000000006000000000000000000000000000000000000000000000"
            "0000000000000000008000000000000000000000000000000000000000000000000000000000000000e000"
            "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
            "000000000000000000000000000000000000002a3078626535343232643135663339333733656230613937"
            "66663863313066626430653430653239333338000000000000000000000000000000000000000000000000"
            "00000000000000000000000000000000000000000000000000000000006461616161616161616161616161"
            "61616161616161616161616161616161616161616161616161616161616161616161616161616161616161"
            "61616161616161616161616161616161616161616161616161616161616161616161616161616161616161"
            "6100000000000000000000000000000000000000000000000000000000",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // bytes[]
    {
        std::string s0 = "";
        std::string s1 = "0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338";
        std::string s2 = std::string(100, 'a');
        auto hex0 = *toHexString(s0);
        auto hex1 = *toHexString(s1);
        auto hex2 = *toHexString(s2);

        std::string params =
            "[[\"hex://" + hex0 + "\",\"hex://" + hex1 + "\",\"hex://" + hex2 + "\"]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();

        auto fixedList = bcos::cppsdk::abi::DynamicList::newValue();
        auto dyBytes0 = DynamicBytes::newValue();
        auto dyBytes1 = DynamicBytes::newValue();
        auto dyBytes2 = DynamicBytes::newValue();
        fixedList->addMember(std::move(dyBytes0));
        fixedList->addMember(std::move(dyBytes1));
        fixedList->addMember(std::move(dyBytes2));

        sPtr->addMember(std::move(fixedList));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::DYNAMIC_LIST);
        // BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<DynamicBytes>(sPtr->value()[0])->value(),
        //     "0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000"
            "00000000000000000000000000000000000000000300000000000000000000000000000000000000000000"
            "00000000000000000060000000000000000000000000000000000000000000000000000000000000008000"
            "000000000000000000000000000000000000000000000000000000000000e0000000000000000000000000"
            "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
            "00000000000000002a30786265353432326431356633393337336562306139376666386331306662643065"
            "34306532393333380000000000000000000000000000000000000000000000000000000000000000000000"
            "00000000000000000000000000000000000064616161616161616161616161616161616161616161616161"
            "61616161616161616161616161616161616161616161616161616161616161616161616161616161616161"
            "61616161616161616161616161616161616161616161616161616161616161616100000000000000000000"
            "000000000000000000000000000000000000",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // bytes1
    {
        std::string s = "0";
        auto hex = *toHexString(s);
        std::string params = "[\"hex://" + hex + "\"]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto fixedBytes = bcos::cppsdk::abi::FixedBytes::newValue(1);
        sPtr->addMember(std::move(fixedBytes));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::FIXED_BYTES);
        // BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<DynamicBytes>(sPtr->value()[0])->value(),
        //     "0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL("3000000000000000000000000000000000000000000000000000000000000000",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // byte20
    {
        std::string s = "01234567890123456789";
        auto base64 = base64Encode(s);
        std::string params = "[\"base64://" + base64 + "\"]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto fixedBytes = bcos::cppsdk::abi::FixedBytes::newValue(20);
        sPtr->addMember(std::move(fixedBytes));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::FIXED_BYTES);
        // BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<DynamicBytes>(sPtr->value()[0])->value(),
        //     "0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL("3031323334353637383930313233343536373839000000000000000000000000",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), "[\"hex://" + *toHexString(s) + "\"]");
    }

    // string empty
    {
        std::string params = "[\"\"]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto str = String::newValue();
        sPtr->addMember(std::move(str));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::STRING);
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<String>(sPtr->value()[0])->value(), "");

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000"
            "000000000000000000000000000000000000000000",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // string
    {
        std::string params = "[\"0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338\"]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto str = String::newValue();
        sPtr->addMember(std::move(str));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::STRING);
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<String>(sPtr->value()[0])->value(),
            "0xbe5422d15f39373eb0a97ff8c10fbd0e40e29338");

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000"
            "00000000000000000000000000000000000000002a30786265353432326431356633393337336562306139"
            "376666386331306662643065343065323933333800000000000000000000000000000000000000000000",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // string[3]
    {
        std::string params =
            "[[\"394c2288b340d0385d4b6fbbdfe5e3f8  "
            "src/main/resources/META-INF/native/bcos-sdk-jni.dll\","
            "\"73683843d97477ebe342e53c87c40ee5  "
            "src/main/resources/META-INF/native/libbcos-sdk-jni.dylib\","
            "\"\"]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto fixedList = bcos::cppsdk::abi::FixedList::newValue(3);

        auto u0 = String::newValue(
            "394c2288b340d0385d4b6fbbdfe5e3f8  "
            "src/main/resources/META-INF/native/bcos-sdk-jni.dll");
        auto u1 = String::newValue(
            "73683843d97477ebe342e53c87c40ee5  "
            "src/main/resources/META-INF/native/libbcos-sdk-jni.dylib");
        auto u2 = String::newValue("");

        fixedList->addMember(std::move(u0));
        fixedList->addMember(std::move(u1));
        fixedList->addMember(std::move(u2));

        sPtr->addMember(std::move(fixedList));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::FIXED_LIST);

        auto p = std::dynamic_pointer_cast<FixedList>(sPtr->value()[0]);
        BOOST_CHECK_EQUAL(p->dimension(), 3);
        BOOST_CHECK_EQUAL(p->value().size(), 3);

        BOOST_CHECK(p->value()[0]->type() == Type::STRING);
        BOOST_CHECK(p->value()[1]->type() == Type::STRING);
        BOOST_CHECK(p->value()[2]->type() == Type::STRING);

        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<String>(p->value()[0])->value(),
            "394c2288b340d0385d4b6fbbdfe5e3f8  "
            "src/main/resources/META-INF/native/bcos-sdk-jni.dll");
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<String>(p->value()[1])->value(),
            "73683843d97477ebe342e53c87c40ee5  "
            "src/main/resources/META-INF/native/libbcos-sdk-jni.dylib");
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<String>(p->value()[2])->value(), "");

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000"
            "00000000000000000000000000000000000000006000000000000000000000000000000000000000000000"
            "000000000000000000e0000000000000000000000000000000000000000000000000000000000000016000"
            "00000000000000000000000000000000000000000000000000000000000055333934633232383862333430"
            "643033383564346236666262646665356533663820207372632f6d61696e2f7265736f75726365732f4d45"
            "54412d494e462f6e61746976652f62636f732d73646b2d6a6e692e646c6c00000000000000000000000000"
            "00000000000000000000000000000000000000000000000000000000005a37333638333834336439373437"
            "3765626533343265353363383763343065653520207372632f6d61696e2f7265736f75726365732f4d4554"
            "412d494e462f6e61746976652f6c696262636f732d73646b2d6a6e692e64796c6962000000000000000000"
            "0000000000000000000000000000000000000000000000000000000000",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // string[]
    {
        std::string params =
            "[[\"394c2288b340d0385d4b6fbbdfe5e3f8  "
            "src/main/resources/META-INF/native/bcos-sdk-jni.dll\","
            "\"73683843d97477ebe342e53c87c40ee5  "
            "src/main/resources/META-INF/native/libbcos-sdk-jni.dylib\","
            "\"\"]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto fixedList = bcos::cppsdk::abi::DynamicList::newValue();

        auto u0 = String::newValue(
            "394c2288b340d0385d4b6fbbdfe5e3f8  "
            "src/main/resources/META-INF/native/bcos-sdk-jni.dll");
        auto u1 = String::newValue(
            "73683843d97477ebe342e53c87c40ee5  "
            "src/main/resources/META-INF/native/libbcos-sdk-jni.dylib");
        auto u2 = String::newValue("");


        fixedList->addMember(std::move(u0));

        sPtr->addMember(std::move(fixedList));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        BOOST_CHECK(sPtr->value()[0]->type() == Type::DYNAMIC_LIST);

        auto p = std::dynamic_pointer_cast<DynamicList>(sPtr->value()[0]);
        BOOST_CHECK_EQUAL(p->value().size(), 3);

        BOOST_CHECK(p->value()[0]->type() == Type::STRING);
        BOOST_CHECK(p->value()[1]->type() == Type::STRING);
        BOOST_CHECK(p->value()[2]->type() == Type::STRING);

        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<String>(p->value()[0])->value(),
            "394c2288b340d0385d4b6fbbdfe5e3f8  "
            "src/main/resources/META-INF/native/bcos-sdk-jni.dll");
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<String>(p->value()[1])->value(),
            "73683843d97477ebe342e53c87c40ee5  "
            "src/main/resources/META-INF/native/libbcos-sdk-jni.dylib");
        BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<String>(p->value()[2])->value(), "");

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000"
            "00000000000000000000000000000000000000000300000000000000000000000000000000000000000000"
            "0000000000000000006000000000000000000000000000000000000000000000000000000000000000e000"
            "00000000000000000000000000000000000000000000000000000000000160000000000000000000000000"
            "00000000000000000000000000000000000000553339346332323838623334306430333835643462366662"
            "62646665356533663820207372632f6d61696e2f7265736f75726365732f4d4554412d494e462f6e617469"
            "76652f62636f732d73646b2d6a6e692e646c6c000000000000000000000000000000000000000000000000"
            "0000000000000000000000000000000000005a373336383338343364393734373765626533343265353363"
            "383763343065653520207372632f6d61696e2f7265736f75726365732f4d4554412d494e462f6e61746976"
            "652f6c696262636f732d73646b2d6a6e692e64796c69620000000000000000000000000000000000000000"
            "000000000000000000000000000000000000",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }
}

BOOST_AUTO_TEST_CASE(test_initAbstractTypeWithJsonParams_Exp)
{
    auto solcImpl = std::make_shared<ContractABITypeCodecSolImpl>();
    ContractABICodec codec(nullptr, solcImpl);

    // empty dynamic list
    {
        std::string params = "[[]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto dyList = DynamicList::newValue();
        sPtr->addMember(std::move(dyList));

        codec.initAbstractTypeWithJsonParams(*sPtr, jParams);

        bcos::bytes buffer;
        solcImpl->serialize(*sPtr, buffer);
        BOOST_CHECK_EQUAL(
            "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000"
            "000000000000000000000000000000000000000000",
            *toHexString(buffer));
        sPtr->clear();
        solcImpl->deserialize(*sPtr, buffer, 0);
        BOOST_CHECK_EQUAL(sPtr->toJsonString(), params);
    }

    // static list
    {
        std::string params = "[[]]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto list = FixedList::newValue(3);

        sPtr->addMember(std::move(list));

        BOOST_CHECK_THROW(codec.initAbstractTypeWithJsonParams(*sPtr, jParams), std::runtime_error);
    }

    // uint
    {
        std::string params = "[\"a\"]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto u = Uint::newValue();
        sPtr->addMember(std::move(u));

        BOOST_CHECK_THROW(codec.initAbstractTypeWithJsonParams(*sPtr, jParams), std::runtime_error);
    }

    // int
    {
        std::string params = "[\"a\"]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto u = Int::newValue();
        sPtr->addMember(std::move(u));

        BOOST_CHECK_THROW(codec.initAbstractTypeWithJsonParams(*sPtr, jParams), std::runtime_error);
    }

    // bool
    {
        std::string params = "[\"a\"]";

        Json::Reader reader;
        Json::Value jParams;
        reader.parse(params, jParams);

        auto sPtr = Struct::newValue();
        auto u = Boolean::newValue();
        sPtr->addMember(std::move(u));

        BOOST_CHECK_THROW(codec.initAbstractTypeWithJsonParams(*sPtr, jParams), std::runtime_error);
    }
}

BOOST_AUTO_TEST_CASE(test_codecByMethodSig)
{
    std::string methodSig0 = "f(string,uint256,int256)";
    std::string methodSig1 = "f(string, uint256, int256)";

    auto hashImpl = std::make_shared<crypto::Keccak256>();
    auto solcImpl = std::make_shared<ContractABITypeCodecSolImpl>();
    ContractABICodec codec(hashImpl, solcImpl);

    ContractABIDefinitionFactory contractABIDefinitionFactory;
    auto method0 = contractABIDefinitionFactory.buildMethod(methodSig0);
    auto method1 = contractABIDefinitionFactory.buildMethod(methodSig1);

    BOOST_CHECK_EQUAL(method0->getMethodSignatureAsString(), method1->getMethodSignatureAsString());

    std::string jsonParams = "[\"aaaaaaa\",111111111,-11111111]";
    std::string exceptedParams = "[\"aaaaaaa\",\"111111111\",\"-11111111\"]";
    auto buffer = codec.encodeMethodBySignature(methodSig0, jsonParams);

    auto decodeParams = codec.decodeMethodInputByMethodSig(methodSig0, buffer);

    BOOST_CHECK_EQUAL(exceptedParams, decodeParams);
}

BOOST_AUTO_TEST_CASE(test_encodeMethod)
{
    std::string abi =
        "[{\"anonymous\":false,\"inputs\":[{\"indexed\":false,\"internalType\":\"int256\",\"name\":"
        "\"a\",\"type\":\"int256\"},{\"components\":[{\"internalType\":\"string\",\"name\":"
        "\"name\",\"type\":\"string\"},{\"internalType\":\"int256\",\"name\":\"count\",\"type\":"
        "\"int256\"},{\"components\":[{\"internalType\":\"int256\",\"name\":\"a\",\"type\":"
        "\"int256\"},{\"internalType\":\"int256\",\"name\":\"b\",\"type\":\"int256\"},{"
        "\"internalType\":\"int256\",\"name\":\"c\",\"type\":\"int256\"}],\"internalType\":"
        "\"struct "
        "Proxy.Item[]\",\"name\":\"items\",\"type\":\"tuple[]\"}],\"indexed\":false,"
        "\"internalType\":\"struct "
        "Proxy.Info[]\",\"name\":\"b\",\"type\":\"tuple[]\"},{\"indexed\":false,\"internalType\":"
        "\"string\",\"name\":\"c\",\"type\":\"string\"}],\"name\":\"output1\",\"type\":\"event\"},{"
        "\"constant\":false,\"inputs\":[{\"internalType\":\"int256\",\"name\":\"a\",\"type\":"
        "\"int256\"},{\"components\":[{\"internalType\":\"string\",\"name\":\"name\",\"type\":"
        "\"string\"},{\"internalType\":\"int256\",\"name\":\"count\",\"type\":\"int256\"},{"
        "\"components\":[{\"internalType\":\"int256\",\"name\":\"a\",\"type\":\"int256\"},{"
        "\"internalType\":\"int256\",\"name\":\"b\",\"type\":\"int256\"},{\"internalType\":"
        "\"int256\",\"name\":\"c\",\"type\":\"int256\"}],\"internalType\":\"struct "
        "Proxy.Item[]\",\"name\":\"items\",\"type\":\"tuple[]\"}],\"internalType\":\"struct "
        "Proxy.Info[]\",\"name\":\"b\",\"type\":\"tuple[]\"},{\"internalType\":\"string\",\"name\":"
        "\"c\",\"type\":\"string\"}],\"name\":\"test\",\"outputs\":[{\"internalType\":\"int256\","
        "\"name\":\"\",\"type\":\"int256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\","
        "\"type\":\"function\"},{\"constant\":false,\"inputs\":[],\"name\":\"test1\",\"outputs\":[{"
        "\"internalType\":\"int256\",\"name\":\"a\",\"type\":\"int256\"},{\"components\":[{"
        "\"internalType\":\"string\",\"name\":\"name\",\"type\":\"string\"},{\"internalType\":"
        "\"int256\",\"name\":\"count\",\"type\":\"int256\"},{\"components\":[{\"internalType\":"
        "\"int256\",\"name\":\"a\",\"type\":\"int256\"},{\"internalType\":\"int256\",\"name\":"
        "\"b\",\"type\":\"int256\"},{\"internalType\":\"int256\",\"name\":\"c\",\"type\":"
        "\"int256\"}],\"internalType\":\"struct "
        "Proxy.Item[]\",\"name\":\"items\",\"type\":\"tuple[]\"}],\"internalType\":\"struct "
        "Proxy.Info[]\",\"name\":\"b\",\"type\":\"tuple[]\"},{\"internalType\":\"string\",\"name\":"
        "\"c\",\"type\":\"string\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":"
        "\"function\"},{\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"fallback\"}"
        "]";

    ContractABIDefinitionFactory factory;
    auto tests = factory.buildMethod(abi, "test");

    auto contractABI = factory.buildABI(abi);
    BOOST_CHECK_EQUAL(contractABI->methodNames().size(), 3);

    BOOST_CHECK_EQUAL(tests.size(), 1);

    auto test = tests[0];

    auto hashImpl = std::make_shared<crypto::Keccak256>();

    auto solcImpl = std::make_shared<ContractABITypeCodecSolImpl>();
    ContractABICodec codec(hashImpl, solcImpl);

    auto methodID = *toHexString(test->getMethodID(hashImpl));

    BOOST_CHECK_EQUAL(methodID, "00a3c75d");

    std::string objParams =
        "[100, [{\"name\":\"Hello "
        "world!\",\"count\":100,\"items\":[{\"a\":1,\"b\":2,\"c\":3}]},{\"name\":\"Hello "
        "world2\",\"count\":200,\"items\":[{\"a\":5,\"b\":6,\"c\":7}]}], \"Hello world!\"]";

    std::string arrayParams =
        "[\"100\",[[\"Hello world!\",\"100\",[[\"1\",\"2\",\"3\"]]],[\"Hello "
        "world2\",\"200\",[[\"5\",\"6\",\"7\"]]]],\"Hello world!\"]";

    std::string expectEncode = std::string("00a3c75d") +
                               "0000000000000000000000000000000000000000000000000000000000000064"
                               "0000000000000000000000000000000000000000000000000000000000000060"
                               "0000000000000000000000000000000000000000000000000000000000000300"
                               "0000000000000000000000000000000000000000000000000000000000000002"
                               "0000000000000000000000000000000000000000000000000000000000000040"
                               "0000000000000000000000000000000000000000000000000000000000000160"
                               "0000000000000000000000000000000000000000000000000000000000000060"
                               "0000000000000000000000000000000000000000000000000000000000000064"
                               "00000000000000000000000000000000000000000000000000000000000000a0"
                               "000000000000000000000000000000000000000000000000000000000000000c"
                               "48656c6c6f20776f726c64210000000000000000000000000000000000000000"
                               "0000000000000000000000000000000000000000000000000000000000000001"
                               "0000000000000000000000000000000000000000000000000000000000000001"
                               "0000000000000000000000000000000000000000000000000000000000000002"
                               "0000000000000000000000000000000000000000000000000000000000000003"
                               "0000000000000000000000000000000000000000000000000000000000000060"
                               "00000000000000000000000000000000000000000000000000000000000000c8"
                               "00000000000000000000000000000000000000000000000000000000000000a0"
                               "000000000000000000000000000000000000000000000000000000000000000c"
                               "48656c6c6f20776f726c64320000000000000000000000000000000000000000"
                               "0000000000000000000000000000000000000000000000000000000000000001"
                               "0000000000000000000000000000000000000000000000000000000000000005"
                               "0000000000000000000000000000000000000000000000000000000000000006"
                               "0000000000000000000000000000000000000000000000000000000000000007"
                               "000000000000000000000000000000000000000000000000000000000000000c"
                               "48656c6c6f20776f726c64210000000000000000000000000000000000000000";


    auto encodeBytes = codec.encodeMethod(abi, "test", objParams);
    auto arrayParamsEncodeBytes = codec.encodeMethod(abi, "test", objParams);

    BOOST_CHECK_EQUAL(*toHexString(encodeBytes), expectEncode);
    BOOST_CHECK_EQUAL(*toHexString(arrayParamsEncodeBytes), expectEncode);

    auto decodeParams = codec.decodeMethodInput(abi, "test", encodeBytes);

    BOOST_CHECK_EQUAL(decodeParams, arrayParams);

    {
        // encode with obj params
        ContractABIDefinitionFactory contractABIDefinitionFactory;
        auto inputStruct = contractABIDefinitionFactory.buildInput(*test);

        BOOST_CHECK_EQUAL(inputStruct->getTypeAsString(),
            "(int256,(string,int256,(int256,int256,int256)[])[],string)");

        codec.initAbstractTypeWithJsonParams(*inputStruct, objParams);

        BOOST_CHECK_EQUAL(arrayParams, inputStruct->toJsonString());
    }

    {
        // encode with array params
        ContractABIDefinitionFactory contractABIDefinitionFactory;
        auto inputStruct = contractABIDefinitionFactory.buildInput(*test);

        codec.initAbstractTypeWithJsonParams(*inputStruct, arrayParams);

        BOOST_CHECK_EQUAL(arrayParams, inputStruct->toJsonString());
    }
}

BOOST_AUTO_TEST_SUITE_END()
