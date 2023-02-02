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
enable_language(C ASM)

# Required to set MIDDLEWARE_PATH so that it is visible in current scope
# and in parent scope
set(MIDDLEWARE_PATH $ENV{NXP_RT1170_SDK_ROOT}/middleware)
set(MIDDLEWARE_PATH ${MIDDLEWARE_PATH} PARENT_SCOPE)

if (SDK_RELEASE)
    message("SDK_RELEASE driver path chosen")
    set(DRIVER_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/devices/MIMXRT1176/drivers)
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
    set(DRIVER_USDHC_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_CACHE_FILES_PATH ${DRIVER_FILES_PATH}/cm7)
    set(DRIVER_CAAM_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_MU_FILES_PATH ${DRIVER_FILES_PATH})
    set(DEBUG_CONSOLE_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/devices/MIMXRT1176/utilities/debug_console)
    set(ASSERT_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/devices/MIMXRT1062/utilities/assert)
    set(STR_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/devices/MIMXRT1176/utilities/str)
    set(FSL_FLEXSPI_NOR_BOOT_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/devices/MIMXRT1176/xip)
    set(INCBIN_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/devices/MIMXRT1176/utilities/incbin)
else ()
    message("SDK_DEV driver path chosen")
    set(DRIVER_LPUART_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/drivers/lpuart)
    set(DRIVER_LPSPI_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/drivers/lpspi)
    set(DRIVER_PIT_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/drivers/pit)
    set(DRIVER_GPT_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/drivers/gpt)
    set(DRIVER_FLEXSPI_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/drivers/flexspi)
    set(DRIVER_IGPIO_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/drivers/igpio)
    set(DRIVER_COMMON_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/drivers/common)
    set(DRIVER_DEVICE_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/devices/MIMXRT1176/drivers)
    set(DRIVER_EDMA_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/drivers/edma)
    set(DRIVER_DMAMUX_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/drivers/dmamux)
    set(DRIVER_USDHC_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/drivers/usdhc)
    set(DRIVER_CACHE_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/drivers/cache/armv7-m7)
    set(DRIVER_CAAM_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/drivers/caam)
    set(DRIVER_MU_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/drivers/mu)
    set(DEBUG_CONSOLE_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/utilities/debug_console)
    set(ASSERT_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/utilities/assert)
    set(STR_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/utilities/str)
    set(FSL_FLEXSPI_NOR_BOOT_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/devices/MIMXRT1176/xip)
    set(INCBIN_FILES_PATH $ENV{NXP_RT1170_SDK_ROOT}/platform/utilities/misc_utilities)
endif ()

list(APPEND SDK_INCLUDES
    #freertos includes
    $ENV{NXP_RT1170_SDK_ROOT}/rtos/freertos/freertos-kernel/include
    $ENV{NXP_RT1170_SDK_ROOT}/rtos/freertos/freertos-kernel/portable/GCC/ARM_CM4F
#CMSIS includes
    $ENV{NXP_RT1170_SDK_ROOT}/CMSIS/Core/Include
#other SDK files
    $ENV{NXP_RT1170_SDK_ROOT}/devices/MIMXRT1176
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
    $ENV{NXP_RT1170_SDK_ROOT}/devices/MIMXRT1176/xip
    $ENV{NXP_RT1170_SDK_ROOT}/boards/evkmimxrt1170/xip
#SDK components
    $ENV{NXP_RT1170_SDK_ROOT}/components/uart
    $ENV{NXP_RT1170_SDK_ROOT}/components/serial_manager
    $ENV{NXP_RT1170_SDK_ROOT}/components/lists
    $ENV{NXP_RT1170_SDK_ROOT}/components/osa
    $ENV{NXP_RT1170_SDK_ROOT}/components/common_task
    $ENV{NXP_RT1170_SDK_ROOT}/components/log
    $ENV{NXP_RT1170_SDK_ROOT}/components/timer_manager
    $ENV{NXP_RT1170_SDK_ROOT}/components/timer
    $ENV{NXP_RT1170_SDK_ROOT}/components/internal_flash
    $ENV{NXP_RT1170_SDK_ROOT}/components/spi
    $ENV{NXP_RT1170_SDK_ROOT}/components/gpio
    $ENV{NXP_RT1170_SDK_ROOT}/components/time_stamp
#File system
    $ENV{NXP_RT1170_SDK_ROOT}/middleware/littlefs
    $ENV{NXP_RT1170_SDK_ROOT}/components/flash/mflash
    $ENV{NXP_RT1170_SDK_ROOT}/components/flash/mflash/mimxrt1170
)

