#
# Copyright (c) 2024, The OpenThread Authors.
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
get_filename_component(OT_NXP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../ REALPATH)

target_compile_definitions(${NXP_DRIVER_LIB} PUBLIC
    -DCPU_RW612ETA1I
    -DCPU_RW612ETA2I
    -DCPU3
    -D__STARTUP_CLEAR_BSS
    -DSDK_OS_FREE_RTOS
    -DOSA_USED
    -DFSL_OSA_MAIN_FUNC_ENABLE=0
    -DFSL_RTOS_FREE_RTOS
    -DSDK_DEBUGCONSOLE_UART=1
    -DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING
    -DSERIAL_PORT_TYPE_UART=1
    -DHAL_UART_ADAPTER_LOWPOWER=1
    -DgPlatformMonolithicApp_d=1
    -DCONFIG_SOC_SERIES_RW6XX_REVISION_A2=1
    -DCONFIG_MONOLITHIC_BLE_15_4=1
    -DCONFIG_MONOLITHIC_WIFI=1
    -DBLE_FW_ADDRESS=0
    -DRW610
    -DWIFI_BT_TX_PWR_LIMITS="wlan_txpwrlimit_cfg_WW_rw610.h"

    # -DSERIAL_MANAGER_NON_BLOCKING_MODE=1
    -DSERIAL_MANAGER_TASK_STACK_SIZE=1024
    -DSERIAL_MANAGER_TASK_PRIORITY=PRIORITY_RTOS_TO_OSA\(2\)
    -DTM_ENABLE_TIME_STAMP=1
    -DIMU_TASK_STACK_SIZE=1024
    -DgNvStorageIncluded_d=1
    -DAPP_FLEXSPI_AMBA_BASE=0x08000000
    -DOT_PLAT_SAVE_NVM_DATA_ON_IDLE=1
    -DFSL_OSA_ALLOCATED_HEAP=0
    -DgAspCapability_d=1
    -DFFU_CNS_TX_PWR_TABLE_CALIBRATION=1
    -DIMU_TASK_PRIORITY=2 # OSA priority: configMAX_PRIORITIES - (osa_prio) - 2 = FreeRTOS priority
    -DgPlatformDisableBleLowPower_d=0 # Enable BLE/15.4 low power feature
    -DNOT_DEFINE_DEFAULT_WIFI_MODULE
    -DWIFI_BOARD_RW610
)

mcux_add_configuration(
    CC "${OT_CFLAGS}"
    CC "-Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-empty-body -Wno-int-conversion -Wno-int-in-bool-context -Wno-memset-elt-size -Wno-parentheses"
)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES
    # There are weak functions in PWR.c, the redefined ones in PWR_Systicks.c, but after
    # building library and linked with other libraries, the weak functions will be used
    # first, this seems to be related to the order of compiling files, as a temporary
    # solution, disable 'CONFIG_MCUX_COMPONENT_middleware.wireless.framework.lowpower'
    # in prj.conf file and adjust the compilation order of files here.
    middleware/wireless/framework/services/LowPower/PWR_systicks.c
    middleware/wireless/framework/services/LowPower/PWR.c
)

if(OT_NXP_BUILD_APP_AS_LIB)
    # ethermind/coex_app_cli include
    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}
	INCLUDES
        middleware/wireless/wpa_supplicant-rtos/port/mbedtls
    )

    mcux_add_include(
        BASE_PATH ${OT_NXP_ROOT}
        INCLUDES
        boards/${OT_NXP_PLATFORM}
        boards/${OT_NXP_PLATFORM}/freertos/br
        src/${OT_NXP_PLATFORM_FAMILY}/${OT_NXP_PLATFORM}
        src/common/lwip
        third_party/lwip
        third_party/wifi
    )
endif()

# This file will be added to wifi library and ensure tcpip_init is called only once in it.
if(OT_NXP_LWIP_WIFI)
    mcux_project_remove_source(
        BASE_PATH ${SdkRootDirPath}
        SOURCES
        middleware/wifi_nxp/port/net/net.c
    )
endif()

if(OT_NXP_LWIP_WIFI OR OT_NXP_LWIP_ETH)
mcux_add_macro(
    -DOT_APP_BR_LWIP_HOOKS_EN
)
mcux_add_include(
     BASE_PATH ${OT_NXP_ROOT}
     INCLUDES
     boards/${OT_NXP_PLATFORM}
     boards/${OT_NXP_PLATFORM}/freertos/br
     src/${OT_NXP_PLATFORM_FAMILY}/${OT_NXP_PLATFORM}
     src/common/br
     src/common/lwip
     third_party/lwip
     third_party/wifi
)
else()
mcux_add_include(
     BASE_PATH ${OT_NXP_ROOT}
     INCLUDES
     boards/${OT_NXP_PLATFORM}
     boards/${OT_NXP_PLATFORM}/freertos
     src/${OT_NXP_PLATFORM_FAMILY}/${OT_NXP_PLATFORM}
)
endif()

