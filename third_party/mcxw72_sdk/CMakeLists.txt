#
#  Copyright (c) 2024, The OpenThread Authors.
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

if (SDK_RELEASE)
    set(DRV_DIR "devices/MCXW727C/drivers" CACHE STRING "")
    set(UTL_DIR "devices/MCXW727C/utilities" CACHE STRING "")
    set(MIDLWR_DIR "middleware" CACHE STRING "")
    set(ZB_EXMPL_DIR "$ENV{NXP_MCXW72_SDK_ROOT}/boards/kw47evk/wireless_examples/zigbee" CACHE STRING "")
else()
    if (SDK_GH)
        set(DRV_DIR "drivers" CACHE STRING "")
        set(UTL_DIR "utilities" CACHE STRING "")
        set(MIDLWR_DIR "../middleware" CACHE STRING "")
        set(ZB_EXMPL_DIR "$ENV{NXP_MCXW72_SDK_ROOT}/../examples/kw47evk/wireless_examples/zigbee" CACHE STRING "")
    else()
        set(DRV_DIR "platform/drivers" CACHE STRING "")
        set(UTL_DIR "platform/utilities" CACHE STRING "")
        set(MIDLWR_DIR "middleware" CACHE STRING "")
        set(ZB_EXMPL_DIR "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/zigbee/examples" CACHE STRING "")
        set(BLE_DIR "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth" CACHE STRING "")
    endif()
endif()
list(APPEND OT_PLATFORM_DEFINES
    "USE_NBU=${USE_NBU}"
    "CPU_MCXW727CMFTA_cm33_core0"
    "SERIAL_USE_CONFIGURE_STRUCTURE=1"
    "SDK_COMPONENT_INTEGRATION=1"
    "SERIAL_PORT_TYPE_UART=1"
    "gSerialManagerMaxInterfaces_c=2"
    "HAL_RPMSG_SELECT_ROLE=0"
    "TM_ENABLE_TIME_STAMP=1"
    "FSL_OSA_TASK_ENABLE=1"
#    "FSL_OSA_MAIN_FUNC_ENABLE=1"
#    "FSL_OSA_BM_TIMER_CONFIG=FSL_OSA_BM_TIMER_SYSTICK"
    "gAspCapability_d=1"
    "gNvStorageIncluded_d=1"
    "gUnmirroredFeatureSet_d=1"
    "gNvFragmentation_Enabled_d=1"
    "gAppButtonCnt_c=2"
    "gBleBondIdentityHeaderSize_c=56"
    "gPlatformShutdownEccRamInLowPower=0"
    "gMemManagerLightExtendHeapAreaUsage=1"
    "gAppHighSystemClockFrequency_d=1"
    "SSS_CONFIG_FILE=\"fsl_sss_config_elemu.h\""
    "SSCP_CONFIG_FILE=\"fsl_sscp_config_elemu.h\""
    "DBG_IO_ENABLE"
    "FWK_RNG_DEPRECATED_API"
    "gHwParamsProdDataPlacement_c=gHwParamsProdDataPlacementLegacyMode_c"
)

if (USE_NBU)
    set_property(SOURCE $ENV{NXP_MCXW72_SDK_ROOT}/devices/MCXW727C/gcc/startup_MCXW727C_cm33_core0.S PROPERTY LANGUAGE C)
else()
    set_property(SOURCE $ENV{NXP_MCXW72_SDK_ROOT}/devices/MCXW727C/gcc/startup_MCXW727C_cm33_core0.S PROPERTY LANGUAGE C)
endif()

if (OT_APP_LOWPOWER)
    list(APPEND OT_PLATFORM_DEFINES
        "gAppLowpowerEnabled_d=1"
    )
else()
    list(APPEND OT_PLATFORM_DEFINES
        "gAppLowpowerEnabled_d=0"
    )
endif()

