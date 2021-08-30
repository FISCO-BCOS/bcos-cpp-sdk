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
 *  m_limitations under the License.
 *
 * @file WsTools.cpp
 * @author: octopus
 * @date 2021-08-23
 */

#include <bcos-cpp-sdk/ws/Common.h>
#include <bcos-cpp-sdk/ws/WsTools.h>
#include <memory>
#include <utility>

using namespace bcos;
using namespace bcos::ws;

void WsTools::connectToWsServer(std::shared_ptr<boost::asio::ip::tcp::resolver> _resolver,
    std::shared_ptr<boost::asio::io_context> _ioc, const std::string& _host, uint16_t _port,
    std::function<void(std::shared_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>>)>
        _callback)
{
    // resolve host
    _resolver->async_resolve(_host.c_str(), std::to_string(_port).c_str(),
        [_host, _port, _callback, _ioc](
            boost::beast::error_code _ec, boost::asio::ip::tcp::resolver::results_type _results) {
            if (_ec)
            {
                WEBSOCKET_TOOL(ERROR) << LOG_BADGE("connectToWsServer") << LOG_DESC("async_resolve")
                                      << LOG_KV("error", _ec.message()) << LOG_KV("host", _host);
                return;
            }

            WEBSOCKET_TOOL(DEBUG) << LOG_BADGE("connectToWsServer")
                                  << LOG_DESC("async_resolve success") << LOG_KV("host", _host)
                                  << LOG_KV("port", _port);

            auto stream =
                std::make_shared<boost::beast::websocket::stream<boost::beast::tcp_stream>>(*_ioc);
            boost::beast::get_lowest_layer(*stream).expires_after(std::chrono::seconds(30));

            // async connect
            boost::beast::get_lowest_layer(*stream).async_connect(_results,
                [stream, _host, _port, _callback](boost::beast::error_code _ec,
                    boost::asio::ip::tcp::resolver::results_type::endpoint_type _ep) mutable {
                    if (_ec)
                    {
                        WEBSOCKET_TOOL(ERROR)
                            << LOG_BADGE("connectToWsServer") << LOG_DESC("async_connect")
                            << LOG_KV("error", _ec.message()) << LOG_KV("host", _host)
                            << LOG_KV("port", _port);
                        return;
                    }

                    WEBSOCKET_TOOL(DEBUG)
                        << LOG_BADGE("connectToWsServer") << LOG_DESC("async_connect success")
                        << LOG_KV("host", _host) << LOG_KV("port", _port);

                    // turn off the timeout on the tcp_stream, because
                    // the websocket stream has its own timeout system.
                    boost::beast::get_lowest_layer(*stream).expires_never();

                    // set suggested timeout settings for the websocket
                    stream->set_option(boost::beast::websocket::stream_base::timeout::suggested(
                        boost::beast::role_type::client));

                    // set a decorator to change the User-Agent of the handshake
                    stream->set_option(boost::beast::websocket::stream_base::decorator(
                        [](boost::beast::websocket::request_type& req) {
                            req.set(boost::beast::http::field::user_agent,
                                std::string(BOOST_BEAST_VERSION_STRING) + " fisco-cppsdk-client");
                        }));

                    std::string tmpHost = _host + ':' + std::to_string(_ep.port());

                    // async handshake
                    stream->async_handshake(tmpHost, "/",
                        [_host, _port, stream, _callback](boost::beast::error_code _ec) mutable {
                            if (_ec)
                            {
                                WEBSOCKET_TOOL(ERROR)
                                    << LOG_BADGE("connectToWsServer") << LOG_DESC("async_handshake")
                                    << LOG_KV("error", _ec.message()) << LOG_KV("host", _host)
                                    << LOG_KV("port", _port);
                                return;
                            }

                            WEBSOCKET_TOOL(DEBUG) << LOG_BADGE("connectToWsServer")
                                                  << LOG_DESC("websocket handshake successfully")
                                                  << LOG_KV("host", _host) << LOG_KV("port", _port);
                            _callback(stream);
                        });
                });
        });
}
