#
#  Copyright (c) 2021-2023, The OpenThread Authors.
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

set(MIDDLEWARE_PATH ${NXP_SDK_ROOT}/middleware)

set(LWIP_PATH $ENV{NXP_RT1170_SDK_ROOT}/middleware/lwip PARENT_SCOPE)
set(WIFI_PATH $ENV{NXP_RT1170_SDK_ROOT}/middleware/wifi_nxp CACHE PATH "wifi path")
set(WIFI_BT_MODULE_PATH $ENV{NXP_RT1170_SDK_ROOT}/components/wifi_bt_module  CACHE PATH "wifi_bt_module path")
set(EDGEFAST_WIFI_PATH $ENV{NXP_RT1170_SDK_ROOT}/middleware/edgefast_wifi CACHE PATH "edgefast_wifi path")

set(CONNFWK_BOARD "rt1170" PARENT_SCOPE)
set(CONNFWK_SDK_ROOT ${NXP_SDK_ROOT} PARENT_SCOPE)

set(EVK_MIMXRT1170 "evkbmimxrt1170" CACHE STRING "evkbmimxrt1170")
set(MIMXRT1170_EVK "mimxrt1170evkb" CACHE STRING "mimxrt1170evkb")

if (${SDK_TYPE} STREQUAL "GITHUB")
    message(FATAL_ERROR "GITHUB SDK is not supported for this platform")
elseif (${SDK_TYPE} STREQUAL "PACKAGE")
    set(DRIVER_FILES_PATH ${NXP_SDK_ROOT}/devices/MIMXRT1176/drivers)
    set(DRIVER_LPUART_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_LPSPI_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_PIT_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_GPT_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_FLEXSPI_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_IGPIO_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_COMMON_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_DEVICE_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_EDMA_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_DMAMUX_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_ENET_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_USDHC_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_SDHC_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_CACHE_FILES_PATH ${DRIVER_FILES_PATH}/cm7)
    set(DRIVER_CAAM_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_MU_FILES_PATH ${DRIVER_FILES_PATH})
    set(DEBUG_CONSOLE_FILES_PATH ${NXP_SDK_ROOT}/devices/MIMXRT1176/utilities/debug_console)
    set(ASSERT_FILES_PATH ${NXP_SDK_ROOT}/devices/MIMXRT1176/utilities)
    set(STR_FILES_PATH ${NXP_SDK_ROOT}/devices/MIMXRT1176/utilities/str)
    set(FSL_FLEXSPI_NOR_BOOT_FILES_PATH ${NXP_SDK_ROOT}/devices/MIMXRT1176/xip)
    set(INCBIN_FILES_PATH ${NXP_SDK_ROOT}/devices/MIMXRT1176/utilities/incbin)
elseif (${SDK_TYPE} STREQUAL "INTERNAL")
    set(DRIVER_LPUART_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/lpuart)
    set(DRIVER_LPSPI_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/lpspi)
    set(DRIVER_PIT_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/pit)
    set(DRIVER_GPT_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/gpt)
    set(DRIVER_FLEXSPI_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/flexspi)
    set(DRIVER_IGPIO_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/igpio)
    set(DRIVER_COMMON_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/common)
    set(DRIVER_DEVICE_FILES_PATH ${NXP_SDK_ROOT}/devices/MIMXRT1176/drivers)
    set(DRIVER_EDMA_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/edma)
    set(DRIVER_DMAMUX_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/dmamux)
    set(DRIVER_ENET_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/enet)
    set(DRIVER_USDHC_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/usdhc)
    set(DRIVER_SDHC_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/sdhc)
    set(DRIVER_CACHE_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/cache/armv7-m7)
    set(DRIVER_CAAM_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/caam)
    set(DRIVER_MU_FILES_PATH ${NXP_SDK_ROOT}/platform/drivers/mu)
    set(DEBUG_CONSOLE_FILES_PATH ${NXP_SDK_ROOT}/platform/utilities/debug_console)
    set(ASSERT_FILES_PATH ${NXP_SDK_ROOT}/platform/utilities/assert)
    set(STR_FILES_PATH ${NXP_SDK_ROOT}/platform/utilities/str)
    set(FSL_FLEXSPI_NOR_BOOT_FILES_PATH ${NXP_SDK_ROOT}/devices/MIMXRT1176/xip)
    set(INCBIN_FILES_PATH ${NXP_SDK_ROOT}/platform/utilities/misc_utilities)
