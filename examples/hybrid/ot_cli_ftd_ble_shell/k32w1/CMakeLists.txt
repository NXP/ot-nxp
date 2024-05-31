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

if (SDK_RELEASE)
    set(EXAMPLE_BASE_DIR ${BLE_EXMPL_DIR}/ble_shell)
else()
    if (SDK_GH)
        set(EXAMPLE_BASE_DIR ${BLE_EXMPL_DIR}/ble_shell)
    else()
        set(EXAMPLE_BASE_DIR ${BLE_EXMPL_DIR}/ble_shell)
    endif()
endif()

add_executable(ot-cli-ftd-ble-shell-${OT_NXP_PLATFORM}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/k32w1/main.c
    ${PROJECT_SOURCE_DIR}/openthread/examples/apps/cli/cli_uart.cpp

    ${EXAMPLE_BASE_DIR}/bm/app_advertiser.c
    ${EXAMPLE_BASE_DIR}/bm/app_ble_init.c
    ${EXAMPLE_BASE_DIR}/bm/app_conn.c
    ${EXAMPLE_BASE_DIR}/bm/app_connection.c
    ${EXAMPLE_BASE_DIR}/bm/app_nvm.c
    ${EXAMPLE_BASE_DIR}/bm/app_scanner.c
    ${EXAMPLE_BASE_DIR}/bm/app_services_init.c
    ${EXAMPLE_BASE_DIR}/bm/ble_init.c

    ${EXAMPLE_BASE_DIR}/app_config.c
    ${EXAMPLE_BASE_DIR}/ble_shell.c
    ${EXAMPLE_BASE_DIR}/shell_gap.c
    ${EXAMPLE_BASE_DIR}/shell_gatt.c
    ${EXAMPLE_BASE_DIR}/shell_gattdb.c
    ${EXAMPLE_BASE_DIR}/shell_thrput.c

    $ENV{NXP_K32W1_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/application/common/gatt_db/gatt_database_dynamic.c
)

target_include_directories(ot-cli-ftd-ble-shell-${OT_NXP_PLATFORM} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src

    ${EXAMPLE_BASE_DIR}
    ${EXAMPLE_BASE_DIR}/bm

    ${PROJECT_SOURCE_DIR}/openthread/examples/platforms

    $ENV{NXP_K32W1_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/application/common
    $ENV{NXP_K32W1_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/application/common/gatt_db
    $ENV{NXP_K32W1_SDK_ROOT}/${MIDLWR_DIR}/wireless/ble_controller/interface
)

target_compile_options(${NXP_DRIVER_LIB}
    PRIVATE
        -include ${CMAKE_CURRENT_SOURCE_DIR}/../src/k32w1/app_preinclude.h
)

target_compile_options(nxp-k32w1-ble
    PRIVATE
        -include ${CMAKE_CURRENT_SOURCE_DIR}/../src/k32w1/app_preinclude.h
)

target_include_directories(nxp-k32w1-ble PUBLIC
    ${EXAMPLE_BASE_DIR}
    ${EXAMPLE_BASE_DIR}/bm
)

target_compile_options(ot-cli-ftd-ble-shell-${OT_NXP_PLATFORM}
    PRIVATE
        -include ${CMAKE_CURRENT_SOURCE_DIR}/../src/k32w1/ble_preinclude.h
)

set(APP_LINKER_FILE ${PROJECT_SOURCE_DIR}/src/k32w1/k32w1/K32W1480_connectivity.ld)

target_compile_options(ot-config INTERFACE
    -Wno-unused-parameter
    -Wno-sign-compare
    -Wno-pedantic
    -Wno-unused-function
)

target_link_libraries(ot-cli-ftd-ble-shell-${OT_NXP_PLATFORM} PUBLIC
    ${SDK_CRYPTO_LIB}
    ${SDK_BLE_LIB}
    ${NXP_DRIVER_LIB}
    nxp-k32w1-ble
    ${SDK_CRYPTO_LIB}
    ${SDK_BLE_LIB}
    openthread-cli-ftd
    ${OT_PLATFORM_LIB}
    openthread-ftd
    ${OT_PLATFORM_LIB}
    openthread-cli-ftd
    ${OT_MBEDTLS}
    ot-config

    -Llib
    -T${APP_LINKER_FILE}
    -Wl,--wrap=otCliInputLine
    -Wl,--wrap=SerialManager_TryRead
    -Wl,--wrap=SerialManager_InstallRxCallback
)
