# ------------------------------------------------------------------------------
# Top-level CMake file for bcos-cpp-sdk
# ------------------------------------------------------------------------------
# Copyright (C) 2021 bcos-cpp-sdk
# SPDX-License-Identifier: Apache-2.0
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-pthread -fPIC -fvisibility=hidden -fvisibility-inlines-hidden -fexceptions")
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.3" CACHE STRING "Minimum OS X deployment version")
include(FetchContent)

if(NOT DEFINED URL_BASE)
    set(URL_BASE "github.com")

    # set(URL_BASE "github.com.cnpmjs.org")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# init hunter
include(Options)
set(ARCH_NATIVE OFF)

if("${ARCHITECTURE}" MATCHES "aarch64")
    set(ARCH_NATIVE ON)
endif()

include(HunterGate)
HunterGate(
    URL "https://${URL_BASE}/FISCO-BCOS/hunter/archive/155dfdb7b5888493eefb656b3abcc026edb98730.tar.gz"
    SHA1 "a797abf35de1904ec089a9f517d847637731d467"
    FILEPATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake"
)

project(bcos-cpp-sdk VERSION "3.0.0")

# Boost
hunter_add_package(Boost COMPONENTS all)
find_package(Boost CONFIG REQUIRED log chrono system filesystem thread iostreams)

# Jsoncpp
hunter_add_package(jsoncpp)
find_package(jsoncpp CONFIG REQUIRED)

# TASSL
hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

# wedpr-crypto
hunter_add_package(wedpr-crypto)
find_package(wedpr-crypto CONFIG REQUIRED)

#hunter_add_package(tarscpp)
#find_package(tarscpp REQUIRED)

hunter_add_package(Microsoft.GSL)

# bcos-utilities
include(InstallBcosUtilities)

# bcos-boostssl
hunter_add_package(bcos-boostssl)
find_package(bcos-boostssl CONFIG REQUIRED)

# hsm-crypto
include(InstallSDF)

# bcos-crypto
include(InstallBcosCrypto)

# basic settings
configure_project()
include(CompilerSettings)
include(InstallConfig)

# install the include files for hash
include(InstallInclude)

include_directories(${CMAKE_INSTALL_INCLUDEDIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/bcos-cpp-sdk)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/bcos-cpp-sdk/bcos-framework)
include_directories(${CMAKE_INSTALL_INCLUDEDIR}/bcos-cpp-sdk)
include_directories(${CMAKE_INSTALL_INCLUDEDIR}/bcos-cpp-sdk/bcos-framework)

set(BCOS_CPP_SDK_TARGET "bcos-cpp-sdk")
add_subdirectory(bcos-cpp-sdk)

if(BUILD_SAMPLE)
    # sample exec binary
    add_subdirectory(sample)
endif()

if(TESTS)
    enable_testing()
    set(CTEST_OUTPUT_ON_FAILURE TRUE)
    add_subdirectory(test)
endif()

# install bcos-cpp-sdk target
install(
    TARGETS ${BCOS_CPP_SDK_TARGET}
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# for doxygen
# include(BuildDocs)

# for code coverage
if(COVERAGE)
    include(Coverage)
    config_coverage("rpc-coverage" "'/usr*' '${CMAKE_CURRENT_SOURCE_DIR}/bcos-cmake-scripts*' '${CMAKE_SOURCE_DIR}/test/mock**' '${CMAKE_SOURCE_DIR}/test/main**'")
endif()
