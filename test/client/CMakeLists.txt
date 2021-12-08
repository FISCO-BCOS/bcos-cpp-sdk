set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

# OpenSSL
hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

# bcos-boostssl
hunter_add_package(bcos-boostssl)
find_package(bcos-boostssl REQUIRED)

add_executable(sub-client sub_client.cpp)
target_link_libraries(sub-client PUBLIC ${CPP_SDK_TARGET} ${EVENT_TARGET} ${JSON_RPC_TARGET} ${AMOP_TARGET} ${WS_SERVICE_TARGET} bcos-boostssl::boostssl-websocket bcos-boostssl::boostssl-utilities bcos-boostssl::boostssl-context bcos-boostssl::boostssl-httpserver jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto)

add_executable(pub-client pub_client.cpp)
target_link_libraries(pub-client PUBLIC ${CPP_SDK_TARGET} ${EVENT_TARGET} ${JSON_RPC_TARGET} ${AMOP_TARGET} ${WS_SERVICE_TARGET} bcos-boostssl::boostssl-websocket bcos-boostssl::boostssl-utilities bcos-boostssl::boostssl-context bcos-boostssl::boostssl-httpserver jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto)

add_executable(broadcast-client broadcast_client.cpp)
target_link_libraries(broadcast-client PUBLIC ${CPP_SDK_TARGET} ${EVENT_TARGET} ${JSON_RPC_TARGET} ${AMOP_TARGET} ${WS_SERVICE_TARGET} bcos-boostssl::boostssl-websocket bcos-boostssl::boostssl-utilities bcos-boostssl::boostssl-context bcos-boostssl::boostssl-httpserver jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto)

add_executable(event-sub event_sub.cpp)
target_link_libraries(event-sub PUBLIC ${CPP_SDK_TARGET} ${EVENT_TARGET} ${JSON_RPC_TARGET} ${AMOP_TARGET} ${WS_SERVICE_TARGET} bcos-boostssl::boostssl-websocket bcos-boostssl::boostssl-utilities bcos-boostssl::boostssl-context bcos-boostssl::boostssl-httpserver jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto)

add_executable(block-notifier block_notifier.cpp)
target_link_libraries(block-notifier PUBLIC ${CPP_SDK_TARGET} ${EVENT_TARGET} ${JSON_RPC_TARGET} ${AMOP_TARGET} ${WS_SERVICE_TARGET} bcos-boostssl::boostssl-websocket bcos-boostssl::boostssl-utilities bcos-boostssl::boostssl-context bcos-boostssl::boostssl-httpserver jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto)

add_executable(get-group-info get_group_info.cpp)
target_link_libraries(get-group-info PUBLIC ${CPP_SDK_TARGET} ${EVENT_TARGET} ${JSON_RPC_TARGET} ${AMOP_TARGET} ${WS_SERVICE_TARGET} bcos-boostssl::boostssl-websocket bcos-boostssl::boostssl-utilities bcos-boostssl::boostssl-context bcos-boostssl::boostssl-httpserver jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto)
