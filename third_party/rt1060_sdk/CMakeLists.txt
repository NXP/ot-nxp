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

set(MIDDLEWARE_PATH ${NXP_RT1060_SDK_ROOT_ABS}/middleware)

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

add_library(nxp-rt1060-driver
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
#File system
    ${MIDDLEWARE_PATH}/littlefs/lfs.c
    ${MIDDLEWARE_PATH}/littlefs/lfs_util.c
    ${NXP_RT1060_SDK_ROOT_ABS}/components/flash/mflash/mimxrt1062/mflash_drv.c
    ${MIDDLEWARE_PATH}/wireless/framework/platform/rt1060/fwk_platform_flash.c
    ${MIDDLEWARE_PATH}/wireless/framework/FileSystem/fwk_filesystem.c
)


target_include_directories(nxp-rt1060-driver 
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
        ${MIDDLEWARE_PATH}/wireless/framework/FileSystem
#File system
        ${MIDDLEWARE_PATH}/littlefs
        ${NXP_RT1060_SDK_ROOT_ABS}/components/flash/mflash
        ${NXP_RT1060_SDK_ROOT_ABS}/components/flash/mflash/mimxrt1062
)

target_compile_definitions(nxp-rt1060-driver
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
target_compile_definitions(nxp-rt1060-driver
    PUBLIC
    BOARD_DEBUG_UART_BASEADDR=LPUART2
    BOARD_DEBUG_UART_INSTANCE=2U
    BOARD_UART_IRQ=LPUART2_IRQn
    BOARD_UART_IRQ_HANDLER=LPUART2_IRQHandler
)
endif ()

target_link_libraries(nxp-rt1060-driver
    PRIVATE
        openthread-rt1060
)

target_compile_options(nxp-rt1060-driver
    PRIVATE
        ${OT_CFLAGS}
        -Wno-unknown-pragmas
        -Wno-sign-compare
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-empty-body
)

add_library(nxp-rt1060-mbedtls
    ${MIDDLEWARE_PATH}/mbedtls/library/aes.c
    ${MIDDLEWARE_PATH}/mbedtls/library/aesni.c
    ${MIDDLEWARE_PATH}/mbedtls/library/arc4.c
    ${MIDDLEWARE_PATH}/mbedtls/library/aria.c
    ${MIDDLEWARE_PATH}/mbedtls/library/asn1parse.c
    ${MIDDLEWARE_PATH}/mbedtls/library/asn1write.c
    ${MIDDLEWARE_PATH}/mbedtls/library/base64.c
    ${MIDDLEWARE_PATH}/mbedtls/library/bignum.c
    ${MIDDLEWARE_PATH}/mbedtls/library/blowfish.c
    ${MIDDLEWARE_PATH}/mbedtls/library/camellia.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ccm.c
    ${MIDDLEWARE_PATH}/mbedtls/library/certs.c
    ${MIDDLEWARE_PATH}/mbedtls/library/cipher.c
    ${MIDDLEWARE_PATH}/mbedtls/library/cipher_wrap.c
    ${MIDDLEWARE_PATH}/mbedtls/library/chacha20.c
    ${MIDDLEWARE_PATH}/mbedtls/library/chachapoly.c
    ${MIDDLEWARE_PATH}/mbedtls/library/cmac.c
    ${MIDDLEWARE_PATH}/mbedtls/library/constant_time.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ctr_drbg.c
    ${MIDDLEWARE_PATH}/mbedtls/library/debug.c
    ${MIDDLEWARE_PATH}/mbedtls/library/des.c
    ${MIDDLEWARE_PATH}/mbedtls/library/dhm.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ecdh.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ecdsa.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ecjpake.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ecp.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ecp_curves.c
    ${MIDDLEWARE_PATH}/mbedtls/library/entropy.c
    ${MIDDLEWARE_PATH}/mbedtls/library/entropy_poll.c
    ${MIDDLEWARE_PATH}/mbedtls/library/error.c
    ${MIDDLEWARE_PATH}/mbedtls/library/gcm.c
    ${MIDDLEWARE_PATH}/mbedtls/library/havege.c
    ${MIDDLEWARE_PATH}/mbedtls/library/hkdf.c
    ${MIDDLEWARE_PATH}/mbedtls/library/hmac_drbg.c
    ${MIDDLEWARE_PATH}/mbedtls/library/md.c
    ${MIDDLEWARE_PATH}/mbedtls/library/md2.c
    ${MIDDLEWARE_PATH}/mbedtls/library/md4.c
    ${MIDDLEWARE_PATH}/mbedtls/library/md5.c
    ${MIDDLEWARE_PATH}/mbedtls/library/memory_buffer_alloc.c
    ${MIDDLEWARE_PATH}/mbedtls/library/net_sockets.c
    ${MIDDLEWARE_PATH}/mbedtls/library/nist_kw.c
    ${MIDDLEWARE_PATH}/mbedtls/library/oid.c
    ${MIDDLEWARE_PATH}/mbedtls/library/padlock.c
    ${MIDDLEWARE_PATH}/mbedtls/library/pem.c
    ${MIDDLEWARE_PATH}/mbedtls/library/pk.c
    ${MIDDLEWARE_PATH}/mbedtls/library/pk_wrap.c
    ${MIDDLEWARE_PATH}/mbedtls/library/pkcs5.c
    ${MIDDLEWARE_PATH}/mbedtls/library/pkcs11.c
    ${MIDDLEWARE_PATH}/mbedtls/library/pkcs12.c
    ${MIDDLEWARE_PATH}/mbedtls/library/pkparse.c
    ${MIDDLEWARE_PATH}/mbedtls/library/pkwrite.c
    ${MIDDLEWARE_PATH}/mbedtls/library/platform.c
    ${MIDDLEWARE_PATH}/mbedtls/library/platform_util.c
    ${MIDDLEWARE_PATH}/mbedtls/library/poly1305.c
    ${MIDDLEWARE_PATH}/mbedtls/library/psa_crypto.c
    ${MIDDLEWARE_PATH}/mbedtls/library/psa_crypto_client.c
    ${MIDDLEWARE_PATH}/mbedtls/library/psa_crypto_driver_wrappers.c
    ${MIDDLEWARE_PATH}/mbedtls/library/psa_crypto_ecp.c
    ${MIDDLEWARE_PATH}/mbedtls/library/psa_crypto_rsa.c
    ${MIDDLEWARE_PATH}/mbedtls/library/psa_crypto_se.c
    ${MIDDLEWARE_PATH}/mbedtls/library/psa_crypto_slot_management.c
    ${MIDDLEWARE_PATH}/mbedtls/library/psa_crypto_storage.c
    ${MIDDLEWARE_PATH}/mbedtls/library/psa_its_file.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ripemd160.c
    ${MIDDLEWARE_PATH}/mbedtls/library/rsa.c
    ${MIDDLEWARE_PATH}/mbedtls/library/rsa_internal.c
    ${MIDDLEWARE_PATH}/mbedtls/library/sha1.c
    ${MIDDLEWARE_PATH}/mbedtls/library/sha256.c
    ${MIDDLEWARE_PATH}/mbedtls/library/sha512.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ssl_cache.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ssl_ciphersuites.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ssl_cli.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ssl_cookie.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ssl_msg.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ssl_srv.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ssl_ticket.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ssl_tls.c
    ${MIDDLEWARE_PATH}/mbedtls/library/ssl_tls13_keys.c
    ${MIDDLEWARE_PATH}/mbedtls/library/threading.c
    ${MIDDLEWARE_PATH}/mbedtls/library/timing.c
    ${MIDDLEWARE_PATH}/mbedtls/library/version.c
    ${MIDDLEWARE_PATH}/mbedtls/library/version_features.c
    ${MIDDLEWARE_PATH}/mbedtls/library/x509.c
    ${MIDDLEWARE_PATH}/mbedtls/library/x509_create.c
    ${MIDDLEWARE_PATH}/mbedtls/library/x509_crl.c
    ${MIDDLEWARE_PATH}/mbedtls/library/x509_crt.c
    ${MIDDLEWARE_PATH}/mbedtls/library/x509_csr.c
    ${MIDDLEWARE_PATH}/mbedtls/library/x509write_crt.c
    ${MIDDLEWARE_PATH}/mbedtls/library/x509write_csr.c
    ${MIDDLEWARE_PATH}/mbedtls/library/xtea.c
    ${MIDDLEWARE_PATH}/mbedtls/port/ksdk/ksdk_mbedtls.c
    ${MIDDLEWARE_PATH}/mbedtls/port/ksdk/des_alt.c
    ${MIDDLEWARE_PATH}/mbedtls/port/ksdk/aes_alt.c
    ${MIDDLEWARE_PATH}/mbedtls/port/ksdk/ecp_alt.c
    ${MIDDLEWARE_PATH}/mbedtls/port/ksdk/ecp_curves_alt.c
    ${MIDDLEWARE_PATH}/mbedtls/port/ksdk/ecp_alt_ksdk.c
)

target_include_directories(nxp-rt1060-mbedtls
    PRIVATE
    ${MIDDLEWARE_PATH}/mbedtls/library
    PUBLIC
    ${MIDDLEWARE_PATH}/mbedtls/include
    ${MIDDLEWARE_PATH}/mbedtls/port/ksdk
    ${PROJECT_SOURCE_DIR}/openthread/third_party/mbedtls
)

target_compile_definitions(nxp-rt1060-mbedtls 
    PUBLIC
    "MBEDTLS_CONFIG_FILE=\"rt-mbedtls-config.h\""
)

target_compile_options(nxp-rt1060-mbedtls
    PRIVATE
        -Wno-unused-function
)

target_link_libraries(nxp-rt1060-mbedtls
    PUBLIC
    nxp-rt1060-driver
    ot-config
)

target_link_libraries(ot-config
    INTERFACE
    nxp-rt1060-mbedtls
)
