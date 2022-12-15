#
#  Copyright (c) 2021-2022, The OpenThread Authors.
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

get_filename_component(NXP_RT1060_SDK_ROOT_ABS "$ENV{NXP_RT1060_SDK_ROOT}" ABSOLUTE)

set(EVK_RT1060_BOARD "evkbmimxrt1060" CACHE STRING "evkbmimxrt1060")

# Required to set MIDDLEWARE_PATH so that it is visible in current scope
# and in parent scope
set(MIDDLEWARE_PATH ${NXP_RT1060_SDK_ROOT_ABS}/middleware)
set(MIDDLEWARE_PATH ${MIDDLEWARE_PATH} PARENT_SCOPE)

set(RTOS_PATH ${NXP_RT1060_SDK_ROOT_ABS}/rtos)

message(STATUS " absolut path  ${NXP_RT1060_SDK_ROOT_ABS}")
if (${SDK_TYPE} STREQUAL "GITHUB")
    message("SDK_GITHUB driver path chosen")
    set(NXP_RT1060_SDK_ROOT_ABS ${NXP_RT1060_SDK_ROOT_ABS}/core)
    set(DRIVER_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/drivers)
    set(DRIVER_LPUART_FILES_PATH ${DRIVER_FILES_PATH}/lpuart)
    set(DRIVER_EDMA_FILES_PATH ${DRIVER_FILES_PATH}/edma)
    set(DRIVER_DMAMUX_FILES_PATH ${DRIVER_FILES_PATH}/dmamux)
    set(DRIVER_PIT_FILES_PATH ${DRIVER_FILES_PATH}/pit)
    set(DRIVER_TRNG_FILES_PATH ${DRIVER_FILES_PATH}/trng)
    set(DRIVER_FLEXSPI_FILES_PATH ${DRIVER_FILES_PATH}/flexspi)
    set(DRIVER_IGPIO_FILES_PATH ${DRIVER_FILES_PATH}/igpio)
    set(DRIVER_DCP_FILES_PATH ${DRIVER_FILES_PATH}/dcp)
    set(DRIVER_FSL_CACHE_FILES_PATH ${DRIVER_FILES_PATH}/cache/armv7-m7)
    set(DRIVER_COMMON_FILES_PATH ${DRIVER_FILES_PATH}/common)
    set(DRIVER_DEVICE_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1062/drivers)
    set(UTILITIES_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/utilities)
    set(UTILITIES_DEVICE_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1062/utilities)
    set(DEBUG_CONSOLE_FILES_PATH ${UTILITIES_FILES_PATH}/debug_console_lite)
    set(STR_FILES_PATH ${UTILITIES_FILES_PATH}/debug_console/str)
    set(FSL_FLEXSPI_NOR_BOOT_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1062/xip)
elseif (${SDK_TYPE} STREQUAL "PACKAGE")
    message("SDK_PACKAGE driver path chosen")
    set(DRIVER_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1062/drivers)
    set(DRIVER_LPUART_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_PIT_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_TRNG_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_FLEXSPI_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_IGPIO_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_DCP_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_FSL_CACHE_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_COMMON_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_DEVICE_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_EDMA_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_DMAMUX_FILES_PATH ${DRIVER_FILES_PATH})
    set(DEBUG_CONSOLE_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1062/utilities/debug_console)
    set(STR_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1062/utilities/str)
    set(FSL_FLEXSPI_NOR_BOOT_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1062/xip)
else ()
    message("SDK_DEV driver path chosen")
    set(DRIVER_LPUART_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/platform/drivers/lpuart)
    set(DRIVER_PIT_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/platform/drivers/pit)
    set(DRIVER_TRNG_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/platform/drivers/trng)
    set(DRIVER_FLEXSPI_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/platform/drivers/flexspi)
    set(DRIVER_IGPIO_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/platform/drivers/igpio)
    set(DRIVER_DCP_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/platform/drivers/dcp)
    set(DRIVER_FSL_CACHE_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/platform/drivers/cache/armv7-m7)
    set(DRIVER_COMMON_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/platform/drivers/common)
    set(DRIVER_DEVICE_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1062/drivers)
    set(DRIVER_EDMA_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/platform/drivers/edma)
    set(DRIVER_DMAMUX_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/platform/drivers/dmamux)
    set(DEBUG_CONSOLE_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/platform/utilities/debug_console)
    set(STR_FILES_PATH ${DEBUG_CONSOLE_FILES_PATH}/str)
    set(FSL_FLEXSPI_NOR_BOOT_FILES_PATH ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1052/xip)
