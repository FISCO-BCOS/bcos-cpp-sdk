set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(${CONFIG_TARGET} ${SRC_LIST} ${HEADERS})
target_link_libraries(${CONFIG_TARGET} PUBLIC bcos-boostssl::boostssl-utilities)