else()
    message(FATAL_ERROR "Unknown SDK_TYPE")
endif ()

list(APPEND SDK_INCLUDES
#CMSIS includes
    ${NXP_SDK_ROOT}/CMSIS/Core/Include
#other SDK files
    ${NXP_SDK_ROOT}/devices/MIMXRT1176
    ${DRIVER_DEVICE_FILES_PATH}
    ${DRIVER_DEVICE_FILES_PATH}/cm7
    ${DRIVER_COMMON_FILES_PATH}
    ${DRIVER_LPUART_FILES_PATH}
    ${DRIVER_LPSPI_FILES_PATH}
    ${DRIVER_IGPIO_FILES_PATH}
    ${DRIVER_PIT_FILES_PATH}
    ${DRIVER_GPT_FILES_PATH}
    ${DRIVER_FLEXSPI_FILES_PATH}
    ${DRIVER_EDMA_FILES_PATH}
    ${DRIVER_CACHE_FILES_PATH}
    ${DRIVER_CAAM_FILES_PATH}
    ${DRIVER_MU_FILES_PATH}
    ${DRIVER_DMAMUX_FILES_PATH}
    ${DEBUG_CONSOLE_FILES_PATH}
    ${STR_FILES_PATH}
# XIP
    ${NXP_SDK_ROOT}/devices/MIMXRT1176/xip
    ${NXP_SDK_ROOT}/boards/${EVK_MIMXRT1170}/xip
#SDK components
    ${NXP_SDK_ROOT}/components/uart
    ${NXP_SDK_ROOT}/components/serial_manager
    ${NXP_SDK_ROOT}/components/lists
    ${NXP_SDK_ROOT}/components/osa
    ${NXP_SDK_ROOT}/components/common_task
    ${NXP_SDK_ROOT}/components/log
    ${NXP_SDK_ROOT}/components/timer_manager
    ${NXP_SDK_ROOT}/components/timer
    ${NXP_SDK_ROOT}/components/internal_flash
    ${NXP_SDK_ROOT}/components/spi
    ${NXP_SDK_ROOT}/components/gpio
    ${NXP_SDK_ROOT}/components/time_stamp
#File system
    ${NXP_SDK_ROOT}/middleware/littlefs
    ${NXP_SDK_ROOT}/components/flash/mflash
    $ENV{NXP_RT1170_SDK_ROOT}/components/flash/mflash/mimxrt1170
)

