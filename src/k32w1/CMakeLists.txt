#
#  Copyright (c) 2021-2023, The OpenThread Authors.
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

set(K32W1_SOURCES
    alarm.c
    diag.c
    entropy.c
    logging.c
    misc.c
    radio.c
    system.c
    uart.c
    ../common/flash_nvm.c
    ../../openthread/examples/apps/cli/cli_uart.cpp
)

if (NOT NO_THREAD_1_3_FLAGS)
    list(APPEND K32W1_SOURCES
        ../common/crypto/ecdsa_sss.cpp
    )
endif()

list(APPEND OT_PLATFORM_DEFINES
    "USE_NBU=${USE_NBU}"
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

set(K32W1_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/openthread/examples/platforms
)

include(k32w1.cmake)
