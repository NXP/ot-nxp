# Copyright (c) 2022, NXP.
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the
#    names of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.

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

set(PLATFORM_C_FLAGS "-mcpu=cortex-m7 -mfloat-abi=hard -mthumb -mfpu=fpv5-d16 -fno-common -ffreestanding -fno-builtin -mapcs")
set(PLATFORM_CXX_FLAGS "${PLATFORM_C_FLAGS} -MMD -MP")
set(PLATFORM_LINKER_FLAGS "${PLATFORM_C_FLAGS} -u qspiflash_config -u image_vector_table -u boot_data -u dcd_data -Wl,--sort-section=alignment -Wl,--cref")

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${PLATFORM_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PLATFORM_CXX_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${PLATFORM_C_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PLATFORM_LINKER_FLAGS} ")

# FreeRTOS CMake config
set(FREERTOS_PORT GCC_ARM_CM4F CACHE STRING "")
set(FREERTOS_HEAP 4)

# Connectivity Framework CMake config
set(CONNFWK_PLATFORM rt1170)
set(CONNFWK_PLATFORM_FAMILY imx_rt)
set(CONNFWK_TRANSCEIVER ${OT_NXP_TRANSCEIVER})
if ("${OT_NXP_TRANSCEIVER}" STREQUAL "k32w0")
    #Define here the default transceiver path can be overwritten by cmake -D option
    set(OT_NXP_TRANSCEIVER_BIN_PATH "${PROJECT_SOURCE_DIR}/build_k32w061/rcp_only_uart_flow_control/bin/ot-rcp.elf.bin.h" CACHE PATH "Path to the transceiver binary file")
    set(CONNFWK_TRANSCEIVER_BIN_PATH ${OT_NXP_TRANSCEIVER_BIN_PATH})
    set(CONNFWK_OTW ON)
    set(CONNFWK_COMPILE_DEFINITIONS
        #OTW configurations
        -DPLATFORM_OTW_RESET_PIN_PORT=3
        -DPLATFORM_OTW_RESET_PIN_NUM=9
        -DPLATFORM_OTW_DIO5_PIN_PORT=6 #TODO identify DIO pin on RT1170
        -DPLATFORM_OTW_DIO5_PIN_NUM=26 #TODO identify DIO pin on RT1170
    )
    if ("${SPINEL_INTERFACE_TYPE}" STREQUAL "UART")
        list(APPEND CONNFWK_COMPILE_DEFINITIONS
            #HDLC configuration
            -DSPINEL_UART_INSTANCE=7
            -DSPINEL_ENABLE_RX_RTS=1
            -DSPINEL_ENABLE_TX_RTS=1
            -DSPINEL_UART_CLOCK_RATE=CLOCK_GetRootClockFreq\(kCLOCK_Root_Lpuart7\)
        )
    endif()
elseif(${OT_NXP_TRANSCEIVER} STREQUAL "iwx12")
    set(CONNFWK_COMPILE_DEFINITIONS
        -DPLATFORM_RESET_PIN_PORT=3
        -DPLATFORM_RESET_PIN_NUM=9
    )
endif()
# Enable FunctionLib and FileSystem modules
set(CONNFWK_FLIB ON)
set(CONNFWK_FILESYSTEM ON)
