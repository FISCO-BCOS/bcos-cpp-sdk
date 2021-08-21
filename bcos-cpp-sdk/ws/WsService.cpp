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
#include <json/json.h>
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

    // NOTE: reconnect logic
    auto ss = service->sessions();
    for (auto const &session : ss) {
      boost::ignore_unused(session);
    }

    WEBSOCKET_SERVICE(INFO)
        << LOG_BADGE("doLoop") << LOG_KV("connected sdk count", ss.size());
    service->doLoop();
  });
}

void WsService::initMethod() {
  m_msgType2Method.clear();

  auto self = std::weak_ptr<WsService>(shared_from_this());
  m_msgType2Method[WsMessageType::BLOCK_NOTIFY] =
      [self](std::shared_ptr<WsMessage> _msg,
             std::shared_ptr<WsSession> _session) {
        auto service = self.lock();
        if (service) {
          service->onRecvBlockNumberNotify(_msg, _session);
        }
      };
  m_msgType2Method[WsMessageType::AMOP_REQUEST] =
      [self](std::shared_ptr<WsMessage> _msg,
             std::shared_ptr<WsSession> _session) {
        auto service = self.lock();
        if (service) {
          service->onRecvAMOPRequest(_msg, _session);
        }
      };
  m_msgType2Method[WsMessageType::AMOP_RESPONSE] =
      [self](std::shared_ptr<WsMessage> _msg,
             std::shared_ptr<WsSession> _session) {
        auto service = self.lock();
        if (service) {
          service->onRecvAMOPResponse(_msg, _session);
        }
      };
  m_msgType2Method[WsMessageType::AMOP_BROADCAST] =
      [self](std::shared_ptr<WsMessage> _msg,
             std::shared_ptr<WsSession> _session) {
        auto service = self.lock();
        if (service) {
          service->onRecvAMOPBroadcast(_msg, _session);
        }
      };

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

void WsService::onRecvMessage(std::shared_ptr<WsMessage> _msg,
                              std::shared_ptr<WsSession> _session) {

  auto seq = std::string(_msg->seq()->begin(), _msg->seq()->end());

  WEBSOCKET_SERVICE(TRACE) << LOG_BADGE("onRecvMessage")
                           << LOG_KV("type", _msg->type()) << LOG_KV("seq", seq)
                           << LOG_KV("endpoint", _session->remoteEndPoint())
                           << LOG_KV("data size", _msg->data()->size());

  auto it = m_msgType2Method.find(_msg->type());
  if (it != m_msgType2Method.end()) {
    auto callback = it->second;
    callback(_msg, _session);
  } else {
    WEBSOCKET_SERVICE(ERROR)
        << LOG_BADGE("onRecvMessage") << LOG_DESC("unrecognized message type")
        << LOG_KV("type", _msg->type())
        << LOG_KV("endpoint", _session->remoteEndPoint()) << LOG_KV("seq", seq)
        << LOG_KV("data size", _msg->data()->size());
  }
}

void WsService::onRecvAMOPRequest(std::shared_ptr<WsMessage> _msg,
                                  std::shared_ptr<WsSession> _session) {
  auto request = m_requestFactory->buildRequest();
  request->decode(bytesConstRef(_msg->data()->data(), _msg->data()->size()));
  auto data = std::string(request->data().begin(), request->data().end());
  WEBSOCKET_VERSION(INFO) << LOG_DESC("onRecvAMOPRequest")
                          << LOG_KV("endpoint", _session->remoteEndPoint())
                          << LOG_KV("message", data);

  _msg->setType(WsMessageType::AMOP_RESPONSE);
  _msg->setData(std::make_shared<bcos::bytes>(request->data().begin(),
                                              request->data().end()));
  // NOTE: just send the message response
  _session->asyncSendMessage(_msg);
}

void WsService::onRecvAMOPResponse(std::shared_ptr<WsMessage> _msg,
                                   std::shared_ptr<WsSession> _session) {
  auto strMsg = std::string(_msg->data()->begin(), _msg->data()->end());
  WEBSOCKET_VERSION(INFO) << LOG_DESC("onRecvAMOPResponse")
                          << LOG_KV("endpoint", _session->remoteEndPoint())
                          << LOG_KV("message", strMsg);
}

void WsService::onRecvAMOPBroadcast(std::shared_ptr<WsMessage> _msg,
                                    std::shared_ptr<WsSession> _session) {
  auto strMsg = std::string(_msg->data()->begin(), _msg->data()->end());
  WEBSOCKET_VERSION(INFO) << LOG_DESC("onRecvAMOPBroadcast")
                          << LOG_KV("endpoint", _session->remoteEndPoint())
                          << LOG_KV("message", strMsg);
}

void WsService::onRecvBlockNumberNotify(std::shared_ptr<WsMessage> _msg,
                                        std::shared_ptr<WsSession> _session) {
  auto jsonValue = std::string(_msg->data()->begin(), _msg->data()->end());
  WEBSOCKET_VERSION(INFO) << LOG_DESC("onRecvBlockNumberNotify")
                          << LOG_KV("blockNumber", jsonValue)
                          << LOG_KV("endpoint", _session->remoteEndPoint());
}

void WsService::subscribe(const std::set<std::string> _topics,
                          std::shared_ptr<WsSession> _session) {
  Json::Value jTopics(Json::arrayValue);
  for (const auto &topic : _topics) {
    jTopics.append(topic);
  }
  Json::Value jReq;
  jReq["topics"] = jTopics;
  Json::FastWriter writer;
  std::string request = writer.write(jReq);

  auto msg = m_messageFactory->buildMessage();
  msg->setType(bcos::ws::WsMessageType::AMOP_SUBTOPIC);
  msg->setData(std::make_shared<bcos::bytes>(request.begin(), request.end()));

  WEBSOCKET_VERSION(INFO) << LOG_DESC("subscribe") << LOG_KV("topics", request);

  _session->asyncSendMessage(msg);
}

void WsService::publish(
    const std::string &_topic, std::shared_ptr<bcos::bytes> _msg,
    std::shared_ptr<WsSession> _session,
    std::function<void(Error::Ptr, std::shared_ptr<bcos::bytes>)> _callback) {
  auto requestFactory = std::make_shared<bcos::ws::AMOPRequestFactory>();
  auto request = requestFactory->buildRequest();
  request->setTopic(_topic);
  request->setData(bytesConstRef(_msg->data(), _msg->size()));
  auto buffer = std::make_shared<bcos::bytes>();
  request->encode(*buffer);

  auto message = m_messageFactory->buildMessage();
  message->setType(bcos::ws::WsMessageType::AMOP_REQUEST);
  message->setData(buffer);

  _session->asyncSendMessage(message);

  boost::ignore_unused(_callback);
  //   if (_callback) {
  //     // TODO:
  //   }
}

void WsService::broadcast(const std::string &_topic,
                          std::shared_ptr<bcos::bytes> _msg,
                          std::shared_ptr<WsSession> _session) {
  auto requestFactory = std::make_shared<bcos::ws::AMOPRequestFactory>();
  auto request = requestFactory->buildRequest();
  request->setTopic(_topic);
  request->setData(bytesConstRef(_msg->data(), _msg->size()));
  auto buffer = std::make_shared<bcos::bytes>();
  request->encode(*buffer);

  auto message = m_messageFactory->buildMessage();
  message->setType(bcos::ws::WsMessageType::AMOP_BROADCAST);
  message->setData(buffer);

  _session->asyncSendMessage(message);
}