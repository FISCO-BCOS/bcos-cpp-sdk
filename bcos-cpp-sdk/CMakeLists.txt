
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

hunter_add_package(bcos-boostssl)
find_package(bcos-boostssl CONFIG REQUIRED)

add_subdirectory(rpc)
add_subdirectory(amop)
add_subdirectory(event)
add_subdirectory(ws)

add_library(${CPP_SDK_TARGET} ${SRC_LIST} ${HEADERS})
target_link_libraries(${CPP_SDK_TARGET} PUBLIC bcos-boostssl::boostssl-websocket bcos-boostssl::boostssl-utilities bcos-boostssl::boostssl-httpserver bcos-boostssl::boostssl-context  ${WS_SERVICE_TARGET} ${JSON_RPC_TARGET} ${AMOP_TARGET} ${EVENT_TARGET} jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto)

# install bcos-cpp-sdk target
install(
    TARGETS ${CPP_SDK_TARGET}
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
