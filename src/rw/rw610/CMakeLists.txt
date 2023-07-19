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

add_library(openthread-rw610
    ../../common/alarm_freertos.c
    ../../common/uart.c
    ../../common/flash_nvm.c
    #../../common/entropy.c
    platform/entropy.c
    platform/diag.c
    platform/logging.c
    platform/misc.c
    platform/radio.c
    platform/system.c
    board/pin_mux.c
    board/hardware_init.c
)

set_target_properties(
    openthread-rw610
    PROPERTIES
        C_STANDARD 99
        CXX_STANDARD 11
)

SET (FCB_BINARY_PATH ${PROJECT_SOURCE_DIR}/src/rw/rw610/)

target_link_libraries(openthread-rw610
    PRIVATE
        ot-config
        openthread-platform
        openthread-platform-utils-static
    PUBLIC
        nxp-rw610-driver
        -T${PROJECT_SOURCE_DIR}/src/rw/rw610/rw610.ld
        -Wl,-Map=$<TARGET_PROPERTY:NAME>.map,-print-memory-usage
        -Wl,--gc-sections,--defsym=gUseNVMLink_d=1
        -L${FCB_BINARY_PATH}
        -Wl,-rpath=${FCB_BINARY_PATH}
)

target_compile_definitions(openthread-rw610
    PUBLIC
        ${OT_PLATFORM_DEFINES}
        -DOT_PLAT_UART_INSTANCE=3
        -DOT_PLAT_UART_CLK_FREQ=BOARD_DEBUG_UART_CLK_FREQ
        -DBOARD_DEBUG_UART_BAUDRATE=115200
        -DOT_PLAT_UART_BAUDRATE=BOARD_DEBUG_UART_BAUDRATE
        -DOT_PLAT_UART_TYPE=BOARD_DEBUG_UART_TYPE
)

target_compile_options(openthread-rw610
    PRIVATE
        ${OT_CFLAGS}
        -Wno-unknown-pragmas
        -Wno-sign-compare
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-empty-body
)

target_include_directories(openthread-rw610
    PUBLIC
        board
        freertos
    PRIVATE
        ${OT_PUBLIC_INCLUDES}
        ${PROJECT_SOURCE_DIR}/src/common
        ${PROJECT_SOURCE_DIR}/openthread/examples/platforms
        ${PROJECT_SOURCE_DIR}/openthread/third_party/mbedtls/repo/include
)
