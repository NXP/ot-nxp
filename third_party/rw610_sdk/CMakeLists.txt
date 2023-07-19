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

set_property(SOURCE $ENV{NXP_RW610_SDK_ROOT}/devices/RW610/gcc/startup_RW610.S PROPERTY LANGUAGE C)

if (SDK_RELEASE)
    message("SDK_RELEASE driver path chosen")
    set(BOARD_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/boards/rdrw610/project_template)
    set(DRIVER_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/devices/RW610/drivers)
    set(DRIVER_CACHE_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_FLEXCOMM_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_FLEXSPI_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_LPC_GPIO_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_IMU_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_GDMA_FILES_PATH ${DRIVER_FILES_PATH})
    #set(DRIVER_TRNG_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_COMMON_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_DEVICE_FILES_PATH ${DRIVER_FILES_PATH})
    set(DEBUG_CONSOLE_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/devices/RW610/utilities/debug_console)
    set(STR_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/devices/RW610/utilities/str)
else ()
    message("SDK_DEV driver path chosen")
    set(BOARD_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/boards/rdrw610)
    set(DRIVER_CACHE_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/platform/drivers/cache/cache64)
    set(DRIVER_FLEXCOMM_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/platform/drivers/flexcomm)
    set(DRIVER_FLEXSPI_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/platform/drivers/flexspi)
    set(DRIVER_LPC_GPIO_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/platform/drivers/lpc_gpio)
    set(DRIVER_IMU_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/platform/drivers/imu)
    set(DRIVER_GDMA_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/platform/drivers/gdma)
    #set(DRIVER_TRNG_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/platform/drivers/trng)
    set(DRIVER_COMMON_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/platform/drivers/common)
    set(DRIVER_DEVICE_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/devices/RW610/drivers)
    set(DEBUG_CONSOLE_FILES_PATH $ENV{NXP_RW610_SDK_ROOT}/platform/utilities/debug_console)
    set(STR_FILES_PATH ${DEBUG_CONSOLE_FILES_PATH}/str)
endif ()

