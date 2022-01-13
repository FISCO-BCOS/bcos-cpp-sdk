file(GLOB_RECURSE SRC_LIST "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")

add_library(${BCOS_CPP_SDK_TARGET} ${SRC_LIST} ${HEADERS})
target_compile_options(${BCOS_CPP_SDK_TARGET} PRIVATE -Wno-error -Wno-unused-variable)
target_link_libraries(${BCOS_CPP_SDK_TARGET} PUBLIC wedpr-crypto::crypto wedpr-crypto::extend-crypto bcos-boostssl::bcos-boostssl bcos-utilities::bcos-utilities jsoncpp_lib_static tarscpp::tarsutil OpenSSL::SSL OpenSSL::Crypto)