endif ()

set_property(SOURCE ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1062/gcc/startup_MIMXRT1062.S PROPERTY LANGUAGE C)

add_library(${NXP_DRIVER_LIB}
#freertos files
    ${RTOS_PATH}/freertos/freertos_kernel/event_groups.c
    ${RTOS_PATH}/freertos/freertos_kernel/croutine.c
    ${RTOS_PATH}/freertos/freertos_kernel/list.c
    ${RTOS_PATH}/freertos/freertos_kernel/portable/GCC/ARM_CM4F/port.c
    ${RTOS_PATH}/freertos/freertos_kernel/queue.c
    ${RTOS_PATH}/freertos/freertos_kernel/stream_buffer.c
    ${RTOS_PATH}/freertos/freertos_kernel/tasks.c
    ${RTOS_PATH}/freertos/freertos_kernel/timers.c
    ${RTOS_PATH}/freertos/freertos_kernel/portable/MemMang/heap_4.c
#startup files
    ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1062/gcc/startup_MIMXRT1062.S
    #${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1062/mcuxpresso/startup_mimxrt1062.c
    ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1062/system_MIMXRT1062.c
#other SDK files
    ${DRIVER_LPUART_FILES_PATH}/fsl_lpuart.c
    ${DRIVER_LPUART_FILES_PATH}/fsl_lpuart_edma.c
    ${DRIVER_IGPIO_FILES_PATH}/fsl_gpio.c
    ${DRIVER_PIT_FILES_PATH}/fsl_pit.c
    ${DRIVER_TRNG_FILES_PATH}/fsl_trng.c
    ${DRIVER_FLEXSPI_FILES_PATH}/fsl_flexspi.c
    ${DRIVER_COMMON_FILES_PATH}/fsl_common_arm.c
    ${DRIVER_DEVICE_FILES_PATH}/fsl_clock.c
    ${DRIVER_DCP_FILES_PATH}/fsl_dcp.c
    ${DRIVER_FSL_CACHE_FILES_PATH}/fsl_cache.c
    ${DRIVER_EDMA_FILES_PATH}/fsl_edma.c
    ${DRIVER_DMAMUX_FILES_PATH}/fsl_dmamux.c
    ${DEBUG_CONSOLE_FILES_PATH}/fsl_debug_console.c
    ${STR_FILES_PATH}/fsl_str.c
# XIP
    ${FSL_FLEXSPI_NOR_BOOT_FILES_PATH}/fsl_flexspi_nor_boot.c
    ${NXP_RT1060_SDK_ROOT_ABS}/boards/${EVK_RT1060_BOARD}/xip/${EVK_RT1060_BOARD}_flexspi_nor_config.c
#SDK components
    ${NXP_RT1060_SDK_ROOT_ABS}/components/serial_manager/fsl_component_serial_manager.c
    ${NXP_RT1060_SDK_ROOT_ABS}/components/serial_manager/fsl_component_serial_port_uart.c
    ${NXP_RT1060_SDK_ROOT_ABS}/components/uart/fsl_adapter_lpuart.c
    ${NXP_RT1060_SDK_ROOT_ABS}/components/lists/fsl_component_generic_list.c
    ${NXP_RT1060_SDK_ROOT_ABS}/components/osa/fsl_os_abstraction_free_rtos.c
    ${NXP_RT1060_SDK_ROOT_ABS}/components/log/fsl_component_log.c
    ${NXP_RT1060_SDK_ROOT_ABS}/components/log/fsl_component_log_backend_debugconsole.c
    ${NXP_RT1060_SDK_ROOT_ABS}/components/timer_manager/fsl_component_timer_manager.c
    ${NXP_RT1060_SDK_ROOT_ABS}/components/timer/fsl_adapter_pit.c
    ${NXP_RT1060_SDK_ROOT_ABS}/components/rng/fsl_adapter_trng.c
    ${NXP_RT1060_SDK_ROOT_ABS}/components/internal_flash/fsl_adapter_flexspi_nor_flash.c
    ${NXP_RT1060_SDK_ROOT_ABS}/components/gpio/fsl_adapter_igpio.c
#Framework files
    ${MIDDLEWARE_PATH}/wireless/framework/FunctionLib/FunctionLib.c
    ${MIDDLEWARE_PATH}/wireless/framework/platform/rt/fwk_platform_ot.c
    ${MIDDLEWARE_PATH}/wireless/framework/platform/rt/fwk_platform_hdlc.c
    ${MIDDLEWARE_PATH}/wireless/framework/platform/rt/k32w0/fwk_platform_coex.c
#File system
    ${MIDDLEWARE_PATH}/littlefs/lfs.c
    ${MIDDLEWARE_PATH}/littlefs/lfs_util.c
    ${NXP_RT1060_SDK_ROOT_ABS}/components/flash/mflash/mimxrt1062/mflash_drv.c
    ${MIDDLEWARE_PATH}/wireless/framework/platform/rt1060/fwk_platform_flash.c
    ${MIDDLEWARE_PATH}/wireless/framework/FileSystem/fwk_filesystem.c
)