set(RW610_SDK_SOURCES
#freertos files
    $ENV{NXP_RW610_SDK_ROOT}/rtos/freertos/freertos_kernel/event_groups.c
    $ENV{NXP_RW610_SDK_ROOT}/rtos/freertos/freertos_kernel/croutine.c
    $ENV{NXP_RW610_SDK_ROOT}/rtos/freertos/freertos_kernel/list.c
    $ENV{NXP_RW610_SDK_ROOT}/rtos/freertos/freertos_kernel/portable/GCC/ARM_CM33_NTZ/non_secure/port.c
    $ENV{NXP_RW610_SDK_ROOT}/rtos/freertos/freertos_kernel/portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c
    $ENV{NXP_RW610_SDK_ROOT}/rtos/freertos/freertos_kernel/queue.c
    $ENV{NXP_RW610_SDK_ROOT}/rtos/freertos/freertos_kernel/stream_buffer.c
    $ENV{NXP_RW610_SDK_ROOT}/rtos/freertos/freertos_kernel/tasks.c
    $ENV{NXP_RW610_SDK_ROOT}/rtos/freertos/freertos_kernel/timers.c
    #$ENV{NXP_RW610_SDK_ROOT}/rtos/freertos/freertos_kernel/portable/MemMang/heap_4.c
    $ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/framework/Common/rtos/freertos/heap_mem_manager.c
#startup files
    $ENV{NXP_RW610_SDK_ROOT}/devices/RW610/gcc/startup_RW610.S
    $ENV{NXP_RW610_SDK_ROOT}/devices/RW610/system_RW610.c
#board files
    ${BOARD_FILES_PATH}/board.c
    ${BOARD_FILES_PATH}/clock_config.c
#other SDK files
    ${DRIVER_COMMON_FILES_PATH}/fsl_common.c
    ${DRIVER_DEVICE_FILES_PATH}/fsl_clock.c
    ${DRIVER_DEVICE_FILES_PATH}/fsl_power.c
    ${DRIVER_DEVICE_FILES_PATH}/fsl_reset.c
    ${DRIVER_DEVICE_FILES_PATH}/fsl_ocotp.c
    ${DRIVER_CACHE_FILES_PATH}/fsl_cache.c
    ${DRIVER_FLEXCOMM_FILES_PATH}/fsl_flexcomm.c
    ${DRIVER_FLEXCOMM_FILES_PATH}/fsl_usart.c
    ${DRIVER_FLEXSPI_FILES_PATH}/fsl_flexspi.c
    ${DRIVER_LPC_GPIO_FILES_PATH}/fsl_gpio.c
    ${DRIVER_IMU_FILES_PATH}/fsl_imu.c
    ${DRIVER_GDMA_FILES_PATH}/fsl_gdma.c
    #${DRIVER_TRNG_FILES_PATH}/fsl_trng.c
    ${DEBUG_CONSOLE_FILES_PATH}/fsl_debug_console.c
    ${STR_FILES_PATH}/fsl_str.c
#SDK components
    $ENV{NXP_RW610_SDK_ROOT}/components/serial_manager/fsl_component_serial_manager.c
    $ENV{NXP_RW610_SDK_ROOT}/components/mem_manager/fsl_component_mem_manager_light.c
    $ENV{NXP_RW610_SDK_ROOT}/components/messaging/fsl_component_messaging.c
    $ENV{NXP_RW610_SDK_ROOT}/components/serial_manager/fsl_component_serial_port_uart.c
    $ENV{NXP_RW610_SDK_ROOT}/components/lists/fsl_component_generic_list.c
    $ENV{NXP_RW610_SDK_ROOT}/components/osa/fsl_os_abstraction_free_rtos.c
    $ENV{NXP_RW610_SDK_ROOT}/components/uart/fsl_adapter_usart.c
    $ENV{NXP_RW610_SDK_ROOT}/components/rpmsg/fsl_adapter_rfimu.c
    $ENV{NXP_RW610_SDK_ROOT}/components/rng/fsl_adapter_software_rng.c
    $ENV{NXP_RW610_SDK_ROOT}/components/internal_flash/fsl_adapter_flexspi_nor_flash.c
    #$ENV{NXP_RW610_SDK_ROOT}/components/rng/fsl_adapter_trng.c
#Framework files
    $ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/framework/FunctionLib/FunctionLib.c
    #$ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/framework/SecLib/SecLib.c
    #$ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/framework/RNG/RNG.c
    $ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/framework/NVM/Source/NV_Flash.c
#Loader files
    $ENV{NXP_RW610_SDK_ROOT}/components/conn_fwloader/fsl_loader.c
    $ENV{NXP_RW610_SDK_ROOT}/components/conn_fwloader/life_cycle.c
    $ENV{NXP_RW610_SDK_ROOT}/components/conn_fwloader/nboot_hal.c
#802.15.4 files
    $ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/RW610N/RW610_RPMSG/Phy.c
)

if (SDK_RELEASE)
    list(APPEND RW610_SDK_SOURCES
        $ENV{NXP_RW610_SDK_ROOT}/devices/RW610/utilities/format/fsl_format.c
	$ENV{NXP_RW610_SDK_ROOT}/devices/RW610/drivers/fsl_common_arm.c
)
else()
    list(APPEND RW610_SDK_SOURCES
        $ENV{NXP_RW610_SDK_ROOT}/platform/utilities/misc_utilities/fsl_format.c
	$ENV{NXP_RW610_SDK_ROOT}/platform/drivers/common/fsl_common_arm.c
)
endif()

add_library(nxp-rw610-driver
    ${RW610_SDK_SOURCES}
)