set(MCXW72_SDK_SOURCES
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/FunctionLib/FunctionLib.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/HWParameter/HWParameter.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/connected_mcu/fwk_platform.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/connected_mcu/fwk_platform_lowpower.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/connected_mcu/fwk_platform_lowpower_timer.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/connected_mcu/fwk_platform_ics.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/connected_mcu/fwk_platform_ot.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/connected_mcu/fwk_platform_extflash.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/connected_mcu/fwk_platform_rng.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/NVM/Source/NV_Flash.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/LowPower/PWR.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/RNG/RNG.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/SecLib/SecLib_sss.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/boards/kw47_mcxw72x/KW47B42ZB7/clock_config.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/boards/kw47_mcxw72x/hardware_init.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/boards/kw47_mcxw72x/app_services_init.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/boards/kw47_mcxw72x/KW47B42ZB7/pin_mux.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/boards/kw47_mcxw72x/board_comp.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/boards/kw47_mcxw72x/board_dcdc.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/boards/kw47_mcxw72x/board_lp.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/boards/kw47_mcxw72x/board.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/boards/kw47_mcxw72x/board_extflash.c
    $ENV{NXP_MCXW72_SDK_ROOT}/devices/MCXW727C/system_MCXW727C_cm33_core0.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/osa/fsl_os_abstraction_bm.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/lists/fsl_component_generic_list.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/messaging/fsl_component_messaging.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/mem_manager/fsl_component_mem_manager_light.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/panic/fsl_component_panic.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/serial_manager/fsl_component_serial_manager.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/serial_manager/fsl_component_serial_port_uart.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/uart/fsl_adapter_lpuart.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/gpio/fsl_adapter_gpio.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/led/fsl_component_led.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/button/fsl_component_button.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/timer_manager/fsl_component_timer_manager.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/timer/fsl_adapter_lptmr.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/time_stamp/fsl_adapter_lptmr_time_stamp.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/internal_flash/fsl_adapter_k4_flash.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/power_manager/devices/KW47B42ZB7/fsl_pm_device.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/power_manager/core/fsl_pm_core.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/flash/nor/lpspi/fsl_lpspi_nor_flash.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/flash/nor/lpspi/fsl_lpspi_mem_adapter.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/reset/fsl_adapter_reset.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/mbedtls/port/sssapi/entropy_poll_alt.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/mbedtls/port/sssapi/sssapi_mbedtls.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/secure-subsystem/src/sscp/fsl_sscp_mu.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/secure-subsystem/src/sscp/fsl_sss_mgmt.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/secure-subsystem/src/sscp/fsl_sss_sscp.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/secure-subsystem/port/kw45_k4w1/sss_init.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/secure-subsystem/port/kw45_k4w1/sss_aes.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/secure-subsystem/port/kw45_k4w1/sss_ccm.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/secure-subsystem/port/kw45_k4w1/sss_aes_cmac.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/secure-subsystem/port/kw45_k4w1/sss_ecdh.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/secure-subsystem/port/kw45_k4w1/sss_sha256.c
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/connected_mcu/fwk_platform_genfsk.c
)

if (USE_NBU)
    if (SDK_RELEASE)
        list(APPEND MCXW72_SDK_SOURCES
            $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_imu.c
        )
    else()
        list(APPEND MCXW72_SDK_SOURCES
            $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/imu/fsl_imu.c
        )
    endif()

    list(APPEND MCXW72_SDK_SOURCES
        # using MCXW727C startup file not working (crush at run time)
        #$ENV{NXP_MCXW72_SDK_ROOT}/devices/MCXW727C/gcc/startup_MCXW727C_cm33_core0.S
        # K32W1480 startup file is used until MCXW727C startup file above will be fixed
        ${PROJECT_SOURCE_DIR}/src/k32w1/k32w1/startup_K32W1480.S
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/PhyTime.c
        $ENV{NXP_MCXW72_SDK_ROOT}/components/rpmsg/fsl_adapter_rpmsg.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/multicore/mcmgr/src/mcmgr.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/multicore/rpmsg_lite/lib/rpmsg_lite/rpmsg_lite.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/multicore/rpmsg_lite/lib/rpmsg_lite/porting/environment/rpmsg_env_bm.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/multicore/rpmsg_lite/lib/rpmsg_lite/porting/platform/mcxw72x/rpmsg_platform.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/multicore/rpmsg_lite/lib/rpmsg_lite/porting/platform/mcxw72x/rpmsg_platform_ext.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/multicore/rpmsg_lite/lib/common/llist.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/multicore/rpmsg_lite/lib/virtio/virtqueue.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/multicore/mcmgr/src/mcmgr_imu_internal.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/multicore/mcmgr/src/mcmgr_internal_core_api_mcxw727.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/multicore/mcmgr/src/mcmgr_internal_core_api_mcxw727_ext.c
    )

    if (NOT OT_ZB_SUPPORT)
        list(APPEND MCXW72_SDK_SOURCES
            $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/SerialDevice/ASP.c
            $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/SerialDevice/Phy.c
        )
    endif()

