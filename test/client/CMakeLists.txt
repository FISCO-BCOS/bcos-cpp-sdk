set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

find_package(OpenSSL)

add_executable(sub-client sub_client.cpp)
target_link_libraries(sub-client PUBLIC ${CPP_SDK_TARGET} bcos-boostssl::boostssl-websocket bcos-boostssl::boostssl-httpserver bcos-boostssl::boostssl-context bcos-boostssl::utilities ${JSON_RPC_TARGET} ${AMOP_TARGET} ${EVENT_TARGET} ${WS_SERVICE_TARGET} jsoncpp_lib_static)

add_executable(pub-client pub_client.cpp)
target_link_libraries(pub-client PUBLIC ${CPP_SDK_TARGET} bcos-boostssl::boostssl-websocket bcos-boostssl::boostssl-httpserver bcos-boostssl::boostssl-context bcos-boostssl::utilities ${JSON_RPC_TARGET} ${AMOP_TARGET} ${EVENT_TARGET} ${WS_SERVICE_TARGET} jsoncpp_lib_static)

add_executable(broadcast-client broadcast_client.cpp)
target_link_libraries(broadcast-client PUBLIC ${CPP_SDK_TARGET} bcos-boostssl::boostssl-websocket bcos-boostssl::boostssl-httpserver bcos-boostssl::boostssl-context bcos-boostssl::utilities ${JSON_RPC_TARGET} ${AMOP_TARGET} ${EVENT_TARGET} ${WS_SERVICE_TARGET} jsoncpp_lib_static)

add_executable(event-sub event_sub.cpp)
target_link_libraries(event-sub PUBLIC ${CPP_SDK_TARGET} bcos-boostssl::boostssl-websocket bcos-boostssl::boostssl-httpserver bcos-boostssl::boostssl-context bcos-boostssl::utilities ${JSON_RPC_TARGET} ${AMOP_TARGET} ${EVENT_TARGET} ${WS_SERVICE_TARGET} jsoncpp_lib_static)