list(APPEND SDK_SOURCES
    #freertos files
    $ENV{NXP_RT1170_SDK_ROOT}/rtos/freertos/freertos-kernel/event_groups.c
    $ENV{NXP_RT1170_SDK_ROOT}/rtos/freertos/freertos-kernel/croutine.c
    $ENV{NXP_RT1170_SDK_ROOT}/rtos/freertos/freertos-kernel/list.c
    $ENV{NXP_RT1170_SDK_ROOT}/rtos/freertos/freertos-kernel/portable/GCC/ARM_CM4F/port.c
    $ENV{NXP_RT1170_SDK_ROOT}/rtos/freertos/freertos-kernel/queue.c
    $ENV{NXP_RT1170_SDK_ROOT}/rtos/freertos/freertos-kernel/stream_buffer.c
    $ENV{NXP_RT1170_SDK_ROOT}/rtos/freertos/freertos-kernel/tasks.c
    $ENV{NXP_RT1170_SDK_ROOT}/rtos/freertos/freertos-kernel/timers.c
    $ENV{NXP_RT1170_SDK_ROOT}/rtos/freertos/freertos-kernel/portable/MemMang/heap_4.c
#startup files
    $ENV{NXP_RT1170_SDK_ROOT}/devices/MIMXRT1176/gcc/startup_MIMXRT1176_cm7.S
    #$ENV{NXP_RT1170_SDK_ROOT}/devices/MIMXRT1176/mcuxpresso/startup_mimxrt1176_cm7.c
    $ENV{NXP_RT1170_SDK_ROOT}/devices/MIMXRT1176/system_MIMXRT1176_cm7.c
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
    $ENV{NXP_RT1170_SDK_ROOT}/boards/evkmimxrt1170/xip/evkmimxrt1170_flexspi_nor_config.c
#SDK components
    $ENV{NXP_RT1170_SDK_ROOT}/components/serial_manager/fsl_component_serial_manager.c
    $ENV{NXP_RT1170_SDK_ROOT}/components/serial_manager/fsl_component_serial_port_uart.c
    $ENV{NXP_RT1170_SDK_ROOT}/components/spi/fsl_adapter_lpspi.c
    $ENV{NXP_RT1170_SDK_ROOT}/components/uart/fsl_adapter_lpuart.c
    $ENV{NXP_RT1170_SDK_ROOT}/components/lists/fsl_component_generic_list.c
    $ENV{NXP_RT1170_SDK_ROOT}/components/osa/fsl_os_abstraction_free_rtos.c
    $ENV{NXP_RT1170_SDK_ROOT}/components/log/fsl_component_log.c
    $ENV{NXP_RT1170_SDK_ROOT}/components/log/fsl_component_log_backend_debugconsole.c
    $ENV{NXP_RT1170_SDK_ROOT}/components/timer_manager/fsl_component_timer_manager.c
    $ENV{NXP_RT1170_SDK_ROOT}/components/timer/fsl_adapter_pit.c
    $ENV{NXP_RT1170_SDK_ROOT}/components/internal_flash/fsl_adapter_flexspi_nor_flash.c
    $ENV{NXP_RT1170_SDK_ROOT}/components/gpio/fsl_adapter_igpio.c
#File system
    $ENV{NXP_RT1170_SDK_ROOT}/middleware/littlefs/lfs.c
    $ENV{NXP_RT1170_SDK_ROOT}/middleware/littlefs/lfs_util.c
    $ENV{NXP_RT1170_SDK_ROOT}/components/flash/mflash/mimxrt1170/mflash_drv.c
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
    -DUSE_SDK_2_13
)

if ("${OT_NXP_TRANSCEIVER}" STREQUAL "iwx12")
    list(APPEND SDK_INCLUDES
            ${DRIVER_USDHC_FILES_PATH}
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/wifi_nxp/wifidriver
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/wifi_nxp/wifi_bt_firmware
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/sdmmc/common
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/sdmmc/sdio
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/sdmmc/host/usdhc
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/sdmmc/osa
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/wifi_nxp/incl/wifidriver
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/wifi_nxp/incl
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/wifi_nxp/incl/wlcmgr
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/wifi_nxp/incl/port/os
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/wifi_nxp/wifidriver
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/wifi_nxp/wifidriver/incl
            $ENV{NXP_RT1170_SDK_ROOT}/components/wifi_bt_module/template
    )
    list(APPEND SDK_SOURCES
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/sdmmc/common/fsl_sdmmc_common.c
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/sdmmc/sdio/fsl_sdio.c
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/sdmmc/host/usdhc/non_blocking/fsl_sdmmc_host.c
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/sdmmc/osa/fsl_sdmmc_osa.c
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/wifi_nxp/wifidriver/firmware_dnld.c
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/wifi_nxp/wifidriver/mlan_sdio.c
            $ENV{NXP_RT1170_SDK_ROOT}/middleware/wifi_nxp/wifidriver/sdio.c
            ${DRIVER_USDHC_FILES_PATH}/fsl_usdhc.c
            $ENV{NXP_RT1170_SDK_ROOT}/components/wifi_bt_module/template/wifi_bt_config.c
    )
    list(APPEND SDK_COMPILE_DEFINITIONS
        -DIW61x
        -DSDMMCHOST_OPERATION_VOLTAGE_3V3
        -DCONFIG_MAX_AP_ENTRIES=10
        -DCONFIG_UAP_AMPDU_TX=1
        -DCONFIG_UAP_AMPDU_RX=1
        -DCONFIG_WIFI_MAX_PRIO=configMAX_PRIORITIES-1
    )
