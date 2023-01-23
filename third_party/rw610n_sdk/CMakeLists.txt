#
#  Copyright (c) 2021, The OpenThread Authors.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of the copyright holder nor the
#     names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#

set_property(SOURCE $ENV{NXP_RW610N_SDK_ROOT}/middleware/wireless/framework/Common/devices/RW610N/gcc/startup_rw610_bt_cm3_fpga.S PROPERTY LANGUAGE C)

set(CONNFWK_FILES_PATH $ENV{NXP_RW610N_SDK_ROOT}/middleware/wireless/framework)
set(802_15_4_FILES_PATH $ENV{NXP_RW610N_SDK_ROOT}/middleware/wireless/ieee-802.15.4)
set(XCVR_FILES_PATH $ENV{NXP_RW610N_SDK_ROOT}/middleware/wireless/nb_fw_dev/src/ffu/smac/src/conn_test/framework/XCVR)


add_library(${NXP_DRIVER_LIB}
#SDK drivers
    $ENV{NXP_RW610N_SDK_ROOT}/platform/drivers/ltc/fsl_ltc.c
    $ENV{NXP_RW610N_SDK_ROOT}/platform/utilities/misc_utilities/fsl_format.c
#SDK components
    $ENV{NXP_RW610N_SDK_ROOT}/components/serial_manager/fsl_component_serial_manager.c
    $ENV{NXP_RW610N_SDK_ROOT}/components/serial_manager/fsl_component_serial_port_uart.c
    $ENV{NXP_RW610N_SDK_ROOT}/components/messaging/fsl_component_messaging.c
    $ENV{NXP_RW610N_SDK_ROOT}/components/mem_manager/fsl_component_mem_manager_light.c
    $ENV{NXP_RW610N_SDK_ROOT}/components/lists/fsl_component_generic_list.c
    $ENV{NXP_RW610N_SDK_ROOT}/components/osa/fsl_os_abstraction_bm.c
#ConnFWK files
    ${CONNFWK_FILES_PATH}/Common/devices/RW610N/gcc/startup_rw610_bt_cm3_fpga.S
    ${CONNFWK_FILES_PATH}/Common/devices/RW610N/system_RW610N_BT_CM3.c
    ${CONNFWK_FILES_PATH}/Common/devices/RW610N/fsl_apuart.c
    ${CONNFWK_FILES_PATH}/Common/devices/RW610N/fsl_adapter_apuart.c
    ${CONNFWK_FILES_PATH}/boards/rw610n_bt_cm3/board.c
    ${CONNFWK_FILES_PATH}/boards/rw610n_bt_cm3/hardware_init.c
    ${CONNFWK_FILES_PATH}/FunctionLib/FunctionLib.c
    ${CONNFWK_FILES_PATH}/SecLib/SecLib.c
    ${CONNFWK_FILES_PATH}/RNG/RNG.c
    ${CONNFWK_FILES_PATH}/HDI/RW610N/hdi.c
#ieee-802.15.4 files
    ${802_15_4_FILES_PATH}/ieee_802_15_4/phy/source/RW610N/PhyISR.c
    ${802_15_4_FILES_PATH}/ieee_802_15_4/phy/source/RW610N/PhyPacketProcessor.c
    ${802_15_4_FILES_PATH}/ieee_802_15_4/phy/source/RW610N/PhyPlmeData.c
    ${802_15_4_FILES_PATH}/ieee_802_15_4/phy/source/RW610N/PhyStateMachine.c
    ${802_15_4_FILES_PATH}/ieee_802_15_4/phy/source/RW610N/PhyTime.c
#XCVR files (currently in nb_fw_dev private repo)
    ${XCVR_FILES_PATH}/drv/mathfp.c
    ${XCVR_FILES_PATH}/drv/nb2p4ghz/nxp2p4_xcvr.c
    ${XCVR_FILES_PATH}/drv/nb2p4ghz/nxp_xcvr_trim.c
    ${XCVR_FILES_PATH}/drv/nb2p4ghz/configs/gen45/nxp_xcvr_coding_config.c
    ${XCVR_FILES_PATH}/drv/nb2p4ghz/configs/gen45/nxp_xcvr_common_config.c
    ${XCVR_FILES_PATH}/drv/nb2p4ghz/configs/gen45/nxp_xcvr_mode_config.c
    ${XCVR_FILES_PATH}/drv/nb2p4ghz/configs/gen45/nxp_xcvr_msk_config.c
    ${XCVR_FILES_PATH}/drv/nb2p4ghz/configs/gen45/nxp_xcvr_oqpsk_802p15p4_config.c
)

target_include_directories(${NXP_DRIVER_LIB}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/rw/rw610n/platform
        ${PROJECT_SOURCE_DIR}/src/rw/rw610n
#CMSIS includes
        $ENV{NXP_RW610N_SDK_ROOT}/CMSIS/Core/Include
#SDK drivers
        $ENV{NXP_RW610N_SDK_ROOT}/platform/drivers/common
        $ENV{NXP_RW610N_SDK_ROOT}/platform/drivers/ltc
        $ENV{NXP_RW610N_SDK_ROOT}/platform/utilities/misc_utilities
#SDK components
        $ENV{NXP_RW610N_SDK_ROOT}/components/uart
        $ENV{NXP_RW610N_SDK_ROOT}/components/serial_manager
        $ENV{NXP_RW610N_SDK_ROOT}/components/messaging
        $ENV{NXP_RW610N_SDK_ROOT}/components/mem_manager
        $ENV{NXP_RW610N_SDK_ROOT}/components/lists
        $ENV{NXP_RW610N_SDK_ROOT}/components/osa
#Framework files
        ${CONNFWK_FILES_PATH}/Common
        ${CONNFWK_FILES_PATH}/Common/devices/RW610N
        ${CONNFWK_FILES_PATH}/platform/RW610N_BT_CM3
        ${CONNFWK_FILES_PATH}/boards/rw610n_bt_cm3
        ${CONNFWK_FILES_PATH}/SecLib
        ${CONNFWK_FILES_PATH}/RNG
        ${CONNFWK_FILES_PATH}/HDI
        ${CONNFWK_FILES_PATH}/FunctionLib
#ieee-802.15.4 files
        ${802_15_4_FILES_PATH}/ieee_802_15_4/phy/source/RW610N
        ${802_15_4_FILES_PATH}/ieee_802_15_4/phy/interface
#XCVR files (currently in nb_fw_dev private repo)
        ${XCVR_FILES_PATH}/drv
        ${XCVR_FILES_PATH}/drv/nb2p4ghz
        ${XCVR_FILES_PATH}/drv/nb2p4ghz/configs/gen45
)

target_compile_definitions(${NXP_DRIVER_LIB}
    PUBLIC
    -D__STARTUP_CLEAR_BSS
    -DHDI_MODE=1
    -DCPU_RW610N_BT_CM3
    -DMARVELL_BT_FPGA
    -DgPhyUseNeighborTable_d=1
	-DSERIAL_MANAGER_NON_BLOCKING_MODE=1
	-DSERIAL_USE_CONFIGURE_STRUCTURE=1
	-DSDK_COMPONENT_INTEGRATION=1
	-DSERIAL_PORT_TYPE_UART=1
	-DRADIO_IS_GEN_4P5=1
)

target_link_libraries(${NXP_DRIVER_LIB}
    PRIVATE
        ${OT_PLATFORM_LIB}
        ${CONNFWK_FILES_PATH}/SecLib/lib_crypto_m3.a
)

target_compile_options(${NXP_DRIVER_LIB}
    PRIVATE
        ${OT_CFLAGS}
        -Wno-unknown-pragmas
        -Wno-sign-compare
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-unused-label
        -Wno-unused-variable
        -Wno-unused-but-set-variable
        -Wno-empty-body
)