if(OT_NXP_LWIP_IPERF)
    mcux_add_include(
        BASE_PATH ${OT_NXP_ROOT}
        INCLUDES
        src/common/lwip
    )
endif()

if(OT_NXP_LWIP_WIFI)
target_link_libraries(${NXP_DRIVER_LIB}
    PUBLIC
    nxp-lwip
    wifi
)
endif()

if(OT_NXP_LWIP_ETH)
mcux_add_macro(
    -DFSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE
    -DLWIP_DISABLE_PBUF_POOL_SIZE_SANITY_CHECKS
    -DUSE_RTOS
)
endif()

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES
    middleware/wireless/framework/services/LowPower
)

if(OT_NXP_LWIP_WIFI)
mcux_add_include(
     BASE_PATH ${SdkRootDirPath}
     INCLUDES
     components/wifi_bt_module/template
)
endif()

if(OT_NXP_DEVICE_REVISION MATCHES "A0")
    mcux_add_macro(
        -DRW610_A1=0
    )
endif()

if(OT_NXP_ENABLE_WPA_SUPP_MBEDTLS)
    mcux_add_macro(
        -DCONFIG_WPA_SUPP_MBEDTLS # Enable wpa_supplicant mbedtls extend config
    )
endif()

if (OT_NXP_BOARD_NAME MATCHES "rw612_frdm")
    mcux_add_macro(
        -DgBoardUseFro32k_d=1
    )
endif()

if(OT_NCP_RADIO)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/examples/ncp_examples/common
        SOURCES
        crc/crc.c
        ncp_glue_common.c
        ncp_adapter/ncp_intf/ncp_intf_pm.c
        ncp_adapter/ncp_tlv/ncp_tlv_adapter.c
        ncp_adapter/ncp_intf/uart/ncp_intf_uart.c
        ncp_adapter/ncp_intf/usb/usb_device_cdc/ncp_intf_usb_device_cdc.c
        ncp_adapter/ncp_intf/usb/usb_device_cdc/usb_device_cdc_app.c
        ncp_adapter/ncp_intf/usb/usb_device_cdc/usb_device_descriptor.c
        ncp_adapter/ncp_intf/spi/spi_slave/ncp_intf_spi_slave.c
        ncp_adapter/ncp_intf/sdio/sdio_device/ncp_intf_sdio.c
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/examples/ncp_examples/common
        INCLUDES
        .
        crc
        lpm
        ncp_debug
        ncp_adapter
        ncp_adapter/ncp_intf
        ncp_adapter/ncp_tlv
        ncp_adapter/ncp_intf/uart
        ncp_adapter/ncp_intf/usb/usb_device_cdc
        ncp_adapter/ncp_intf/spi/spi_slave
        ncp_adapter/ncp_intf/sdio/sdio_device
    )

    mcux_add_macro(
        -DCONFIG_CRC32_HW_ACCELERATE
        -DCONFIG_NCP=1
        -DCONFIG_NCP_OT=1
        -DNCP_UART_TASK_PRIORITY=3
        -DUSB_DEVICE_CONFIG_LOW_POWER_MODE=1
        -DCONFIG_HOST_SLEEP
        -DRW610
    )

    if(NOT DEFINED OT_NXP_NCP_UART_INTERFACE
       AND NOT DEFINED OT_NXP_NCP_USB_INTERFACE
       AND NOT DEFINED OT_NXP_NCP_SPI_INTERFACE
       AND NOT DEFINED OT_NXP_NCP_SDIO_INTERFACE)
        set(OT_NXP_NCP_UART_INTERFACE "ON") # Use UART as default NCP adapter interface
    endif()

    if(OT_NXP_NCP_UART_INTERFACE)
        mcux_add_macro(
            -DCONFIG_NCP_UART
        )
    elseif(OT_NXP_NCP_USB_INTERFACE)
        mcux_add_macro(
            -DCONFIG_NCP_USB
        )
    elseif(OT_NXP_NCP_SPI_INTERFACE)
        mcux_add_macro(
            -DCONFIG_NCP_SPI
        )
    elseif(OT_NXP_NCP_SDIO_INTERFACE)
        mcux_add_macro(
		-DCONFIG_NCP_SDIO
        )
    else()
        message(FATAL_ERROR "Please select one NCP interface.")
    endif()
endif()

# Remove the default link script, rw612 will use a custom one instead.
mcux_remove_armgcc_linker_script(
    BASE_PATH ${SdkRootDirPath}
    LINKER devices/Wireless/RW/RW612/gcc/RW612_ram.ld
)
