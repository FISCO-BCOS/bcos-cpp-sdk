set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_executable(eventsub eventsub.cpp)
target_link_libraries(eventsub PUBLIC ${BCOS_CPP_SDK_TARGET} bcos-boostssl::bcos-boostssl bcos-utilities::bcos-utilities jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto)