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

list(APPEND PRJ_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

list(APPEND PRJ_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../wifi_${OT_NXP_PLATFORM}")

set(PRJ_CONF_PATH ${PRJ_CONF_PATH} PARENT_SCOPE)

add_library(wifi
    ${SdkRootDirPath}/middleware/wifi_nxp/port/osa/osa_freertos.c
    ${SdkRootDirPath}/middleware/wifi_nxp/port/net/net.c
)

target_compile_definitions(wifi
    PRIVATE
    vApplicationIdleHook=vApplicationIdleHook_wifi # hack to prevent definition of multiple idle tasks (OT has one)
    tcpip_init=tcpip_init_once
)

target_include_directories(wifi
    PUBLIC
    ${SdkRootDirPath}/middleware/wifi_nxp/port/osa
    ${SdkRootDirPath}/middleware/wifi_nxp/port/net
    ${PROJECT_SOURCE_DIR}/third_party/wifi
)

target_link_libraries(wifi
    PRIVATE
    ot-config
    ${NXP_DRIVER_LIB}
    ${NXP_BOARD_LIB}
)
