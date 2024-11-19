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

find_package(McuxSDK 3.0.0)

project(sdk-lib LANGUAGES C CXX ASM
    PROJECT_TYPE LIBRARY
    PROJECT_BOARD_PORT_PATH examples/_boards/${board}/wireless_examples
    CUSTOM_PRJ_CONF_PATH examples/_common/project_segments/wireless/wireless_mcu
)

# SDK platform specific configuration
set(SDK_CONF_FILE ${OT_NXP_ROOT}/third_party/${OT_NXP_PLATFORM}_sdk/config.cmake)
if(EXISTS ${SDK_CONF_FILE})
    include(${SDK_CONF_FILE})
endif()

# ot platform specific configuration
#include(${OT_NXP_ROOT}/src/platform.cmake)

# mbedtls configuration
# Since each platform mbedtls config file includes the openthread
# mbedtls config file, add the dependencies here to avoid issues
# when building the SDK target.
mcux_add_include(
    BASE_PATH ${OT_NXP_ROOT}
    INCLUDES
    openthread/include
    openthread/src/core
    openthread/third_party/mbedtls
    third_party/mbedtls/configs
)

mcux_add_macro(
    MBEDTLS_CONFIG_FILE=\\\"${OT_NXP_PLATFORM}-mbedtls-config.h\\\"
    MBEDTLS_USER_CONFIG_FILE=\\\"${OT_NXP_PLATFORM}-mbedtls-config.h\\\"
)

# Propagate SDK deps to the parent
get_target_property(SdkIncludeDirs ${MCUX_SDK_PROJECT_NAME} INTERFACE_INCLUDE_DIRECTORIES)
get_property(SdkCompileDefinitions TARGET ${MCUX_SDK_PROJECT_NAME} PROPERTY INTERFACE_COMPILE_DEFINITIONS)
get_property(SdkCompileOptions TARGET ${MCUX_SDK_PROJECT_NAME} PROPERTY INTERFACE_COMPILE_OPTIONS)

set(SDK_INCLUDE_DIRS ${SdkIncludeDirs} PARENT_SCOPE)
set(SDK_COMPILE_DEFINITIONS ${SdkCompileDefinitions} PARENT_SCOPE)
set(SDK_COMPILE_OPTIONS ${SdkCompileOptions} PARENT_SCOPE)
