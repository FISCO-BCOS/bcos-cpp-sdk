set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(${RPC_CLIENT_TARGET} ${SRC_LIST} ${HEADERS})

target_link_libraries(${RPC_CLIENT_TARGET} PUBLIC bcos-framework::utilities)
