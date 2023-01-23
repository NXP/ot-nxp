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


if (IWX12_TRANSCEIVER)
    set(TRANSEIVER_C_FILE
            board/sdmmc_config.c
    )
endif ()

if (MULTICORE_LOGGING)
    set(MULTICORE_LOGGING_C_FILE
            multicore/multicore.c
    )
    set (MULTICORE_PATH
            multicore
            multicore/m4_log_dump_uart2_image
    )
else ()
    set(MULTICORE_LOGGING_C_FILE )
endif ()

if ("${SPINEL_INTERFACE_TYPE}" STREQUAL "UART")
    #UART support
    set(SPINEL_FLAGS
        -DOT_PLAT_SPINEL_OVER_HDLC
    )
    set(SPINEL_FILES
        ../../common/spinel/spinel_hdlc.cpp
    )
else ()
    #Assuming SPI support
    set(SPINEL_FLAGS
        -DOT_PLAT_SPINEL_OVER_SPI
    )
    set(SPINEL_FILES
    ../../common/spinel/spi_interface.cpp
    )
endif ()


add_library(openthread-rt1170
    ../../common/alarm_freertos.c
    ../../common/diag.c
    ../../common/flash_littlefs.c
    ../../common/logging.c
    misc.c
    ../../common/spinel/radio.cpp
    ${SPINEL_FILES}
    ../../common/spinel/system.c
    ../../common/uart.c
    board/pin_mux.c
    board/hardware_init.c
    board/peripherals.c
    board/clock_config.c
    board/board.c
    ${TRANSEIVER_C_FILE}
    ${MULTICORE_LOGGING_C_FILE}
)

set_target_properties(
    openthread-rt1170
    PROPERTIES
        C_STANDARD 99
        CXX_STANDARD 11
)

target_link_libraries(openthread-rt1170
    PRIVATE
        ot-config
        openthread-hdlc
        openthread-spinel-rcp
        openthread-platform
    PUBLIC
        ${NXP_DRIVER_LIB}
        -T${PROJECT_SOURCE_DIR}/src/imx_rt/rt1170/RT1170_cm7.ld
        -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_PROPERTY:NAME>.map,-print-memory-usage
)

#OT needs freeRTOS config dependency
target_link_libraries(ot-config
    INTERFACE
    openthread-rt1170
)

target_compile_definitions(openthread-rt1170
    PUBLIC
        ${OT_PLATFORM_DEFINES}
)

target_compile_options(openthread-rt1170
    PRIVATE
        ${OT_CFLAGS}
        ${SPINEL_FLAGS}
        -DOT_PLAT_SYS_CRYPTO_INIT
        -DOT_PLAT_MBEDTLS_PORT_INCLUDE=\"ksdk_mbedtls.h\"
        -DOT_PLAT_SYS_LOG_MANAGEMENT
        -DOT_PLAT_SYS_RANDOM_DISABLE
        -DOT_PLAT_SAVE_NVM_DATA_ON_IDLE
        -DOPENTHREAD_FTD=1
        -Wno-unknown-pragmas
        -Wno-sign-compare
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-empty-body
)

if (MULTICORE_LOGGING)
    target_compile_options(openthread-rt1170
        PRIVATE
        -DMULTICORE_LOGGING_ENABLED
    )
    target_link_libraries(openthread-rt1170
        PUBLIC
        -Xlinker --defsym=__use_shmem__=1
    )
endif ()

if (IWX12_TRANSCEIVER)
    target_compile_options(openthread-rt1170
        PUBLIC
        -DSDIO_ENABLED
    )
endif ()

target_include_directories(openthread-rt1170
    PUBLIC
        board
        freertos
        ${MULTICORE_PATH}
        ../platform
        ${PROJECT_SOURCE_DIR}/src/common
        ${PROJECT_SOURCE_DIR}/src/common/spinel
    PRIVATE
        ${OT_PUBLIC_INCLUDES}
        ${PROJECT_SOURCE_DIR}/openthread/examples/platforms
)
