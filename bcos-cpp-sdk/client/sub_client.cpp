//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket client, asynchronous
//
//------------------------------------------------------------------------------

#include "bcos-cpp-sdk/ws/Common.h"
#include <bcos-cpp-sdk/ws/WsMessage.h>
#include <bcos-cpp-sdk/ws/WsService.h>
#include <bcos-cpp-sdk/ws/WsSession.h>
#include <bcos-framework/libutilities/Common.h>
#include <bcos-framework/libutilities/Log.h>
#include <bcos-framework/libutilities/ThreadPool.h>
#include <json/json.h>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/core/ignore_unused.hpp>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

using Handler = std::function<void(boost::beast::websocket::stream<boost::beast::tcp_stream>&&)>;

//------------------------------------------------------------------------------

void fail(boost::beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
    std::exit(-1);
}

//------------------------------------------------------------------------------

void usage()
{
    std::cerr << "Usage: sub-client <host> <port> <topic>\n"
              << "Example:\n"
              << "    ./sub-client 127.0.0.1 20200 topic\n";
    std::exit(0);
}

//------------------------------------------------------------------------------
bcos::ws::WsService::Ptr buildWsService()
{
    auto threadPool = std::make_shared<bcos::ThreadPool>("t_sub", 4);
    auto ioc = std::make_shared<boost::asio::io_context>();
    auto messageFactory = std::make_shared<bcos::ws::WsMessageFactory>();
    auto requestFactory = std::make_shared<bcos::ws::AMOPRequestFactory>();

    auto wsService = std::make_shared<bcos::ws::WsService>();
    wsService->setIoc(ioc);
    wsService->setThreadPool(threadPool);
    wsService->setMessageFactory(messageFactory);
    wsService->setRequestFactory(requestFactory);
    wsService->initMethod();
    return wsService;
}

class WsSessionInitialize : public std::enable_shared_from_this<WsSessionInitialize>
{
private:
    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::websocket::stream<boost::beast::tcp_stream> m_wsStream;
    std::string m_host;
    std::function<void(boost::beast::websocket::stream<boost::beast::tcp_stream>&&)> m_handler;

public:
    // Resolver and socket require an io_context
    explicit WsSessionInitialize(boost::asio::io_context& ioc)
      : m_resolver(boost::asio::make_strand(ioc)), m_wsStream(boost::asio::make_strand(ioc))
    {}

    // Start the asynchronous operation
    void run(char const* host, char const* port)
    {
        // Save these for later
        m_host = host;

        // Look up the domain name
        m_resolver.async_resolve(host, port,
            boost::beast::bind_front_handler(&WsSessionInitialize::on_resolve, shared_from_this()));
    }

    void on_resolve(
        boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results)
    {
        if (ec)
            return fail(ec, "resolve");

        // Set the timeout for the operation
        boost::beast::get_lowest_layer(m_wsStream).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        boost::beast::get_lowest_layer(m_wsStream)
            .async_connect(results, boost::beast::bind_front_handler(
                                        &WsSessionInitialize::on_connect, shared_from_this()));
    }

    void on_connect(
        boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type ep)
    {
        if (ec)
            return fail(ec, "connect");

        // turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        boost::beast::get_lowest_layer(m_wsStream).expires_never();

        // set suggested timeout settings for the websocket
        m_wsStream.set_option(boost::beast::websocket::stream_base::timeout::suggested(
            boost::beast::role_type::client));

        // set a decorator to change the User-Agent of the handshake
        m_wsStream.set_option(boost::beast::websocket::stream_base::decorator(
            [](boost::beast::websocket::request_type& req) {
                req.set(boost::beast::http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) + " websocket-amop-sample");
            }));

        m_host += ':' + std::to_string(ep.port());

        // perform the websocket handshake
        m_wsStream.async_handshake(m_host, "/", [this](boost::beast::error_code _ec) {
            if (_ec)
            {
                fail(_ec, "async_handshake");
            }
            this->m_handler(std::move(m_wsStream));
        });
    }

    void setHandler(
        std::function<void(boost::beast::websocket::stream<boost::beast::tcp_stream>&&)> _handler)
    {
        m_handler = _handler;
    }

    std::function<void(boost::beast::websocket::stream<boost::beast::tcp_stream>&&)> handler() const
    {
        return m_handler;
    }
};

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        usage();
    }

    std::string host = argv[1];
    std::string port = argv[2];
    std::string topic = argv[3];

    BCOS_LOG(INFO) << LOG_DESC("amop sub client sample") << LOG_KV("ip", host)
                   << LOG_KV("port", port) << LOG_KV("topic", topic);

    auto wsService = buildWsService();
    auto initialize = std::make_shared<WsSessionInitialize>(*wsService->ioc());
    initialize->setHandler(
        [wsService, topic](boost::beast::websocket::stream<boost::beast::tcp_stream>&& _wsStream) {
            auto remoteEndPoint = _wsStream.next_layer().socket().remote_endpoint();
            std::string s =
                remoteEndPoint.address().to_string() + ":" + std::to_string(remoteEndPoint.port());
            auto wsSession = std::make_shared<bcos::ws::WsSession>(std::move(_wsStream));
            auto _wsServiceWeakPtr = std::weak_ptr<bcos::ws::WsService>(wsService);
            wsSession->setThreadPool(wsService->threadPool());
            wsSession->setMessageFactory(wsService->messageFactory());
            wsSession->setEndPoint(s);
            wsSession->setRecvMessageHandler(
                [_wsServiceWeakPtr](std::shared_ptr<bcos::ws::WsMessage> _msg,
                    std::shared_ptr<bcos::ws::WsSession> _session) {
                    auto wsService = _wsServiceWeakPtr.lock();
                    if (wsService)
                    {
                        wsService->onRecvMessage(_msg, _session);
                    }
                });
            wsSession->run();
            wsService->subscribe(std::set<std::string>{topic}, wsSession);
            wsService->addSession(wsSession);
        });
    initialize->run(host.c_str(), port.c_str());

    BCOS_LOG(INFO) << LOG_DESC(" ==> subscible ") << LOG_KV("topic", topic);

    auto ioc = wsService->ioc();
    std::size_t threadC = 4;
    std::shared_ptr<std::vector<std::thread>> threads =
        std::make_shared<std::vector<std::thread>>();
    threads->reserve(threadC);
    for (auto i = threadC; i > 0; --i)
    {
        threads->emplace_back([&ioc]() { ioc->run(); });
    }

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return EXIT_SUCCESS;
}