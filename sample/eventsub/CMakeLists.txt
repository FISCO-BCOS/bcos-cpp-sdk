set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

# OpenSSL
hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

# bcos-boostssl
hunter_add_package(bcos-boostssl)
find_package(bcos-boostssl REQUIRED)

add_executable(eventsub eventsub.cpp)
target_link_libraries(eventsub PUBLIC ${CPP_SDK_TARGET} ${EVENT_TARGET} ${JSON_RPC_TARGET} ${AMOP_TARGET} ${WS_SERVICE_TARGET} bcos-boostssl::boostssl-websocket bcos-boostssl::boostssl-utilities bcos-boostssl::boostssl-context bcos-boostssl::boostssl-httpserver jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto)