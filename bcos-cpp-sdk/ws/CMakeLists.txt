set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(${WS_SERVICE_TARGET} ${SRC_LIST})
target_link_libraries(${WS_SERVICE_TARGET} PUBLIC ${AMOP_TARGET} bcos-framework::utilities)