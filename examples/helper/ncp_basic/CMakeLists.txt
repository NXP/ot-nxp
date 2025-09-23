#
# Copyright (c) 2025, The OpenThread Authors.
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

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(ncp-basic LANGUAGES C CXX ASM
    PROJECT_BOARD_PORT_PATH ${board_root}/${board}/wireless_examples
)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_macro(
    USE_NBU=1
    HAL_RPMSG_SELECT_ROLE=0
    TM_ENABLE_TIME_STAMP=1
    FSL_OSA_TASK_ENABLE=1
    gNvStorageIncluded_d=1
    gPlatformShutdownEccRamInLowPower=0
    gMemManagerLightExtendHeapAreaUsage=1
    gAppHighSystemClockFrequency_d=1
    SERIAL_MANAGER_NON_BLOCKING_MODE=1
	gAppLowpowerEnabled_d=1
)

mcux_add_armgcc_configuration(
    LD "-Xlinker --defsym=gUseNVMLink_d=1"
)

mcux_add_source(
    BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
    SOURCES
            ncp_basic_main.c
            ncp_basic.c
            serial_uart.c
            serial_rpmsg.c
            ncp_serial_intf.h
)
