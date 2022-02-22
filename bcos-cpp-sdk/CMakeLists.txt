
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB_RECURSE SRC_LIST "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")

add_library(${BCOS_CPP_SDK_TARGET} ${SRC_LIST} ${HEADERS})
target_link_libraries(${BCOS_CPP_SDK_TARGET} PUBLIC bcos-boostssl::bcos-boostssl bcos-utilities::bcos-utilities jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto)


