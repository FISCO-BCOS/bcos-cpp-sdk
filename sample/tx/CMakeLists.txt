set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_executable(deploy_hello deploy_hello.cpp)
target_link_libraries(deploy_hello PUBLIC ${BCOS_CPP_SDK_TARGET} bcos-boostssl::bcos-boostssl bcos-utilities::bcos-utilities jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto)