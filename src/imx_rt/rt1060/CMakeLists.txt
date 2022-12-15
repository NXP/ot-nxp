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

add_library(openthread-rt1060
    ../../common/alarm_freertos.c
    ../../common/diag.c
    ../../common/entropy.c
    ../../common/flash_littlefs.c
    ../../common/logging.c
    ../../common/spinel/misc.c
    ../../common/spinel/radio.cpp
    ../../common/spinel/spinel_hdlc.cpp
    ../../common/spinel/system.c
    ../../common/uart.c
    board/pin_mux.c
    board/hardware_init.c
    board/peripherals.c
    board/clock_config.c
    board/board.c
)


set_target_properties(
    openthread-rt1060
    PROPERTIES
        C_STANDARD 99
        CXX_STANDARD 11
)

target_link_libraries(openthread-rt1060
    PRIVATE
        ot-config
        openthread-hdlc
        openthread-spinel-rcp
        openthread-platform
    PUBLIC
        ${NXP_DRIVER_LIB}
        -T${PROJECT_SOURCE_DIR}/src/imx_rt/rt1060/RT1060.ld
        -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_PROPERTY:NAME>.map,-print-memory-usage
)

#OT needs freeRTOS config dependency
target_link_libraries(ot-config
    INTERFACE
    openthread-rt1060
)

target_compile_definitions(openthread-rt1060
    PUBLIC
        ${OT_PLATFORM_DEFINES}
)

target_compile_options(openthread-rt1060
    PRIVATE
        ${OT_CFLAGS}
        -DOT_PLAT_SYS_CRYPTO_INIT
        -DOT_PLAT_MBEDTLS_PORT_INCLUDE=\"ksdk_mbedtls.h\"
        -DOT_PLAT_SYS_LOG_MANAGEMENT
        -DOT_PLAT_SAVE_NVM_DATA_ON_IDLE=1
        -DOT_PLAT_SPINEL_OVER_HDLC
        -DOPENTHREAD_FTD=1
        -Wno-unknown-pragmas
        -Wno-sign-compare
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-empty-body
)

target_include_directories(openthread-rt1060
    PUBLIC
        board
        freertos
        ../platform
        ${PROJECT_SOURCE_DIR}/src/common/spinel
        ${PROJECT_SOURCE_DIR}/src/common
    PRIVATE
        ${OT_PUBLIC_INCLUDES}
        ${PROJECT_SOURCE_DIR}/openthread/examples/platforms
        ${PROJECT_SOURCE_DIR}/openthread/third_party/mbedtls/repo/include
)

if (OT_BUILD_COEX)
    target_compile_definitions(ot-config
        INTERFACE
        OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE=1
)
endif()