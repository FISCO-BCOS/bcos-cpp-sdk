set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(${JSON_RPC_TARGET} ${SRC_LIST} ${HEADERS})

target_link_libraries(${JSON_RPC_TARGET} PUBLIC bcos-framework::utilities)