set(RW610_SDK_INCLUDES
#freertos includes
        $ENV{NXP_RW610_SDK_ROOT}/rtos/freertos/freertos_kernel/include
        $ENV{NXP_RW610_SDK_ROOT}/rtos/freertos/freertos_kernel/portable/GCC/ARM_CM33_NTZ/non_secure
        $ENV{NXP_RW610_SDK_ROOT}/middleware/wifi/incl/port/os
        $ENV{NXP_RW610_SDK_ROOT}/middleware/wifi/incl
#boards include path location
        ${BOARD_FILES_PATH}
#CMSIS includes
        $ENV{NXP_RW610_SDK_ROOT}/CMSIS/Core/Include
#other SDK files
        $ENV{NXP_RW610_SDK_ROOT}/devices/RW610/drivers
        $ENV{NXP_RW610_SDK_ROOT}/devices/RW610
        ${DRIVER_COMMON_FILES_PATH}
        ${DRIVER_DEVICE_FILES_PATH}
        ${DRIVER_FLEXSPI_FILES_PATH}
        ${DEBUG_CONSOLE_FILES_PATH}
        ${DRIVER_CACHE_FILES_PATH}
        ${DRIVER_FLEXCOMM_FILES_PATH}
        ${DRIVER_LPC_GPIO_FILES_PATH}
        ${DRIVER_IMU_FILES_PATH}
        ${DRIVER_GDMA_FILES_PATH}
        ${DRIVER_TRNG_FILES_PATH}
        ${STR_FILES_PATH}
        $ENV{NXP_RW610_SDK_ROOT}/platform/utilities/misc_utilities
        $ENV{NXP_RW610_SDK_ROOT}/platform/drivers/common
#SDK components
        $ENV{NXP_RW610_SDK_ROOT}/components/uart
        $ENV{NXP_RW610_SDK_ROOT}/components/serial_manager
        $ENV{NXP_RW610_SDK_ROOT}/components/mem_manager
        $ENV{NXP_RW610_SDK_ROOT}/components/messaging
        $ENV{NXP_RW610_SDK_ROOT}/components/lists
        $ENV{NXP_RW610_SDK_ROOT}/components/osa
        $ENV{NXP_RW610_SDK_ROOT}/components/common_task
        $ENV{NXP_RW610_SDK_ROOT}/components/rpmsg
        $ENV{NXP_RW610_SDK_ROOT}/components/rng
        $ENV{NXP_RW610_SDK_ROOT}/components/internal_flash
#Framework files
        $ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/framework/FunctionLib
        $ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/framework/Common
        $ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/framework/SecLib
        $ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/framework/RNG
        $ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/framework/NVM/Interface
        $ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/framework/NVM/Source
#Loader files
        $ENV{NXP_RW610_SDK_ROOT}/components/conn_fwloader/include
#802.15.4 files
        $ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/ieee-802.15.4/ieee_802_15_4/phy/interface
        $ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/ieee-802.15.4/ieee_802_15_4/phy/source/RW610N/RW610_RPMSG
        $ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/ieee-802.15.4/utils/
)

if (SDK_RELEASE)
    list(APPEND RW610_SDK_INCLUDES
        $ENV{NXP_RW610_SDK_ROOT}/devices/RW610/utilities/format
)
else()
    list(APPEND RW610_SDK_INCLUDES
        $ENV{NXP_RW610_SDK_ROOT}/platform/utilities/misc_utilities
)
endif()

#freertos
target_include_directories(nxp-rw610-driver
    PUBLIC
	${RW610_SDK_INCLUDES}
)

target_compile_definitions(nxp-rw610-driver
    PUBLIC
    -DCPU_RW610EVA0IK
    -DCPU3
    -D__STARTUP_CLEAR_BSS
    -DSDK_OS_FREE_RTOS
    -DOSA_USED
    -DFSL_OSA_MAIN_FUNC_ENABLE=0
    -DFSL_RTOS_FREE_RTOS
    -DSDK_DEBUGCONSOLE_UART=1
    -DSERIAL_PORT_TYPE_UART=1
    -DSERIAL_MANAGER_NON_BLOCKING_MODE=1
    -DSERIAL_MANAGER_TASK_STACK_SIZE=1024
    -DSERIAL_MANAGER_TASK_PRIORITY=3
    -DIMU_TASK_STACK_SIZE=1024
    -DRPMSG_TXQ_BUFSIZE=16
    -DRPMSG_TXQ_BUFLENGTH=256
    -DOT_MAIN_TASK_SIZE=640 # in WORDS !
    -DgMemManagerLightExtendHeapAreaUsage=1
    -DgNvStorageIncluded_d=1
    -DAPP_FLEXSPI_AMBA_BASE=0x08000000
    -DOT_PLAT_SAVE_NVM_DATA_ON_IDLE=0
)

target_link_libraries(nxp-rw610-driver
    PRIVATE
        openthread-rw610
        $ENV{NXP_RW610_SDK_ROOT}/middleware/wireless/framework/SecLib/lib_crypto_m33.a
)

target_compile_options(nxp-rw610-driver
    PRIVATE
        ${OT_CFLAGS}
        -Wno-unknown-pragmas
        -Wno-sign-compare
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-empty-body
)
