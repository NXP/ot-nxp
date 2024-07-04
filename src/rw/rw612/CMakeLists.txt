#
#  Copyright (c) 2021-2023, The OpenThread Authors.
#  Copyright 2024 NXP
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

if(OT_NXP_LWIP)
    list(APPEND PLATFORM_FILES
        ${PROJECT_SOURCE_DIR}/src/common/lwip/ot_lwip.c
        ${PROJECT_SOURCE_DIR}/src/common/lwip/token_bucket.c
    )
    set(OT_LWIP_INCLUDES
        ${PROJECT_SOURCE_DIR}/src/common/lwip
    )
    set(OT_LWIP_LIBS
        nxp-lwip
        openthread-ftd
    )
endif()

if(OT_APP_BR_FREERTOS)
    list(APPEND PLATFORM_FILES
        ${PROJECT_SOURCE_DIR}/src/common/br/infra_if.c
        ${PROJECT_SOURCE_DIR}/src/common/br/udp_plat.c
        ${PROJECT_SOURCE_DIR}/src/common/br/lwip_hooks.c
        ${PROJECT_SOURCE_DIR}/src/common/br/lwip_mcast.c
        ${PROJECT_SOURCE_DIR}/src/common/br/border_agent.c
        ${PROJECT_SOURCE_DIR}/src/common/br/br_rtos_manager.c
        ${PROJECT_SOURCE_DIR}/src/common/br/mdns_socket.c
        ${PROJECT_SOURCE_DIR}/src/common/br/utils.c
        ${PROJECT_SOURCE_DIR}/src/common/br/trel_plat.c
    )

    set(OT_BR_INCLUDES
        ${PROJECT_SOURCE_DIR}/src/common/br
    )
endif()

if(OT_NCP_RADIO)
    list(APPEND PLATFORM_FILES
        ${PROJECT_SOURCE_DIR}/src/common/ncp/ncp_glue_ot.c
        ${PROJECT_SOURCE_DIR}/src/common/ncp/ncp_ot.c
        platform/ncp.c
    )

    # For rw612 ot ncp libs, no lp code is needed since it will use wifi/ble low power desing in ncp coex
    if(NOT DEFINED OT_NXP_BUILD_APP_AS_LIB)
        list(APPEND PLATFORM_FILES
            ${PROJECT_SOURCE_DIR}/src/common/ncp/sys/ncp_lp_sys.c
            ${PROJECT_SOURCE_DIR}/src/common/ncp/sys/ncp_lpm.c
            ${PROJECT_SOURCE_DIR}/src/common/ncp/notify/app_notify.c
        )
    endif()

    set(OT_NCP_INCLUDES
        ${PROJECT_SOURCE_DIR}/src/common/ncp
        ${PROJECT_SOURCE_DIR}/src/common/ncp/ot_opcode
        ${PROJECT_SOURCE_DIR}/examples/common/includes
    )

    if(NOT DEFINED OT_NXP_BUILD_APP_AS_LIB)
        list(APPEND OT_NCP_INCLUDES
            ${PROJECT_SOURCE_DIR}/src/common/ncp/sys
            ${PROJECT_SOURCE_DIR}/src/common/ncp/notify
	)
    endif()
else()
    list(APPEND PLATFORM_FILES
        ${PROJECT_SOURCE_DIR}/src/common/uart.c
    )
endif()

if(OT_NCP_RADIO MATCHES "ON")
    if(NOT DEFINED OT_NXP_NCP_UART_INTERFACE
       AND NOT DEFINED OT_NXP_NCP_USB_INTERFACE
       AND NOT DEFINED OT_NXP_NCP_SPI_INTERFACE
       AND NOT DEFINED OT_NXP_NCP_SDIO_INTERFACE)
        set(OT_NXP_NCP_UART_INTERFACE "ON") # Use UART as default NCP adapter interface
    endif()

    if(OT_NXP_NCP_UART_INTERFACE MATCHES "ON")
        target_compile_definitions(ot-config INTERFACE
            -DOT_NCP_RADIO=1
            -DCONFIG_NCP_UART=1
        )
    elseif(OT_NXP_NCP_USB_INTERFACE MATCHES "ON")
        target_compile_definitions(ot-config INTERFACE
            -DOT_NCP_RADIO=1
            -DCONFIG_NCP_USB=1
        )
    elseif(OT_NXP_NCP_SPI_INTERFACE MATCHES "ON")
        target_compile_definitions(ot-config INTERFACE
            -DOT_NCP_RADIO=1
            -DCONFIG_NCP_SPI=1
        )
    elseif(OT_NXP_NCP_SDIO_INTERFACE MATCHES "ON")
        target_compile_definitions(ot-config INTERFACE
            -DOT_NCP_RADIO=1
            -DCONFIG_NCP_SDIO=1
        )
    else()
        message(FATAL_ERROR "Please select one NCP interface.")
    endif()

    if(OT_NXP_BUILD_APP_AS_LIB MATCHES "ON")
        target_compile_definitions(ot-config INTERFACE
            -DOT_NCP_LIBS=1
        )
    endif()