target_include_directories(${NXP_DRIVER_LIB}
    PUBLIC
#freertos includes
        ${RTOS_PATH}/freertos/freertos_kernel/include
        ${RTOS_PATH}/freertos/freertos_kernel/portable/GCC/ARM_CM4F
#CMSIS includes
        ${NXP_RT1060_SDK_ROOT_ABS}/CMSIS/Core/Include
#other SDK files
        ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1062/drivers
        ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1062
        ${DRIVER_COMMON_FILES_PATH}
        ${DRIVER_LPUART_FILES_PATH}
        ${DRIVER_IGPIO_FILES_PATH}
        ${DRIVER_DCP_FILES_PATH}
        ${DRIVER_FSL_CACHE_FILES_PATH}
        ${DRIVER_PIT_FILES_PATH}
        ${DRIVER_TRNG_FILES_PATH}
        ${DRIVER_FLEXSPI_FILES_PATH}
        ${DRIVER_EDMA_FILES_PATH}
        ${DRIVER_DMAMUX_FILES_PATH}
        ${DEBUG_CONSOLE_FILES_PATH}
        ${UTILITIES_FILES_PATH}/debug_console/debug_console
        ${STR_FILES_PATH}
# XIP
        ${NXP_RT1060_SDK_ROOT_ABS}/devices/MIMXRT1052/xip
        ${NXP_RT1060_SDK_ROOT_ABS}/boards/${EVK_RT1060_BOARD}/xip
#SDK components
        ${NXP_RT1060_SDK_ROOT_ABS}/components/uart
        ${NXP_RT1060_SDK_ROOT_ABS}/components/serial_manager
        ${NXP_RT1060_SDK_ROOT_ABS}/components/lists
        ${NXP_RT1060_SDK_ROOT_ABS}/components/osa
        ${NXP_RT1060_SDK_ROOT_ABS}/components/common_task
        ${NXP_RT1060_SDK_ROOT_ABS}/components/log
        ${NXP_RT1060_SDK_ROOT_ABS}/components/timer_manager
        ${NXP_RT1060_SDK_ROOT_ABS}/components/timer
        ${NXP_RT1060_SDK_ROOT_ABS}/components/rng
        ${NXP_RT1060_SDK_ROOT_ABS}/components/internal_flash
        ${NXP_RT1060_SDK_ROOT_ABS}/components/gpio
#Framework files
        ${MIDDLEWARE_PATH}/wireless/framework/FunctionLib
        ${MIDDLEWARE_PATH}/wireless/framework/Common
        ${MIDDLEWARE_PATH}/wireless/framework/platform/rt1060
        ${MIDDLEWARE_PATH}/wireless/framework/platform/include
        ${MIDDLEWARE_PATH}/wireless/framework/FileSystem
#File system
        ${MIDDLEWARE_PATH}/littlefs
        ${NXP_RT1060_SDK_ROOT_ABS}/components/flash/mflash
        ${NXP_RT1060_SDK_ROOT_ABS}/components/flash/mflash/mimxrt1062
)

