#
#  Copyright (c) 2021-2025, The OpenThread Authors.
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

if (NOT OT_NXP_PLATFORM MATCHES "nbu")
    set(OT_NXP_PLATFORM_SOURCES
        platform/alarm.c
        platform/diag.c
        platform/logging.c
        platform/misc.c
        platform/radio.c
        platform/system.c
        platform/uart.c
    )

    if (OT_NXP_NVS MATCHES "ON")
        list(APPEND OT_NXP_PLATFORM_SOURCES
            ../common/flash_nvs.c
        )
    else()
        list(APPEND OT_NXP_PLATFORM_SOURCES
            ../common/flash_nvm.c
        )
    endif()

    if (OT_NXP_PLATFORM MATCHES "^mcxw30$")
        list(APPEND OT_NXP_PLATFORM_SOURCES
            mcxw30/entropy.c
        )
    else()
        list(APPEND OT_NXP_PLATFORM_SOURCES
            platform/entropy.c
        )
    endif()

    if (NOT NO_THREAD_1_3_FLAGS)
        list(APPEND OT_NXP_PLATFORM_SOURCES
            ../common/crypto/ecdsa_psa.cpp
        )
    endif()
else()
    include(${PROJECT_SOURCE_DIR}/src/mcxw/mcxw72_nbu/mcxw72_nbu.cmake)
endif()

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

set(OT_NXP_PLATFORM_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/openthread/examples/platforms
    ../common
)

include(${PROJECT_SOURCE_DIR}/src/mcxw/platform/mcxw.cmake)