endif()

add_library(${OT_PLATFORM_LIB}
    ${PLATFORM_FILES}
    ${PROJECT_SOURCE_DIR}/src/common/spinel/radio.cpp
    ${PROJECT_SOURCE_DIR}/src/common/spinel/spinel_hdlc.cpp
    platform/reset.c
    ${PROJECT_SOURCE_DIR}/src/common/spinel/system.c
    ${PROJECT_SOURCE_DIR}/src/common/spinel/misc.c
    ${PROJECT_SOURCE_DIR}/src/common/alarm_freertos.c
    #${PROJECT_SOURCE_DIR}/src/common/uart.c
    ${PROJECT_SOURCE_DIR}/src/common/flash_fsa.c
    ${PROJECT_SOURCE_DIR}/src/common/logging.c
    ${PROJECT_SOURCE_DIR}/src/common/entropy.c
    ${PROJECT_SOURCE_DIR}/src/common/diag.c
)

set_target_properties(
    ${OT_PLATFORM_LIB}
    PROPERTIES
        C_STANDARD 99
        CXX_STANDARD 11
)

target_link_libraries(${OT_PLATFORM_LIB}
    PRIVATE
        ot-config
        openthread-platform
        openthread-platform-utils-static
        openthread-hdlc
        openthread-radio-spinel
        openthread-url
        openthread-spinel-rcp
        connfwk-FunctionLib
        connfwk-FSAbstraction
        connfwk-LowPower
        connfwk-platform-rw61x
        ${NXP_DRIVER_LIB}
        ${NXP_BOARD_LIB}
    PUBLIC
        connfwk-config
        ${OT_LWIP_LIBS}
        -T${PROJECT_SOURCE_DIR}/src/rw/rw612/rw612.ld
        -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_PROPERTY:NAME>.map,-print-memory-usage
        -Wl,--gc-sections,--defsym=gUseNVMLink_d=1
)

# Rebuild executables if the linker script is changed
set_target_properties(ot-config PROPERTIES INTERFACE_LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rw612.ld)

target_compile_definitions(${OT_PLATFORM_LIB}
    PUBLIC
        ${OT_PLATFORM_DEFINES}
        -DOT_PLAT_SPINEL_OVER_HDLC
        -DOT_PLAT_SYS_CRYPTO_INIT
        -DOT_PLAT_MBEDTLS_PORT_INCLUDE=\"els_pkc_mbedtls.h\"
        -DLOG_BUFFER_ENABLED
    PRIVATE
        -DOT_APP_THREAD_RATE_LIMIT=${OT_APP_THREAD_RATE_LIMIT}
)

if(OT_FTD)
    target_compile_definitions(${OT_PLATFORM_LIB}
        PUBLIC
            -DOPENTHREAD_FTD=1
    )
elseif(OT_MTD)
    target_compile_definitions(${OT_PLATFORM_LIB}
        PUBLIC
            -DOPENTHREAD_MTD=1
    )
endif()

if(OT_NXP_INDEPENDENT_RESET MATCHES "ON")
    target_compile_definitions(${OT_PLATFORM_LIB}
	PUBLIC
            -DOT_NXP_INDEPENDENT_RST=1
    )
endif()

if(OT_NXP_BUILD_APP_AS_LIB MATCHES "ON")
    if(OT_APP_CLI_FREERTOS_IPERF MATCHES "ON")
        target_compile_definitions(${OT_PLATFORM_LIB}
            PUBLIC
            -DDISABLE_TCPIP_INIT #Do not initialize LWIP stack, if building ot lib for tri radio
        )
    endif()
endif()

if (A2_ETH_REWORK MATCHES "ON")
    target_compile_definitions(${OT_PLATFORM_LIB}
        PUBLIC
            -DgBoardUseFro32k_d=1
    )
endif()

target_compile_options(${OT_PLATFORM_LIB}
    PRIVATE
        ${OT_CFLAGS}
        -Wno-unknown-pragmas
        -Wno-sign-compare
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-empty-body
)

target_include_directories(${OT_PLATFORM_LIB}
    PUBLIC
        ${OT_LWIP_INCLUDES}
        ${OT_BR_INCLUDES}
        ${OT_NCP_INCLUDES}
        ${PROJECT_SOURCE_DIR}/src/common
        ${PROJECT_SOURCE_DIR}/src/common/spinel
    PRIVATE
        ${OT_PUBLIC_INCLUDES}
        ${PROJECT_SOURCE_DIR}/openthread/examples/platforms
        ${PROJECT_SOURCE_DIR}/openthread/third_party/mbedtls/repo/include
)
