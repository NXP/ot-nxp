#
# Copyright (c) 2021-2025, The OpenThread Authors.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the
# names of its contributors may be used to endorse or promote products
# derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

cmake_minimum_required(VERSION 3.10.2)

message(STATUS "SdkRootDirPath ${SdkRootDirPath}")

if(DEFINED SdkRootDirPath)
    cmake_path(SET SdkRootDirPath ${SdkRootDirPath})
    include(${SdkRootDirPath}/cmake/extension/logging.cmake)
    include(${SdkRootDirPath}/cmake/toolchain/toolchain.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain/arm-none-eabi.cmake)
endif()

project(ot-nxp
    VERSION 0.2.0
    LANGUAGES C CXX ASM
)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/platform)

include(utils)

# Explicitly add some platform specific compile options to avoid
# running into issues related to CPU architecture mismatch. This
# happens because some targets (e.g. SDK) are built with some
# options which are not available for some targets in the first
# run generation. Once the cmake flags are cached, the targets are
# built correctly upon subsequent runs.
include(${OT_NXP_PLATFORM} OPTIONAL)
include(options)

otnxp_git_version(OT_PACKAGE_VERSION)
set(OT_PACKAGE_VERSION "${OT_PACKAGE_VERSION}" CACHE STRING "")
message(STATUS "Setting default package version: ${OT_PACKAGE_VERSION}")
message(STATUS "Package Version: ${OT_PACKAGE_VERSION}")
message(STATUS "NXP Platform: ${OT_NXP_PLATFORM}")

set(PRJ_CONF_PATH "" CACHE STRING "Define all custom prj.conf files")

set(OT_PLATFORM_LIB openthread-${OT_NXP_PLATFORM})
set(NXP_DRIVER_LIB nxp-${OT_NXP_PLATFORM}-driver)

if(DEFINED SdkRootDirPath)
    if(DEFINED core_id)
        set(OT_EXTERNAL_MBEDTLS ${NXP_DRIVER_LIB}_${core_id} CACHE STRING "")
    else()
        set(OT_EXTERNAL_MBEDTLS ${NXP_DRIVER_LIB} CACHE STRING "")
    endif()

    # This variable will be set in the SDK project. This is necessary in order
    # to have a valid target name in other scopes, such as src/platform scope.
    # Some platforms are dual core, in which case the SDK framework will add
    # a suffix to the SDK target name: e.g. ${NXP_DRIVER_LIB}_cm33_core0
    set(MCUX_SDK_PROJECT_NAME "")
endif()

# Set default log level if not set
if(OT_STACK_ENABLE_LOG)
    if(NOT OT_LOG_LEVEL)
        set(OT_LOG_LEVEL INFO)
    endif()

    if(NOT OT_LOG_OUTPUT)
        set(OT_LOG_OUTPUT APP)
    endif()
else()
    set(OT_LOG_LEVEL NONE)
    set(OT_LOG_OUTPUT NONE)
endif()

if(OT_NCP_RTOS_HOST)
    message(WARNING "select ot nxp ncp host, the openthread stack will be not compiled.")
    add_library(ot-config INTERFACE)
    add_library(openthread-cli-ftd INTERFACE)
    add_library(openthread-ftd INTERFACE)
    add_library(ot-cli-addons INTERFACE)
else()
    add_subdirectory(openthread)
endif()

# If OT logging is enabled add a dependency to SDK log components
if(OT_STACK_ENABLE_LOG)
    target_link_libraries(ot-config
        INTERFACE
        ${NXP_DRIVER_LIB})
    target_compile_definitions(ot-config
        INTERFACE
        -DOT_STACK_ENABLE_LOG)
endif()

set(CONFIG_FILE "\"openthread-core-${OT_NXP_PLATFORM}-config.h\"")
set(CORE_CONFIG_FILE ${CONFIG_FILE})
set(CORE_CONFIG_CHECK "\"openthread-core-${OT_NXP_PLATFORM}-config-check.h\"")
set(OT_LIB_CONFIG "\"openthread-core-${OT_NXP_PLATFORM}-config.h\"")

