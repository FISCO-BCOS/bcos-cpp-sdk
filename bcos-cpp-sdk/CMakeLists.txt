
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_subdirectory(client)
add_subdirectory(rpc)
add_subdirectory(ws)

add_library(${CPP_SDK_TARGET} ${SRC_LIST} ${HEADERS})
target_link_libraries(${CPP_SDK_TARGET} PUBLIC jsoncpp_lib_static bcos-framework::utilities)