set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_executable(sub-client sub_client.cpp)
target_link_libraries(sub-client PUBLIC ${CPP_SDK_TARGET} ${WS_SERVICE_TARGET} ${JSON_RPC_TARGET} ${AMOP_TARGET} jsoncpp_lib_static bcos-framework::utilities)

add_executable(pub-client pub_client.cpp)
target_link_libraries(pub-client PUBLIC ${CPP_SDK_TARGET} ${WS_SERVICE_TARGET} ${JSON_RPC_TARGET} ${AMOP_TARGET} jsoncpp_lib_static bcos-framework::utilities)

add_executable(broadcast-client broadcast_client.cpp)
target_link_libraries(broadcast-client PUBLIC ${CPP_SDK_TARGET} ${WS_SERVICE_TARGET} ${JSON_RPC_TARGET} ${AMOP_TARGET} jsoncpp_lib_static bcos-framework::utilities)
