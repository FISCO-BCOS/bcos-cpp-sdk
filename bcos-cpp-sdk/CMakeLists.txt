
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_subdirectory(rpc)
add_subdirectory(amop)
add_subdirectory(event)
add_subdirectory(ws)

# dynamic link
add_library(${CPP_SDK_DY_TARGET} SHARED ${SRC_LIST} ${HEADERS})
target_link_libraries(${CPP_SDK_DY_TARGET} PUBLIC bcos-boostssl::boostssl-websocket bcos-boostssl::boostssl-httpserver bcos-boostssl::boostssl-context ${JSON_RPC_TARGET} ${AMOP_TARGET} ${EVENT_TARGET} ${WS_SERVICE_TARGET} jsoncpp_lib_static bcos-framework::utilities)

# static link
add_library(${CPP_SDK_TARGET} ${SRC_LIST} ${HEADERS})
target_link_libraries(${CPP_SDK_TARGET} PUBLIC bcos-boostssl::boostssl-websocket bcos-boostssl::boostssl-httpserver bcos-boostssl::boostssl-context ${JSON_RPC_TARGET} ${AMOP_TARGET} ${EVENT_TARGET} ${WS_SERVICE_TARGET} jsoncpp_lib_static bcos-framework::utilities)