else()
     list(APPEND MCXW72_SDK_SOURCES
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv/dbg_ram_capture.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv/mathfp.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv/rfmc_ctrl.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv/nb2p4ghz/nxp2p4_xcvr.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv/nb2p4ghz/nxp_xcvr_trim.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv/nb2p4ghz/configs/gen47/nxp_xcvr_coding_config.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv/nb2p4ghz/configs/gen47/nxp_xcvr_common_config.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv/nb2p4ghz/configs/gen47/nxp_xcvr_mode_config.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv/nb2p4ghz/configs/gen47/nxp_xcvr_msk_config.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv/nb2p4ghz/configs/gen47/nxp_xcvr_oqpsk_802p15p4_config.c
       $ENV{NXP_MCXW72_SDK_ROOT}/devices/MCXW727C/gcc/startup_MCXW727C_cm33_core0.S
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/PhyPacket.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/ASP.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/MPM.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/PhyISR.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/PhyPacketProcessor.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/PhyPlmeData.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/PhyStateMachine.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/PhyTime.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/PhySec.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/K32W148/PhyPlatform.c
       $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/connected_mcu/fwk_platform_genfsk.c
       )
endif()

if (SDK_RELEASE)
    list(APPEND MCXW72_SDK_SOURCES
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_k4_flash.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_k4_controller.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_clock.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_gpio.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_lpuart.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_lpspi.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_ltc.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_lptmr.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_elemu.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_spc.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_cmc.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_ccm32k.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_crc.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_wuu.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_edma.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/fsl_lpuart_edma.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${UTL_DIR}/fsl_assert.c
)
else()
    list(APPEND MCXW72_SDK_SOURCES
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/utils/dbg_io.c
        $ENV{NXP_MCXW72_SDK_ROOT}/devices/MCXW727C/drivers/fsl_clock.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/flash_k4/fsl_k4_flash.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/flash_k4/fsl_k4_controller.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/gpio/fsl_gpio.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/lpuart/fsl_lpuart.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/ltc/fsl_ltc.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/lptmr/fsl_lptmr.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/lpspi/fsl_lpspi.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/elemu/fsl_elemu.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/spc/fsl_spc.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/cmc/fsl_cmc.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/ccm32k/fsl_ccm32k.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/crc/fsl_crc.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/wuu/fsl_wuu.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/dma3/fsl_edma.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/lpuart/fsl_lpuart_edma.c
        $ENV{NXP_MCXW72_SDK_ROOT}/components/assert/fsl_assert.c
        $ENV{NXP_MCXW72_SDK_ROOT}/components/format/fsl_format.c
)
endif()

list(APPEND MCXW72_SDK_SOURCES
    $ENV{NXP_MCXW72_SDK_ROOT}/components/debug_console/fsl_debug_console.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/str/fsl_str.c
    $ENV{NXP_MCXW72_SDK_ROOT}/components/shell/fsl_shell.c
)

add_library(${NXP_DRIVER_LIB}
    ${MCXW72_SDK_SOURCES}
)

if (OT_RCP_FLOW_CONTROL)
    target_compile_definitions(${NXP_DRIVER_LIB}
        PUBLIC
             -DOT_APP_UART_BAUDRATE=${UART_BAUD_RATE}
             -DSERIAL_MANAGER_RING_BUFFER_SIZE=320
             -DOT_APP_SERIAL_PORT_USE_FC=1
    )
endif()

if (OT_RCP_UART_DMA)
    target_compile_definitions(${NXP_DRIVER_LIB}
        PUBLIC
             -DSERIAL_PORT_TYPE_UART_DMA=1
             -DOT_APP_UART_BAUDRATE=${UART_BAUD_RATE}
             -DSERIAL_MANAGER_RING_BUFFER_SIZE=320
             -DHAL_UART_DMA_ENABLE=1
             -DOT_APP_UART_TYPE=kSerialPort_UartDma
             -DOT_APP_SERIAL_PORT_USE_DMA=1
             -DLPUART_RING_BUFFER_SIZE=1024
    )
endif()

target_compile_definitions(${NXP_DRIVER_LIB}
    PUBLIC
        ${OT_PLATFORM_DEFINES}
        -DOT_APP_UART_INSTANCE=${UART_INSTANCE}
        -DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING=1
)

target_compile_options(${NXP_DRIVER_LIB}
    PRIVATE
        ${OT_CFLAGS}
        -Wno-unknown-pragmas
        -Wno-sign-compare
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-empty-body
        -Wno-missing-field-initializers
        -Wno-clobbered
        -fno-strict-aliasing
)

