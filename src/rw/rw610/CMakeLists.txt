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

if ("${SPINEL_INTERFACE_TYPE}" STREQUAL "RPMSG")
    set(PLATFORM_FILES
        ${PROJECT_SOURCE_DIR}/src/common/spinel/radio.cpp
        ${PROJECT_SOURCE_DIR}/src/common/spinel/spinel_hdlc.cpp
        ${PROJECT_SOURCE_DIR}/src/common/spinel/system.c
        ${PROJECT_SOURCE_DIR}/src/common/spinel/misc.c
    )
    set(OT_SPINEL_LIBS
        openthread-hdlc
        openthread-spinel-rcp
    )
    set(OT_SPINEL_CFLAGS
        -DOT_PLAT_SPINEL_OVER_HDLC
    )
    set(OT_SPINEL_INCLUDES
        ${PROJECT_SOURCE_DIR}/src/common/spinel
    )
else()
    set(PLATFORM_FILES
        platform/radio.c
        platform/misc.c
        platform/system.c
    )
endif()

if(OT_NXP_LWIP)
    list(APPEND PLATFORM_FILES
        ${PROJECT_SOURCE_DIR}/src/common/lwip/ot_lwip.c
    )
    set(OT_LWIP_INCLUDES
        ${PROJECT_SOURCE_DIR}/src/common/lwip
    )
    set(OT_LWIP_LIBS
        nxp-lwip
        openthread-ftd
    )
endif()

add_library(${OT_PLATFORM_LIB}
    ${PLATFORM_FILES}
    ${PROJECT_SOURCE_DIR}/src/common/alarm_freertos.c
    platform/alarm.c
    ${PROJECT_SOURCE_DIR}/src/common/uart.c
    ${PROJECT_SOURCE_DIR}/src/common/flash_littlefs.c
    ${PROJECT_SOURCE_DIR}/src/common/logging.c
    #../../common/entropy.c
    platform/entropy.c
    platform/diag.c
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
        ${OT_SPINEL_LIBS}
        connfwk-config
        connfwk-FunctionLib
        connfwk-FileSystem
        connfwk-platform-rdrw610
        ${NXP_DRIVER_LIB}
        ${NXP_BOARD_LIB}
    PUBLIC
        ${OT_LWIP_LIBS}
        -T${PROJECT_SOURCE_DIR}/src/rw/rw610/rw610.ld
        -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_PROPERTY:NAME>.map,-print-memory-usage
        -Wl,--gc-sections,--defsym=gUseNVMLink_d=1
)

# Rebuild executables if the linker script is changed
set_target_properties(ot-config PROPERTIES INTERFACE_LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rw610.ld)

if (NOT "${SPINEL_INTERFACE_TYPE}" STREQUAL "RPMSG")
    target_link_libraries(${OT_PLATFORM_LIB}
        PRIVATE
            ieee_802_15_4_rw610
    )
endif()

if(NOT DEFINED OT_PLAT_UART_INSTANCE)
    set(OT_PLAT_UART_INSTANCE 3)
endif()

target_compile_definitions(${OT_PLATFORM_LIB}
    PUBLIC
        ${OT_PLATFORM_DEFINES}
        ${OT_SPINEL_CFLAGS}
        -DOT_PLAT_UART_INSTANCE=${OT_PLAT_UART_INSTANCE}
        -DOT_PLAT_UART_TYPE=BOARD_DEBUG_UART_TYPE
        -DOT_PLAT_SYS_CRYPTO_INIT
        -DOT_PLAT_MBEDTLS_PORT_INCLUDE=\"els_pkc_mbedtls.h\"
)

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
        ${OT_SPINEL_INCLUDES}
        ${OT_LWIP_INCLUDES}
        ${PROJECT_SOURCE_DIR}/src/common
    PRIVATE
        ${OT_PUBLIC_INCLUDES}
        ${PROJECT_SOURCE_DIR}/openthread/examples/platforms
        ${PROJECT_SOURCE_DIR}/openthread/third_party/mbedtls/repo/include
)
