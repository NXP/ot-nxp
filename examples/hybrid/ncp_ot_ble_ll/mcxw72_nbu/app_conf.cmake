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

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}/middleware/wireless/bluetooth
    PREINCLUDE TRUE
    SOURCES boards/${board}/bluetooth/ncp_fscibb/app_preinclude.h
)

mcux_add_macro(
    gMatterConfig_d=1
    SDK_COMPONENT_INTEGRATION=1
    gUseHciTransportUpward_d=0
    HCI_FREE_RxBuffer=0
    gMWS_Enabled_d=1
    HDI_MODE=0
    RF_OSC_26MHZ=0
    FPGA_TARGET=0
    PHY_15_4_LOW_POWER_ENABLED=1
    FSL_RTOS_THREADX
    TX_INCLUDE_USER_DEFINE_FILE
)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}/middleware/wireless/ble_controller
    SOURCES
        src/KW4x/nbu_version.c
        src/KW4x/sw_version.h
        src/KW4x/controller_init.c
        src/KW4x/controller_api_ll.c
        src/KW4x/ll_types.h
        interface/controller_init.h
        lib/mll_inc/controller_api_ll.h
        lib/mll_inc/ble_debug_struct.h
        src/KW4x/app.h
        src/KW4x/board.h
        src/KW4x/board.c
        src/KW4x/armgcc/nbu_ble_wrap.s
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}/middleware/wireless/ble_controller
    INCLUDES
        src/KW4x
        interface
        lib/mll_inc
)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}/middleware/wireless/bluetooth
    SOURCES
           boards/${board}/app_preinclude_common.h
           examples/ncp_fsci_black_box/ncp_fsci_black_box.c
           examples/ncp_fsci_black_box/ncp_fsci_black_box.h
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}/middleware/wireless/bluetooth
    INCLUDES
        boards/${board}
        examples/ncp_fsci_black_box
)

mcux_add_configuration(
    TOOLCHAINS armgcc
    AS "-DINIT_BLE_RL_DAT"
)