list(APPEND SDK_SOURCES
#startup files
    ${NXP_SDK_ROOT}/devices/MIMXRT1176/gcc/startup_MIMXRT1176_cm7.S
    #${NXP_SDK_ROOT}/devices/MIMXRT1176/mcuxpresso/startup_mimxrt1176_cm7.c
    ${NXP_SDK_ROOT}/devices/MIMXRT1176/system_MIMXRT1176_cm7.c
#other SDK files
    ${DRIVER_LPUART_FILES_PATH}/fsl_lpuart.c
    ${DRIVER_LPSPI_FILES_PATH}/fsl_lpspi.c
    ${DRIVER_LPUART_FILES_PATH}/fsl_lpuart_edma.c
    ${DRIVER_IGPIO_FILES_PATH}/fsl_gpio.c
    ${DRIVER_PIT_FILES_PATH}/fsl_pit.c
    ${DRIVER_GPT_FILES_PATH}/fsl_gpt.c
    ${DRIVER_FLEXSPI_FILES_PATH}/fsl_flexspi.c
    ${DRIVER_COMMON_FILES_PATH}/fsl_common_arm.c
    ${DRIVER_DEVICE_FILES_PATH}/fsl_clock.c
    ${DRIVER_DEVICE_FILES_PATH}/fsl_pmu.c
    ${DRIVER_DEVICE_FILES_PATH}/fsl_anatop_ai.c
    ${DRIVER_DEVICE_FILES_PATH}/fsl_soc_src.c
    ${DRIVER_DEVICE_FILES_PATH}/fsl_dcdc.c
    ${DRIVER_EDMA_FILES_PATH}/fsl_edma.c
    ${DRIVER_DMAMUX_FILES_PATH}/fsl_dmamux.c
    ${DRIVER_CACHE_FILES_PATH}/fsl_cache.c
    ${DRIVER_CAAM_FILES_PATH}/fsl_caam.c
    ${DRIVER_MU_FILES_PATH}/fsl_mu.c
    ${DEBUG_CONSOLE_FILES_PATH}/fsl_debug_console.c
    ${STR_FILES_PATH}/fsl_str.c
    ${ASSERT_FILES_PATH}/fsl_assert.c
# XIP
    ${FSL_FLEXSPI_NOR_BOOT_FILES_PATH}/fsl_flexspi_nor_boot.c
    ${NXP_SDK_ROOT}/boards/${EVK_MIMXRT1170}/xip/${EVK_MIMXRT1170}_flexspi_nor_config.c
#SDK components
    ${NXP_SDK_ROOT}/components/serial_manager/fsl_component_serial_manager.c
    ${NXP_SDK_ROOT}/components/serial_manager/fsl_component_serial_port_uart.c
    ${NXP_SDK_ROOT}/components/spi/fsl_adapter_lpspi.c
    ${NXP_SDK_ROOT}/components/uart/fsl_adapter_lpuart.c
    ${NXP_SDK_ROOT}/components/lists/fsl_component_generic_list.c
    ${NXP_SDK_ROOT}/components/osa/fsl_os_abstraction_free_rtos.c
    ${NXP_SDK_ROOT}/components/log/fsl_component_log.c
    ${NXP_SDK_ROOT}/components/log/fsl_component_log_backend_debugconsole.c
    ${NXP_SDK_ROOT}/components/timer_manager/fsl_component_timer_manager.c
    ${NXP_SDK_ROOT}/components/time_stamp/fsl_adapter_gpt_time_stamp.c
    ${NXP_SDK_ROOT}/components/timer/fsl_adapter_pit.c
    ${NXP_SDK_ROOT}/components/internal_flash/${EVK_MIMXRT1170}/fsl_adapter_flexspi_nor_flash.c
    ${NXP_SDK_ROOT}/components/gpio/fsl_adapter_igpio.c
#File system
    ${NXP_SDK_ROOT}/middleware/littlefs/lfs.c
    ${NXP_SDK_ROOT}/middleware/littlefs/lfs_util.c
    ${NXP_SDK_ROOT}/components/flash/mflash/${MIMXRT1170_EVK}/mflash_drv.c
)

