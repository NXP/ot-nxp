#
#  Copyright (c) 2024-2025, The OpenThread Authors.
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

add_executable(ot-cli-ftd-ble-shell-${OT_NXP_PLATFORM}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/mcxw/main.c
    ${PROJECT_SOURCE_DIR}/openthread/examples/apps/cli/cli_uart.cpp
    ${SdkRootDirPath}/middleware/wireless/bluetooth/application/common/app_conn.c
    ${SdkRootDirPath}/middleware/wireless/bluetooth/application/common/ble_init.c
    ${SdkRootDirPath}/middleware/wireless/bluetooth/application/common/app_advertiser.c
    ${SdkRootDirPath}/middleware/wireless/bluetooth/application/common/app_connection.c
    ${SdkRootDirPath}/middleware/wireless/bluetooth/application/common/app_scanner.c
    ${SdkRootDirPath}/middleware/wireless/bluetooth/application/common/app_nvm.c

    ${SdkRootDirPath}/middleware/wireless/bluetooth/examples/ble_shell/app_config.c
    ${SdkRootDirPath}/middleware/wireless/bluetooth/examples/ble_shell/ble_shell.c
    ${SdkRootDirPath}/middleware/wireless/bluetooth/examples/ble_shell/shell_gap.c
    ${SdkRootDirPath}/middleware/wireless/bluetooth/examples/ble_shell/shell_gatt.c
    ${SdkRootDirPath}/middleware/wireless/bluetooth/examples/ble_shell/shell_gattdb.c
    ${SdkRootDirPath}/middleware/wireless/bluetooth/examples/ble_shell/shell_thrput.c
    ${SdkRootDirPath}/middleware/wireless/bluetooth/examples/ble_shell/dynamic_gatt_database.c
)

target_include_directories(ot-cli-ftd-ble-shell-${OT_NXP_PLATFORM} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/
    ${PROJECT_SOURCE_DIR}/openthread/examples/platforms
    ${SdkRootDirPath}/middleware/wireless/bluetooth/examples/ble_shell/
)

target_include_directories(${OT_MCUX_SDK_TARGET} PUBLIC
    ${SdkRootDirPath}/middleware/wireless/bluetooth/application/common
    ${SdkRootDirPath}/middleware/wireless/bluetooth/examples/ble_shell
    ${SdkRootDirPath}/middleware/wireless/bluetooth/boards/${board}
)

target_compile_options(${OT_MCUX_SDK_TARGET}
    PRIVATE
        -include ${CMAKE_CURRENT_SOURCE_DIR}/../src/mcxw/app_preinclude.h
)

target_compile_options(ot-cli-ftd-ble-shell-${OT_NXP_PLATFORM}
    PRIVATE
        -include ${CMAKE_CURRENT_SOURCE_DIR}/../src/mcxw/ble_preinclude.h
)

target_compile_options(ot-config INTERFACE
    -Wno-unused-parameter
    -Wno-sign-compare
    -Wno-pedantic
    -Wno-unused-function
)

target_link_libraries(ot-cli-ftd-ble-shell-${OT_NXP_PLATFORM} PUBLIC
    ${OT_MCUX_SDK_TARGET}
    # Use --start-group to account for circular dependencies
    -Wl,--start-group
    ${OT_PLATFORM_LIB}
    openthread-cli-ftd
    openthread-ftd
    ot-config
    -Wl,--end-group

    -Llib
    -Wl,--wrap=otCliInputLine
    -Wl,--wrap=SerialManager_TryRead
    -Wl,--wrap=SerialManager_InstallRxCallback
)
