
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
 * @file WsMessage.cpp
 * @author: octopus
 * @date 2021-07-28
 */
#include <bcos-cpp-sdk/ws/Common.h>
#include <bcos-cpp-sdk/ws/WsMessage.h>
#include <boost/asio/detail/socket_ops.hpp>
#include <iterator>
#include <stdexcept>
#include <string>

using namespace bcos;
using namespace bcos::ws;

// seq field length
const size_t WsMessage::SEQ_LENGTH;
/// type(2) + error(2) + seq(32) + data(N)
const size_t WsMessage::MESSAGE_MIN_LENGTH;

bool WsMessage::encode(bcos::bytes& _buffer)
{
    /*
        struct Packet {
        1 require short type;
        2 require short status;
        3 require char[32] id;
        4 optional vector<char> data;
    }；*/
    _buffer.clear();

    uint16_t type = boost::asio::detail::socket_ops::host_to_network_short(m_type);
    uint16_t status = boost::asio::detail::socket_ops::host_to_network_short(m_status);

    // seq length should be SEQ_LENGTH(32) long
    if (m_seq->size() != SEQ_LENGTH)
    {
        return false;
    }

    _buffer.insert(_buffer.end(), (byte*)&type, (byte*)&type + 2);
    _buffer.insert(_buffer.end(), (byte*)&status, (byte*)&status + 2);
    _buffer.insert(_buffer.end(), m_seq->begin(), m_seq->end());
    _buffer.insert(_buffer.end(), m_data->begin(), m_data->end());

    return true;
}

ssize_t WsMessage::decode(const bcos::byte* _buffer, std::size_t _size)
{
    if (_size < MESSAGE_MIN_LENGTH)
    {
        return -1;
    }

    m_seq->clear();
    m_data->clear();

    std::size_t offset = 0;

    auto p = _buffer + offset;
    // type field
    m_type = boost::asio::detail::socket_ops::network_to_host_short(*((uint16_t*)p));
    p += 2;

    // status field
    m_status = boost::asio::detail::socket_ops::network_to_host_short(*((uint16_t*)p));
    p += 2;

    // seq field
    m_seq->insert(m_seq->begin(), p, p + SEQ_LENGTH);
    p += SEQ_LENGTH;
    // data field
    m_data->insert(m_data->begin(), p, _buffer + _size);

    return _size;
}