list(APPEND SDK_COMPILE_DEFINITIONS
    -D__STARTUP_CLEAR_BSS
    -D__STARTUP_INITIALIZE_NONCACHEDATA
    #-DSERIAL_MANAGER_NON_BLOCKING_MODE=1
    #-DSERIAL_MANAGER_TASK_HANDLE_RX_AVAILABLE_NOTIFY=1
    -DSDK_OS_FREE_RTOS
    -DUSE_RTOS
    -DOSA_USED
    -DFSL_OSA_MAIN_FUNC_ENABLE=0
    -DFSL_RTOS_FREE_RTOS
    -DXIP_EXTERNAL_FLASH=1
    -DXIP_BOOT_HEADER_ENABLE=1
    -DCPU_MIMXRT1176DVMAA
    -DCPU_MIMXRT1176DVMAA_cm7
    -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1
    -DSDK_COMPONENT_INTEGRATION=1
    -DFSL_DRIVER_TRANSFER_DOUBLE_WEAK_IRQ=0
    -DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING
    -DHAL_UART_ADAPTER_FIFO=1
    -DSDK_DEBUGCONSOLE_UART=1
    -DDEBUG_CONSOLE_RX_ENABLE=0
    -DSERIAL_PORT_TYPE_UART=1
    -DCONFIG_ARM=1
    -DSERIAL_MANAGER_TASK_STACK_SIZE=4048
    #LittleFS configs
    LFS_CONFIG=fwk_lfs_config.h
    LFS_THREADSAFE=1
    #Flash size should be increased for RT1170
    -DFLASH_ADAPTER_SIZE=0x4000
    #Security flags
    -DCRYPTO_USE_DRIVER_CAAM
    -DCACHE_MODE_WRITE_THROUGH=1
)

if ("${OT_NXP_TRANSCEIVER}" STREQUAL "iwx12")
    list(APPEND SDK_INCLUDES
            ${DRIVER_USDHC_FILES_PATH}
            ${DRIVER_SDHC_FILES_PATH}
            ${NXP_SDK_ROOT}/middleware/wifi_nxp/wifidriver
            ${NXP_SDK_ROOT}/middleware/wifi_nxp/wifi_bt_firmware
            ${NXP_SDK_ROOT}/middleware/sdmmc/common
            ${NXP_SDK_ROOT}/middleware/sdmmc/sdio
            ${NXP_SDK_ROOT}/middleware/sdmmc/host/usdhc
            ${NXP_SDK_ROOT}/middleware/sdmmc/osa
            ${NXP_SDK_ROOT}/middleware/wifi_nxp/incl/wifidriver
            ${NXP_SDK_ROOT}/middleware/wifi_nxp/incl
            ${NXP_SDK_ROOT}/middleware/wifi_nxp/incl/wlcmgr
            ${NXP_SDK_ROOT}/middleware/wifi_nxp/incl/port/os
            ${NXP_SDK_ROOT}/middleware/wifi_nxp/wifidriver
            ${NXP_SDK_ROOT}/middleware/wifi_nxp/wifidriver/incl
            ${NXP_SDK_ROOT}/components/wifi_bt_module/template
    )
    list(APPEND SDK_SOURCES
            ${NXP_SDK_ROOT}/middleware/sdmmc/common/fsl_sdmmc_common.c
            ${NXP_SDK_ROOT}/middleware/sdmmc/sdio/fsl_sdio.c
            ${NXP_SDK_ROOT}/middleware/sdmmc/host/usdhc/non_blocking/fsl_sdmmc_host.c
            ${NXP_SDK_ROOT}/middleware/sdmmc/osa/fsl_sdmmc_osa.c
            ${NXP_SDK_ROOT}/middleware/wifi_nxp/wifidriver/firmware_dnld.c
            ${NXP_SDK_ROOT}/middleware/wifi_nxp/wifidriver/mlan_sdio.c
            ${NXP_SDK_ROOT}/middleware/wifi_nxp/wifidriver/sdio.c
            ${DRIVER_USDHC_FILES_PATH}/fsl_usdhc.c
            ${NXP_SDK_ROOT}/components/wifi_bt_module/template/wifi_bt_config.c
    )
    list(APPEND SDK_COMPILE_DEFINITIONS
        -DSD9177
        -DSDMMCHOST_OPERATION_VOLTAGE_3V3
        -DCONFIG_UAP_AMPDU_TX=1
        -DCONFIG_UAP_AMPDU_RX=1
    )
