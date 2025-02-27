#
#  Copyright (c) 2024, The OpenThread Authors.
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

list(APPEND OT_PUBLIC_DEFINES
    -Wno-unused-parameter
    -Wno-expansion-to-defined
)

list(APPEND OT_PUBLIC_INCLUDES
        "$ENV{NXP_MCXW72_SDK_ROOT}"
        "$ENV{NXP_MCXW72_SDK_ROOT}/CMSIS/Include"
        "$ENV{NXP_MCXW72_SDK_ROOT}/devices/MCXW727C"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/common"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/flash_k4"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/gpio"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/lpspi"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/lpuart"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/ltc"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/port"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/cmc"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/ccm32k"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/crc"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/tpm"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${DRV_DIR}/lptmr"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${UTL_DIR}/misc_utilities"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${UTL_DIR}/debug_console"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${UTL_DIR}/str"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/osa"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/lists"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/messaging"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/mem_manager"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/internal_flash"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/panic"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/serial_manager"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/uart"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/gpio"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/led"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/button"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/timer_manager"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/timer"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/power_manager/devices/KW47B42ZB7"
        "$ENV{NXP_MCXW72_SDK_ROOT}/components/power_manager/core"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/mbedtls/"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/mbedtls/include/"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/Common"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/connected_mcu"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/include"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/DBG"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/FunctionLib"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/HWParameter"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/SecLib"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/RNG"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/Sensors"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/LowPower"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/boards/kw47_mcxw72x"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv/nb2p4ghz"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/XCVR/drv/nb2p4ghz/configs/gen45"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/source"
        "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/ieee_802_15_4/phy/interface"
)

set(MCXW72_SOURCES
    ../../k32w1/alarm.c
    ../../k32w1/diag.c
    ../../k32w1/entropy.c
    ../../k32w1/logging.c
    ../../k32w1/misc.c
    ../../k32w1/radio.c
    ../../k32w1/system.c
    ../../k32w1/uart.c
    ../../common/flash_nvm.c
    ../../../openthread/examples/apps/cli/cli_uart.cpp
)

if (NOT NO_THREAD_1_3_FLAGS)
    list(APPEND MCXW72_SOURCES
        ../../common/crypto/ecdsa_sss.cpp
    )
endif()

set(MCXW72_LIBS
    $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/SecLib/lib_crypto_m33.a
)

if (NOT OT_MAC_CSL_RECEIVER_ENABLE)
    # disable CSL receiver for now
    list(APPEND OT_PLATFORM_DEFINES
        OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE=0
    )
endif()

if (NO_THREAD_1_3_FLAGS)
    list(APPEND OT_PLATFORM_DEFINES
        "NO_THREAD_1_3_FLAGS"
    )
endif()

set(MCXW72_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/mcxw/mcxw72
    ${PROJECT_SOURCE_DIR}/openthread/examples/platforms
)

include(mcxw72.cmake)
