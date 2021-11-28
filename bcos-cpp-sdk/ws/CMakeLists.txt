set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(${WS_SERVICE_TARGET} ${SRC_LIST} ${HEADERS})

target_link_libraries(${WS_SERVICE_TARGET} PUBLIC boostssl-utilities jsoncpp_lib_static)