target_compile_definitions(${NXP_DRIVER_LIB}
    PUBLIC
    -D__STARTUP_CLEAR_BSS
    -D__STARTUP_INITIALIZE_NONCACHEDATA
    #-DSERIAL_MANAGER_NON_BLOCKING_MODE=1
    -DSERIAL_MANAGER_TASK_HANDLE_RX_AVAILABLE_NOTIFY=1
    -DSDK_OS_FREE_RTOS
    -DUSE_RTOS
    -DOSA_USED
    -DFSL_OSA_MAIN_FUNC_ENABLE=0
    -DFSL_RTOS_FREE_RTOS
    -DXIP_EXTERNAL_FLASH=1
    -DXIP_BOOT_HEADER_ENABLE=1
    -DCPU_MIMXRT1062DVL6A
    -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1
    -DSDK_COMPONENT_INTEGRATION=1
    -DFSL_DRIVER_TRANSFER_DOUBLE_WEAK_IRQ=0
    -DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING
    -DHAL_UART_ADAPTER_FIFO=1
    -DSDK_DEBUGCONSOLE_UART=1
    -DSERIAL_PORT_TYPE_UART=1
    -DCONFIG_ARM=1
    -DSERIAL_MANAGER_RING_BUFFER_FLOWCONTROL=1
    -DSERIAL_MANAGER_TASK_STACK_SIZE=4048
    -DFWK_FILESYSTEM_LOCAL_BUF_SIZE=1024
    -DLFS_THREADSAFE=1
    -DLFS_NO_DEBUG
    -DLFS_NO_ERROR
    -DLFS_NO_WARN
    -DSPINEL_UART_INSTANCE=3
    -DSPINEL_ENABLE_RX_RTS=1
    -DSPINEL_ENABLE_TX_RTS=1
    -DRESET_PIN_PORT=6
    -DRESET_PIN_NUM=2
    #DMA support enablement
    #-DHAL_UART_DMA_ENABLE=1
    #-DSERIAL_PORT_TYPE_UART_DMA=1
    #Async logging feature
    #-DLOG_ENABLE_ASYNC_MODE=1
    #-DLOG_MAX_ARGUMENT_COUNT=17
    #-DLOG_MAX_BUFF_LOG_COUNT=250
    #-DLOG_ENABLE_OVERWRITE=1
)

if (OT_STACK_ENABLE_LOG)
#move logs to UART2
target_compile_definitions(${NXP_DRIVER_LIB}
    PUBLIC
    BOARD_DEBUG_UART_BASEADDR=LPUART2
    BOARD_DEBUG_UART_INSTANCE=2U
    BOARD_UART_IRQ=LPUART2_IRQn
    BOARD_UART_IRQ_HANDLER=LPUART2_IRQHandler
)
endif ()

target_link_libraries(${NXP_DRIVER_LIB}
    PRIVATE
        ${OT_PLATFORM_LIB}
)

target_compile_options(${NXP_DRIVER_LIB}
    PRIVATE
        ${OT_CFLAGS}
        -Wno-unknown-pragmas
        -Wno-sign-compare
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-empty-body
)

set (OT_NXP_MBEDTLS_CONFIG_FILE "\"ot-nxp-mbedtls-config.h\"" PARENT_SCOPE)
set (OT_NXP_MBEDTLS_PORT ksdk PARENT_SCOPE)