target_compile_definitions(ot-config INTERFACE
    OPENTHREAD_CONFIG_FILE=${CONFIG_FILE}
    OPENTHREAD_PROJECT_CORE_CONFIG_FILE=${CORE_CONFIG_FILE}
    OPENTHREAD_CORE_CONFIG_PLATFORM_CHECK_FILE=${CORE_CONFIG_CHECK}
    OPENTHREAD_PROJECT_LIB_CONFIG_FILE=${OT_LIB_CONFIG}
)

if(OT_NXP_PLATFORM_FAMILY)
    target_include_directories(ot-config INTERFACE
        ${PROJECT_SOURCE_DIR}/src/${OT_NXP_PLATFORM_FAMILY}/${OT_NXP_PLATFORM}
    )
else()
    target_include_directories(ot-config INTERFACE
        ${PROJECT_SOURCE_DIR}/src/${OT_NXP_PLATFORM}
    )
endif()

if(NOT FREERTOS_CONFIG_FILE_DIRECTORY)
    set(FREERTOS_CONFIG_FILE_DIRECTORY ${PROJECT_SOURCE_DIR}/boards/${OT_NXP_BOARD}/freertos CACHE STRING "")
endif()

# Give the possibility to the user to define his own path to board files
if(OT_NXP_BOARD_PATH)
    message(STATUS "Using user defined board folder:\n"
        "${OT_NXP_BOARD_PATH}"
    )

    if(NOT NXP_BOARD_LIB)
        message(WARNING "The name of the board lib is not specified. It may not be linked properly by ot-nxp targets\n"
            "You can add the link dependency in your projet:\n"
            "target_link_libraries(<ot-nxp-target> <your board target>)\n"
            "Or you can define the name of your board library:\n"
            "-DNXP_BOARD_LIB=<name of the board lib>\n"
            "or set it in your project before including ot-nxp:\n"
            "set(NXP_BOARD_LIB <name of the board lib>)"
        )
    endif()

    add_subdirectory(${OT_NXP_BOARD_PATH} ${PROJECT_BINARY_DIR}/board)
elseif(OT_NXP_BOARD) # TODO: move to "else" when all boards are supported
    set(NXP_BOARD_LIB nxp-${OT_NXP_BOARD}-board)
    add_subdirectory(boards)
else()
    # TODO: enable following message when all boards are supported
    # if(NOT OT_NXP_BOARD)
    # message(FATAL_ERROR  "Board is not defined. Please specify to board to build:\n"
    # "-DOT_NXP_BOARD=<board name>"
    # )
    # endif()
endif()

# Flag used to determine if the RCP transceiver used is K32W0 or IW/RW. Since K32W0 RCP has 
# a fix for the extended frame pending bit issue and IW/RW do not, the build flag is used to
# determine in the radio spinel code the correct endianness to send to the RCP transceiver.
if("${OT_NXP_TRANSCEIVER}" STREQUAL "k32w0")
    target_compile_definitions(ot-config INTERFACE
        -DOT_NXP_TRANSCEIVER_k32w0
    )
endif()

if(NOT NXP_SDK_ROOT)
    # Provide a default path to MCUXpresso SDK as if ot-nxp is located in <path to sdk>/middleware/wireless/
    set(NXP_SDK_ROOT ${PROJECT_SOURCE_DIR}/../../../)
endif()

# Determine whether the SDK used is the github repo, a package or the internal repo
if(EXISTS ${NXP_SDK_ROOT}/core/.github)
    set(SDK_TYPE "GITHUB")
elseif(EXISTS ${NXP_SDK_ROOT}/SW-Content-Register.txt)
    set(SDK_TYPE "PACKAGE")
else()
    set(SDK_TYPE "GITHUB")
endif()

message(STATUS "SDK_TYPE: ${SDK_TYPE}")

if(DEFINED SdkRootDirPath)
    if(OT_NXP_LWIP)
        add_subdirectory(third_party/lwip/lwip_next_gen)
    endif()

    if(OT_NXP_LWIP_WIFI)
        add_subdirectory(third_party/wifi/wifi_next_gen)
    endif()

    add_subdirectory(third_party/common_sdk)
else()
    add_subdirectory(third_party)
endif()

add_subdirectory(src)
add_subdirectory(examples)

if(OT_NXP_POST_BUILD_COMMAND)
    run_post_build_command()
endif()

if(OT_NXP_EXPORT_TO_BIN)
    export_all_targets_to_bin()
endif()

if(OT_NXP_EXPORT_TO_SREC)
    export_all_targets_to_srec()
endif()