elseif ("${OT_NXP_TRANSCEIVER}" STREQUAL "k32w0")
    list(APPEND SDK_COMPILE_DEFINITIONS
        -DHAL_UART_DMA_ENABLE=1 # OTW requires the DMA to be enabled
        -DLPUART_RING_BUFFER_SIZE=768 # required for OTW to be able to handle RX frame received on the UART
    )
endif ()

if (MULTICORE_LOGGING)
    list(APPEND SDK_INCLUDES
        $ENV{NXP_RT1170_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/include
        $ENV{NXP_RT1170_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/include/environment/freertos
        $ENV{NXP_RT1170_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/include/platform/imxrt1170
        $ENV{NXP_RT1170_SDK_ROOT}/middleware/multicore/mcmgr/src
    )
    list(APPEND SDK_SOURCES
        $ENV{NXP_RT1170_SDK_ROOT}/middleware/multicore/mcmgr/src/mcmgr_mu_internal.c
        $ENV{NXP_RT1170_SDK_ROOT}/middleware/multicore/mcmgr/src/mcmgr.c
        $ENV{NXP_RT1170_SDK_ROOT}/middleware/multicore/mcmgr/src/mcmgr_internal_core_api_imxrt1170.c
        $ENV{NXP_RT1170_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/common/llist.c
        $ENV{NXP_RT1170_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/rpmsg_lite/rpmsg_lite.c
        $ENV{NXP_RT1170_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/rpmsg_lite/rpmsg_ns.c
        $ENV{NXP_RT1170_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/virtio/virtqueue.c
        $ENV{NXP_RT1170_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/rpmsg_lite/porting/environment/rpmsg_env_freertos.c
        $ENV{NXP_RT1170_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/rpmsg_lite/rpmsg_queue.c
        $ENV{NXP_RT1170_SDK_ROOT}/middleware/multicore/rpmsg_lite/lib/rpmsg_lite/porting/platform/imxrt1170/rpmsg_platform.c
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

# Use Connectivity Framework CMake build system to build required modules
set(CONNFWK_PLATFORM ${OT_NXP_PLATFORM})
set(CONNFWK_PLATFORM_FAMILY ${OT_NXP_PLATFORM_FAMILY})
set(CONNFWK_TRANSCEIVER ${OT_NXP_TRANSCEIVER})
if ("${OT_NXP_TRANSCEIVER}" STREQUAL "k32w0")
    #Define here the default transceiver path can be overwritten by cmake -D option
    set(OT_NXP_TRANSCEIVER_BIN_PATH "${PROJECT_SOURCE_DIR}/build_k32w061/rcp_only_uart_flow_control/bin/ot-rcp.elf.bin.h" CACHE PATH "Path to the transceiver binary file")
    set(CONNFWK_TRANSCEIVER_BIN_PATH ${OT_NXP_TRANSCEIVER_BIN_PATH})
endif()
# Enable FunctionLib and FileSystem modules
set(CONNFWK_FLIB ON)
set(CONNFWK_FILESYSTEM ON)
# Add the Framework as a subdirectory
add_subdirectory(${MIDDLEWARE_PATH}/wireless/framework ${PROJECT_BINARY_DIR}/framework)
# Forward SDK and OT include paths, compile definitions, etc... to the framework
target_link_libraries(connfwk-config
    INTERFACE
        ${NXP_DRIVER_LIB}
        ${NXP_BOARD_LIB}
)
if ("${OT_NXP_TRANSCEIVER}" STREQUAL "k32w0")
    if ("${SPINEL_INTERFACE_TYPE}" STREQUAL "UART")
        target_compile_definitions(connfwk-platform-${CONNFWK_PLATFORM_FAMILY}
            INTERFACE
            #HDLC configuration
            -DSPINEL_UART_INSTANCE=7
            -DSPINEL_ENABLE_RX_RTS=1
            -DSPINEL_ENABLE_TX_RTS=1
            -DSPINEL_UART_CLOCK_RATE=CLOCK_GetRootClockFreq\(kCLOCK_Root_Lpuart7\)
        )
    endif ()
    target_compile_definitions(connfwk-platform-${CONNFWK_PLATFORM_FAMILY}
        INTERFACE
        #OTW configurations
        -DPLATFORM_OTW_RESET_PIN_PORT=3
        -DPLATFORM_OTW_RESET_PIN_NUM=9
        -DPLATFORM_OTW_DIO5_PIN_PORT=6 #TODO identify DIO pin on RT1170
        -DPLATFORM_OTW_DIO5_PIN_NUM=26 #TODO identify DIO pin on RT1170
    )
elseif(${OT_NXP_TRANSCEIVER} STREQUAL "iwx12")
    target_compile_definitions(connfwk-platform-${CONNFWK_PLATFORM_FAMILY}
        INTERFACE
        -DPLATFORM_RESET_PIN_PORT=3
        -DPLATFORM_RESET_PIN_NUM=9
    )
endif()

target_link_libraries(${NXP_DRIVER_LIB}
    PUBLIC
    ${NXP_BOARD_LIB}
    #Dependency to little_fs config files
    connfwk-platform-${OT_NXP_PLATFORM_FAMILY}
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
