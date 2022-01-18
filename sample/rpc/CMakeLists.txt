set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_executable(blocknotifier blocknotifier.cpp)
target_compile_options(blocknotifier PRIVATE -Wno-error -Wno-unused-variable)
target_link_libraries(blocknotifier PUBLIC ${BCOS_CPP_SDK_TARGET} bcos-boostssl::bcos-boostssl bcos-utilities::bcos-utilities jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto)