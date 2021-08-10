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
 * @file WsService.h
 * @author: octopus
 * @date 2021-07-28
 */
#pragma once

#include <bcos-cpp-sdk/ws/Common.h>
#include <bcos-framework/interfaces/protocol/ProtocolTypeDef.h>
#include <bcos-framework/libutilities/Common.h>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace bcos {
class ThreadPool;
namespace ws {
class WsSession;
class WsMessage;
class AMOPRequestFactory;
class WsMessageFactory;

using WsSessions = std::vector<std::shared_ptr<WsSession>>;
using WsMsgHandler =
    std::function<void(std::shared_ptr<WsMessage>, std::shared_ptr<WsSession>)>;

class WsService : public std::enable_shared_from_this<WsService> {
public:
  using Ptr = std::shared_ptr<WsService>;
  WsService() = default;
  virtual ~WsService() { stop(); }
  void initMethod();

public:
  virtual void start();
  virtual void stop();
  virtual void doLoop();

public:
  std::shared_ptr<WsSession> getSession(const std::string &_endPoint);
  void addSession(std::shared_ptr<WsSession> _session);
  void removeSession(const std::string &_endPoint);
  WsSessions sessions();

public:
  /**
   * @brief: websocket session disconnect
   * @param _msg: received message
   * @param _error:
   * @param _session: websocket session
   * @return void:
   */
  virtual void onDisconnect(Error::Ptr _error,
                            std::shared_ptr<WsSession> _session);

  //---------------------------------------------------------
  //-------------- message begin
  //---------------------------------------------------------
  virtual void onRecvMessage(std::shared_ptr<WsMessage> _msg,
                             std::shared_ptr<WsSession> _session);
  virtual void onRecvAMOPRequest(std::shared_ptr<WsMessage> _msg,
                                 std::shared_ptr<WsSession> _session);
  virtual void onRecvAMOPResponse(std::shared_ptr<WsMessage> _msg,
                                  std::shared_ptr<WsSession> _session);
  virtual void onRecvAMOPBroadcast(std::shared_ptr<WsMessage> _msg,
                                   std::shared_ptr<WsSession> _session);
  virtual void onRecvBlockNumberNotify(std::shared_ptr<WsMessage> _msg,
                                       std::shared_ptr<WsSession> _session);

  //-------------- message end
  //---------------------------------------------------------

  void subscribe(const std::set<std::string> _topics,
                 std::shared_ptr<WsSession> _session);
  void publish(const std::string &_topic, std::shared_ptr<bcos::bytes> _msg,
               std::shared_ptr<WsSession> _session,
               std::function<void(Error::Ptr, std::shared_ptr<bcos::bytes>)>
                   _callback = nullptr);
  void broadcast(const std::string &_topic, std::shared_ptr<bcos::bytes> _msg,
                 std::shared_ptr<WsSession> _session);

public:
  std::shared_ptr<AMOPRequestFactory> requestFactory() const {
    return m_requestFactory;
  }
  void setRequestFactory(std::shared_ptr<AMOPRequestFactory> _requestFactory) {
    m_requestFactory = _requestFactory;
  }
  std::shared_ptr<WsMessageFactory> messageFactory() {
    return m_messageFactory;
  }
  void setMessageFactory(std::shared_ptr<WsMessageFactory> _messageFactory) {
    m_messageFactory = _messageFactory;
  }

  std::shared_ptr<bcos::ThreadPool> threadPool() const { return m_threadPool; }
  void setThreadPool(std::shared_ptr<bcos::ThreadPool> _threadPool) {
    m_threadPool = _threadPool;
  }

  std::shared_ptr<boost::asio::io_context> ioc() const { return m_ioc; }
  void setIoc(std::shared_ptr<boost::asio::io_context> _ioc) { m_ioc = _ioc; }

private:
  bool m_running{false};
  // AMOPRequestFactory
  std::shared_ptr<AMOPRequestFactory> m_requestFactory;
  // WsMessageFactory
  std::shared_ptr<WsMessageFactory> m_messageFactory;
  // ThreadPool
  std::shared_ptr<bcos::ThreadPool> m_threadPool;
  // mutex for m_sessions
  mutable std::shared_mutex x_mutex;
  // all active sessions
  std::unordered_map<std::string, std::shared_ptr<WsSession>> m_sessions;
  // io context
  std::shared_ptr<boost::asio::io_context> m_ioc;
  std::shared_ptr<boost::asio::deadline_timer> m_loopTimer;
  // type => handler
  std::unordered_map<uint32_t, std::function<void(std::shared_ptr<WsMessage>,
                                                  std::shared_ptr<WsSession>)>>
      m_msgType2Method;
};

} // namespace ws
} // namespace bcos