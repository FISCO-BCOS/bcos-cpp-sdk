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
 * @file WsService.cpp
 * @author: octopus
 * @date 2021-07-28
 */
#include <algorithm>
#include <bcos-cpp-sdk/ws/Common.h>
#include <bcos-cpp-sdk/ws/WsMessageType.h>
#include <bcos-cpp-sdk/ws/WsService.h>
#include <bcos-cpp-sdk/ws/WsSession.h>
#include <bcos-framework/interfaces/protocol/CommonError.h>
#include <bcos-framework/libutilities/Common.h>
#include <bcos-framework/libutilities/DataConvertUtility.h>
#include <bcos-framework/libutilities/Log.h>
#include <bcos-framework/libutilities/ThreadPool.h>
#include <boost/core/ignore_unused.hpp>
#include <memory>
#include <string>
#include <vector>

#define WS_SERVICE_DO_LOOP_PERIOD (10000)

using namespace bcos;
using namespace bcos::ws;

void WsService::start() {
  if (m_running) {
    WEBSOCKET_SERVICE(INFO)
        << LOG_BADGE("start") << LOG_DESC("websocket service is running");
    return;
  }
  m_running = true;
  doLoop();
  WEBSOCKET_SERVICE(INFO) << LOG_BADGE("start")
                          << LOG_DESC("start websocket service successfully");
}

void WsService::stop() {
  if (!m_running) {
    WEBSOCKET_SERVICE(INFO)
        << LOG_BADGE("stop") << LOG_DESC("websocket service has been stopped");
    return;
  }
  m_running = false;

  if (m_loopTimer) {
    m_loopTimer->cancel();
  }

  WEBSOCKET_SERVICE(INFO) << LOG_BADGE("stop")
                          << LOG_DESC("stop websocket service successfully");
}

void WsService::doLoop() {
  m_loopTimer = std::make_shared<boost::asio::deadline_timer>(
      boost::asio::make_strand(*m_ioc),
      boost::posix_time::milliseconds(WS_SERVICE_DO_LOOP_PERIOD));

  auto self = std::weak_ptr<WsService>(shared_from_this());
  m_loopTimer->async_wait([self](const boost::system::error_code &) {
    auto service = self.lock();
    if (!service) {
      return;
    }

    auto ss = service->sessions();
    for (auto const &session : ss) {
      boost::ignore_unused(session);
      // NOTE: server should send heartbeat message
    }
    WEBSOCKET_SERVICE(INFO)
        << LOG_BADGE("doLoop") << LOG_KV("connected sdk count", ss.size());
    service->doLoop();
  });
}

void WsService::initMethod() {
  m_msgType2Method.clear();
  /*
  auto self = std::weak_ptr<WsService>(shared_from_this());
  m_msgType2Method[WsMessageType::HANDESHAKE] =
  [self](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
      auto service = self.lock();
      if (service)
      {
          service->onRecvHandshake(_msg, _session);
      }
  };
  m_msgType2Method[WsMessageType::RPC_REQUEST] =
  [self](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
      auto service = self.lock();
      if (service)
      {
          service->onRecvRPCRequest(_msg, _session);
      }
  };
  m_msgType2Method[WsMessageType::AMOP_SUBTOPIC] =
  [self](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
      auto service = self.lock();
      if (service)
      {
          service->onRecvSubTopics(_msg, _session);
      }
  };
  m_msgType2Method[WsMessageType::AMOP_REQUEST] =
  [self](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
      auto service = self.lock();
      if (service)
      {
          service->onRecvAMOPRequest(_msg, _session);
      }
  };
  m_msgType2Method[WsMessageType::AMOP_BROADCAST] =
  [self](std::shared_ptr<WsMessage> _msg, std::shared_ptr<WsSession> _session) {
      auto service = self.lock();
      if (service)
      {
          service->onRecvAMOPBroadcast(_msg, _session);
      }
  };
  */

  WEBSOCKET_SERVICE(INFO) << LOG_BADGE("initMethod")
                          << LOG_KV("methods", m_msgType2Method.size());
  for (const auto &method : m_msgType2Method) {
    WEBSOCKET_SERVICE(INFO)
        << LOG_BADGE("initMethod") << LOG_KV("type", method.first);
  }
}

void WsService::addSession(std::shared_ptr<WsSession> _session) {
  {
    std::unique_lock lock(x_mutex);
    m_sessions[_session->remoteEndPoint()] = _session;
  }

  WEBSOCKET_SERVICE(INFO) << LOG_BADGE("addSession")
                          << LOG_KV("endpoint", _session
                                                    ? _session->remoteEndPoint()
                                                    : std::string(""));
}

void WsService::removeSession(const std::string &_endPoint) {
  {
    std::unique_lock lock(x_mutex);
    m_sessions.erase(_endPoint);
  }

  WEBSOCKET_SERVICE(INFO) << LOG_BADGE("removeSession")
                          << LOG_KV("endpoint", _endPoint);
}

std::shared_ptr<WsSession> WsService::getSession(const std::string &_endPoint) {
  std::shared_lock lock(x_mutex);
  return m_sessions[_endPoint];
}

WsSessions WsService::sessions() {
  WsSessions sessions;
  {
    std::shared_lock lock(x_mutex);
    for (const auto &session : m_sessions) {
      if (session.second->isConnected()) {
        sessions.push_back(session.second);
      } else {
        WEBSOCKET_SERVICE(DEBUG)
            << LOG_BADGE("sessions") << LOG_DESC("the session is disconnected")
            << LOG_KV("endpoint", session.second->remoteEndPoint());
      }
    }
  }

  WEBSOCKET_SERVICE(TRACE) << LOG_BADGE("sessions")
                           << LOG_KV("size", sessions.size());
  return sessions;
}

/**
 * @brief: websocket session disconnect
 * @param _msg: received message
 * @param _error:
 * @param _session: websocket session
 * @return void:
 */
void WsService::onDisconnect(Error::Ptr _error,
                             std::shared_ptr<WsSession> _session) {
  boost::ignore_unused(_error);
  std::string endpoint = "";
  if (_session) {
    endpoint = _session->remoteEndPoint();
  }

  // clear the session
  removeSession(endpoint);
  // Add additional disconnect logic

  WEBSOCKET_SERVICE(INFO) << LOG_BADGE("onDisconnect")
                          << LOG_KV("endpoint", endpoint);
}

void WsService::onRecvAMOPRequest(std::shared_ptr<WsMessage> _msg,
                                  std::shared_ptr<WsSession> _session) {
  boost::ignore_unused(_msg, _session);
}
void WsService::onRecvAMOPResponse(std::shared_ptr<WsMessage> _msg,
                                   std::shared_ptr<WsSession> _session) {
  boost::ignore_unused(_msg, _session);
}
void WsService::onRecvAMOPBroadcast(std::shared_ptr<WsMessage> _msg,
                                    std::shared_ptr<WsSession> _session) {
  boost::ignore_unused(_msg, _session);
}
void WsService::onRecvRpcResponse(std::shared_ptr<WsMessage> _msg,
                                  std::shared_ptr<WsSession> _session) {
  boost::ignore_unused(_msg, _session);
}
void WsService::onRecvBlockNotify(std::shared_ptr<WsMessage> _msg,
                                  std::shared_ptr<WsSession> _session) {
  boost::ignore_unused(_msg, _session);
}