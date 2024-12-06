#
# Copyright (c) 2024, The OpenThread Authors.
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

# For multi core platforms, the actual target will be driver lib plus
# a suffix related to core id: e.g. ${NXP_DRIVER_LIB}_cm33_core0
if(OT_NXP_LWIP_WIFI OR OT_NXP_LWIP_ETH)
project(${NXP_DRIVER_LIB} LANGUAGES C CXX ASM
    PROJECT_TYPE LIBRARY
    PROJECT_BOARD_PORT_PATH examples/_boards/${board}/wireless_examples
    CUSTOM_PRJ_CONF_PATH ${PRJ_CONF_PATH}
)
else()
project(${NXP_DRIVER_LIB} LANGUAGES C CXX ASM
    PROJECT_TYPE LIBRARY_OBJECT
    PROJECT_BOARD_PORT_PATH examples/_boards/${board}/wireless_examples
    CUSTOM_PRJ_CONF_PATH ${PRJ_CONF_PATH}
)
endif()

set(MCUX_SDK_PROJECT_NAME ${MCUX_SDK_PROJECT_NAME} PARENT_SCOPE)

include(${SdkRootDirPath}/CMakeLists.txt)

# SDK platform specific configuration
include(${OT_NXP_ROOT}/third_party/${OT_NXP_PLATFORM}_sdk/config.cmake OPTIONAL)

# mbedtls configuration
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
    src/${OT_NXP_PLATFORM}
    third_party/mbedtls/configs
)

mcux_add_macro(
    MBEDTLS_CONFIG_FILE=\\\"${OT_NXP_PLATFORM}-mbedtls-config.h\\\"
    OPENTHREAD_PROJECT_CORE_CONFIG_FILE=\\\"${CORE_CONFIG_FILE}\\\"
)
