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

project(ncp-advanced LANGUAGES C CXX ASM
    PROJECT_BOARD_PORT_PATH ${board_root}/${board}/wireless_examples
)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_macro(
    USE_NBU=1
    HAL_RPMSG_SELECT_ROLE=0
    TM_ENABLE_TIME_STAMP=1
    FSL_OSA_TASK_ENABLE=1
    SERIAL_MANAGER_NON_BLOCKING_MODE=1
)

mcux_add_source(
    PREINCLUDE TRUE
    BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
    SOURCES custom_preinclude.h
)

mcux_add_source(
    BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
    SOURCES
        ncp_advanced.c
        ../ncp_basic/ncp_basic.c
        ../ncp_basic/serial_uart.c
        ../ncp_basic/serial_rpmsg.c
        ../ncp_basic/ncp_serial_intf.h
)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}/middleware/wireless/bluetooth
    SOURCES
        examples/w_uart_host/app_config.c
        examples/w_uart_host/dynamic_database.c
        examples/w_uart_host/dynamic_database.h
        examples/w_uart_host/w_uart_application.c
        examples/w_uart_host/w_uart_application.h
        examples/w_uart_host/main.c
        boards/${board}/bluetooth/w_uart_host/app_preinclude.h
        boards/${board}/app_preinclude_common.h
)

mcux_add_include(
    BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
    INCLUDES
        ../ncp_basic
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}/middleware/wireless/bluetooth
    INCLUDES 
        application/common
        examples/w_uart_host
        boards/${board}/bluetooth/w_uart_host
        boards/${board}
)

mcux_add_armgcc_configuration(
    LD "-Wl,--wrap=BOARD_InitSerialManager,--wrap=APP_InitServices,--wrap=Shell_Init,--wrap=OSA_ProcessTasks,--wrap=OSA_TaskShouldYield,--wrap=SerialManager_WriteBlocking,--wrap=SerialManager_WriteNonBlocking,--wrap=SerialManager_InstallTxCallback,--wrap=RegisterRemovableObserver,--wrap=GATTClientRegisterProcedureCallbackRequest,--defsym=gUseNVMLink_d=1"
    CC "-Wno-error"
)
