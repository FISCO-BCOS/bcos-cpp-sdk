set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-std=c++20 -pthread -fPIC -fvisibility=hidden -fvisibility-inlines-hidden -fexceptions")

file(GLOB_RECURSE SRC_LIST "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")

add_library(${BCOS_CPP_SDK_TARGET} ${SRC_LIST} ${HEADERS})

if (NOT WIN32)
   target_compile_options(${BCOS_CPP_SDK_TARGET} PRIVATE -Wno-error -Wno-unused-variable)
endif()

target_link_libraries(${BCOS_CPP_SDK_TARGET} PUBLIC wedpr-crypto::crypto wedpr-crypto::extend-crypto bcos-crypto::bcos-crypto bcos-boostssl::bcos-boostssl bcos-utilities::bcos-utilities jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto)