elseif ("${OT_NXP_TRANSCEIVER}" STREQUAL "k32w0")
    list(APPEND SDK_COMPILE_DEFINITIONS
        -DHAL_UART_DMA_ENABLE=1 # OTW requires the DMA to be enabled
    )
endif ()


if (OT_NXP_LWIP_ETH)
    list(APPEND SDK_INCLUDES
        ${NXP_SDK_ROOT}/components/phy/
        ${NXP_SDK_ROOT}/components/phy/device/phyrtl8211f
        ${NXP_SDK_ROOT}/components/silicon_id
        ${NXP_SDK_ROOT}/components/silicon_id/socs
        ${DRIVER_ENET_FILES_PATH}
        ${DRIVER_DEVICE_FILES_PATH}
        )
    list(APPEND SDK_SOURCES
        ${NXP_SDK_ROOT}/components/phy/device/phyrtl8211f/fsl_phyrtl8211f.c
        ${NXP_SDK_ROOT}/components/silicon_id/fsl_silicon_id.c
        ${NXP_SDK_ROOT}/components/silicon_id/socs/imxrt1170/fsl_silicon_id_soc.c
        ${DRIVER_ENET_FILES_PATH}/fsl_enet.c
        ${DRIVER_DEVICE_FILES_PATH}/fsl_clock.c
        )
    list(APPEND SDK_COMPILE_DEFINITIONS
        )
endif ()

if (MULTICORE_LOGGING)
    list(APPEND SDK_INCLUDES
        ${NXP_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/include
        ${NXP_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/include/environment/freertos
        ${NXP_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/include/platform/imxrt1170
        ${NXP_SDK_ROOT}/middleware/multicore/mcmgr/src
    )
    list(APPEND SDK_SOURCES
        ${NXP_SDK_ROOT}/middleware/multicore/mcmgr/src/mcmgr_mu_internal.c
        ${NXP_SDK_ROOT}/middleware/multicore/mcmgr/src/mcmgr.c
        ${NXP_SDK_ROOT}/middleware/multicore/mcmgr/src/mcmgr_internal_core_api_imxrt1170.c
        ${NXP_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/common/llist.c
        ${NXP_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/rpmsg_lite/rpmsg_lite.c
        ${NXP_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/rpmsg_lite/rpmsg_ns.c
        ${NXP_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/virtio/virtqueue.c
        ${NXP_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/rpmsg_lite/porting/environment/rpmsg_env_freertos.c
        ${NXP_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/rpmsg_lite/rpmsg_queue.c
        ${NXP_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/rpmsg_lite/porting/platform/imxrt1170/rpmsg_platform.c
        ${INCBIN_FILES_PATH}/fsl_incbin.S
    )
endif ()

add_library(${NXP_DRIVER_LIB}
    ${SDK_SOURCES}
)

target_include_directories(${NXP_DRIVER_LIB}
    PUBLIC
    ${SDK_INCLUDES}
)

target_compile_definitions(${NXP_DRIVER_LIB}
    PUBLIC
    ${SDK_COMPILE_DEFINITIONS}
)

target_link_libraries(${NXP_DRIVER_LIB}
    PUBLIC
    freertos_kernel
    ${NXP_BOARD_LIB}
    #Dependency to little_fs config files
    connfwk-config
    connfwk-platform-${OT_NXP_PLATFORM_FAMILY}
    connfwk-platform-${OT_NXP_PLATFORM}
)

target_compile_options(${NXP_DRIVER_LIB}
    PRIVATE
        ${OT_CFLAGS}
        -Wno-unknown-pragmas
        -Wno-sign-compare
        -Wno-unused-parameter
        -Wno-empty-body
        -Wno-unused-function
)

set (OT_NXP_MBEDTLS_CONFIG_FILE "\"ot-nxp-mbedtls-config.h\"" PARENT_SCOPE)
set (OT_NXP_MBEDTLS_PORT ksdk PARENT_SCOPE)
