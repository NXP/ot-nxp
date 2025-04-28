#
# Copyright (c) 2024-2025, The OpenThread Authors.
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

cmake_minimum_required(VERSION 3.30)

get_filename_component(OT_NXP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../ REALPATH)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

list(APPEND PRJ_CONF_PATH "${OT_NXP_ROOT}/third_party/${OT_NXP_PLATFORM}_sdk")

if(OT_NXP_LWIP_ETH)
    list(APPEND PRJ_CONF_PATH "${OT_NXP_ROOT}/third_party/${OT_NXP_PLATFORM}_sdk/eth")
endif()

# For multi core platforms, the SDK target will be project name plus
# suffix for core id: e.g. ${NXP_DRIVER_LIB}_cm33_core0.
# ${MCUX_SDK_PROJECT_NAME} holds the SDK target name.
if(OT_NXP_LWIP_WIFI OR OT_NXP_LWIP_ETH OR OT_NXP_BUILD_APP_AS_LIB)
    project(${NXP_DRIVER_LIB} LANGUAGES C CXX ASM
        PROJECT_TYPE LIBRARY
        PROJECT_BOARD_PORT_PATH examples/_boards/${board}/wireless_examples
        CUSTOM_PRJ_CONF_PATH ${PRJ_CONF_PATH}
    )
else()
    if (OT_NXP_PLATFORM MATCHES "^mcxw30$")
        project(${NXP_DRIVER_LIB} LANGUAGES C CXX ASM
            PROJECT_TYPE LIBRARY_OBJECT
            PROJECT_BOARD_PORT_PATH examples_int/_boards/${board}/wireless_examples
            CUSTOM_PRJ_CONF_PATH ${PRJ_CONF_PATH}
        )
    else()
        project(${NXP_DRIVER_LIB} LANGUAGES C CXX ASM
            PROJECT_TYPE LIBRARY_OBJECT
            PROJECT_BOARD_PORT_PATH examples/_boards/${board}/wireless_examples
            CUSTOM_PRJ_CONF_PATH ${PRJ_CONF_PATH}
        )
    endif()
endif()

# check that ${OT_MCUX_SDK_TARGET} is correct
if(NOT ${OT_MCUX_SDK_TARGET} STREQUAL ${MCUX_SDK_PROJECT_NAME})
    message(FATAL_ERROR "Unexpected SDK target name: OT_MCUX_SDK_TARGET != MCUX_SDK_PROJECT_NAME")
endif()

include(${SdkRootDirPath}/CMakeLists.txt)

# SDK platform specific configuration
include(${OT_NXP_ROOT}/third_party/${OT_NXP_PLATFORM}_sdk/config.cmake OPTIONAL)

# SDK mbedtls configuration.
# Since each platform mbedtls config file includes the openthread
# mbedtls config file, add the dependencies here to avoid issues
# when building the SDK target. This also includes the OT project
# file, which is used internally in the mbedtls config file to select
# features based on OT flags.
mcux_add_include(
    BASE_PATH ${OT_NXP_ROOT}
    INCLUDES
    openthread/include
    openthread/src/core
    openthread/third_party/mbedtls
    src/${OT_NXP_PLATFORM_FAMILY}/${OT_NXP_PLATFORM}
    src/${OT_NXP_PLATFORM}
    third_party/mbedtls/configs
)

if (OT_EXTERNAL_MBEDTLS)
    mcux_add_macro(
        MBEDTLS_CONFIG_FILE=\\\"${OT_NXP_PLATFORM}-mbedtls-config.h\\\"              # includes "mbedtls-config.h" which includes "openthread-core-config.h" which includes
        OPENTHREAD_PROJECT_CORE_CONFIG_FILE=\\\"${OT_CORE_CONFIG_FILE_UNQUOTED}\\\"  # OPENTHREAD_PROJECT_CORE_CONFIG_FILE if defined. "mbedtls-config.h" checks for OPENTHREAD_CONFIG_*
    )
endif()

if (OT_APP_CLI AND OT_FTD)
    target_link_libraries(ot-cli-ftd PRIVATE
        ${OT_MCUX_SDK_TARGET}
    )
endif()

if (OT_APP_CLI AND OT_MTD)
    target_link_libraries(ot-cli-mtd PRIVATE
        ${OT_MCUX_SDK_TARGET}
    )
endif()

if (OT_APP_RCP AND OT_RCP)
    target_link_libraries(ot-rcp PRIVATE
        ${OT_MCUX_SDK_TARGET}
    )
endif()
