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
set(CMAKE_OSX_DEPLOYMENT_TARGET "12" CACHE STRING "Minimum OS X deployment version")
include(FetchContent)

if(NOT DEFINED URL_BASE)
    set(URL_BASE "github.com")

    # set(URL_BASE "github.com.cnpmjs.org")
endif()

include(FetchContent)
set(BCOS_CMAKE_SCRIPTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/bcos-cmake-scripts)
FetchContent_Declare(bcos-cmake-scripts
    GIT_REPOSITORY https://${URL_BASE}/FISCO-BCOS/bcos-cmake-scripts.git
    GIT_TAG 364f603f4ec112cb79840ea4026243399d7c419b
    SOURCE_DIR ${BCOS_CMAKE_SCRIPTS_DIR}
)
FetchContent_MakeAvailable(bcos-cmake-scripts)

set(BCOS_RPC_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "The cmake path for the library")
list(APPEND CMAKE_MODULE_PATH ${BCOS_CMAKE_SCRIPTS_DIR})
list(APPEND CMAKE_MODULE_PATH ${BCOS_RPC_CMAKE_DIR})

# init hunter
include(Options)
set(ARCH_NATIVE OFF)

if("${ARCHITECTURE}" MATCHES "aarch64")
    set(ARCH_NATIVE ON)
endif()

include(HunterGate)
HunterGate(
    URL "https://${URL_BASE}/FISCO-BCOS/hunter/archive/162ff7cde3d843e43afdab8fe3214275c35ef168.tar.gz"
    SHA1 "0ba8099fe89beb0d44986c750fe0d44040725d99"
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

hunter_add_package(Microsoft.GSL)

# bcos-utilities
include(InstallBcosUtilities)

# bcos-boostssl
hunter_add_package(bcos-boostssl)
find_package(bcos-boostssl CONFIG REQUIRED)

# bcos-crypto
include(InstallBcosCrypto)

# basic settings
include(Options)
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
