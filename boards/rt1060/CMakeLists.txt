#
#  Copyright (c) 2023, The OpenThread Authors.
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

add_library(${NXP_BOARD_LIB}
    board.c
    pin_mux.c
    clock_config.c
    hardware_init.c
    peripherals.c
)

target_include_directories(${NXP_BOARD_LIB}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OT_NXP_FREERTOS_CONFIG_PATH}
)

target_link_libraries(${NXP_BOARD_LIB}
    PRIVATE
        ${NXP_DRIVER_LIB}
)

if (OT_STACK_ENABLE_LOG)
#move logs to UART2
target_compile_definitions(${NXP_BOARD_LIB}
    PUBLIC
    BOARD_DEBUG_UART_BASEADDR=LPUART2
    BOARD_DEBUG_UART_INSTANCE=2U
    BOARD_UART_IRQ=LPUART2_IRQn
    BOARD_UART_IRQ_HANDLER=LPUART2_IRQHandler
)
endif ()

if ("${OT_NXP_TRANSCEIVER}" STREQUAL "k32w0")
target_compile_definitions(${NXP_BOARD_LIB}
    PRIVATE
    BOARD_OTW_K32W0_PIN_INIT
)
endif()

target_compile_options(${NXP_BOARD_LIB}
    PRIVATE
        ${OT_CFLAGS}
        -Wno-unknown-pragmas
        -Wno-sign-compare
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-empty-body
)