set(MCXW72_SDK_INCLUDES
    ${PROJECT_SOURCE_DIR}/openthread/examples/platforms
    ${PROJECT_SOURCE_DIR}/third_party/mcxw72_sdk
    $ENV{NXP_MCXW72_SDK_ROOT}/devices/MCXW727C
    $ENV{NXP_MCXW72_SDK_ROOT}/CMSIS/Core/Include
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/mbedtls/port/sssapi
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/secure-subsystem/inc
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/secure-subsystem/inc/elemu
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/secure-subsystem/port/kw45_k4w1
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/Common
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/FunctionLib
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/HWParameter
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/SecLib
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/RNG
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/Sensors
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/NVM/Source
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/NVM/Interface
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/boards/kw47_mcxw72x
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/boards/kw47_mcxw72x/KW47B42ZB7
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/include
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/connected_mcu
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/connected_mcu/configs
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/utils/
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv/nb2p4ghz
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv/nb2p4ghz/configs/gen47
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/interface
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/multicore/rpmsg_lite/lib/include
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/multicore/rpmsg_lite/lib/include/platform/mcxw72x
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/multicore/rpmsg_lite/lib/include/environment/bm
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/multicore/mcmgr/src
    $ENV{NXP_MCXW72_SDK_ROOT}/components/osa
    $ENV{NXP_MCXW72_SDK_ROOT}/components/lists
    $ENV{NXP_MCXW72_SDK_ROOT}/components/messaging
    $ENV{NXP_MCXW72_SDK_ROOT}/components/mem_manager
    $ENV{NXP_MCXW72_SDK_ROOT}/components/panic
    $ENV{NXP_MCXW72_SDK_ROOT}/components/serial_manager
    $ENV{NXP_MCXW72_SDK_ROOT}/components/uart
    $ENV{NXP_MCXW72_SDK_ROOT}/components/gpio
    $ENV{NXP_MCXW72_SDK_ROOT}/components/led
    $ENV{NXP_MCXW72_SDK_ROOT}/components/button
    $ENV{NXP_MCXW72_SDK_ROOT}/components/timer_manager
    $ENV{NXP_MCXW72_SDK_ROOT}/components/time_stamp
    $ENV{NXP_MCXW72_SDK_ROOT}/components/timer
    $ENV{NXP_MCXW72_SDK_ROOT}/components/rpmsg
    $ENV{NXP_MCXW72_SDK_ROOT}/components/internal_flash
    $ENV{NXP_MCXW72_SDK_ROOT}/components/flash/nor/lpspi
    $ENV{NXP_MCXW72_SDK_ROOT}/components/flash/nor
    $ENV{NXP_MCXW72_SDK_ROOT}/components/power_manager/devices/KW47B42ZB7
    $ENV{NXP_MCXW72_SDK_ROOT}/components/power_manager/core
    $ENV{NXP_MCXW72_SDK_ROOT}/components/reset
)

if (SDK_RELEASE)
    list(APPEND MCXW72_SDK_INCLUDES
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}
        $ENV{NXP_MCXW72_SDK_ROOT}/${UTL_DIR}
        $ENV{NXP_MCXW72_SDK_ROOT}/${UTL_DIR}/format
)
else()
    list(APPEND MCXW72_SDK_INCLUDES
        $ENV{NXP_MCXW72_SDK_ROOT}/devices/KW47B42ZB7/drivers

        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/common
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/flash_k4
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/gpio
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/lpuart
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/ltc
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/port
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/lptmr
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/lpspi
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/imu
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/wuu
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/dma3
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/elemu
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/spc
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/lpspi
        $ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/ccm32k
        $ENV{NXP_MCXW72_SDK_ROOT}/components/format
        ${BLE_DIR}/boards/kw47evk/wireless_examples
)
endif()

list(APPEND MCXW72_SDK_INCLUDES
    $ENV{NXP_MCXW72_SDK_ROOT}/components/debug_console
    $ENV{NXP_MCXW72_SDK_ROOT}/components/str
    $ENV{NXP_MCXW72_SDK_ROOT}/components/shell
    $ENV{NXP_MCXW72_SDK_ROOT}/components/assert
)

target_include_directories(${NXP_DRIVER_LIB}
    PUBLIC
    ${MCXW72_SDK_INCLUDES}
)

if (OT_BLE_SUPPORT)
    add_subdirectory(ble)
    target_link_libraries(${NXP_DRIVER_LIB} PUBLIC nxp-mcxw72-ble)
endif()

target_link_libraries(${NXP_DRIVER_LIB} PRIVATE ot-config)
