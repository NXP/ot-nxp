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


set(SDK_CRYPTO_LIB "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/SecLib/lib_crypto_m33.a" CACHE STRING "")
set(SDK_BLE_LIB "$ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/host/lib/lib_ble_host_cm33_gcc.a" CACHE STRING "")


list(APPEND BLE_LIB_SOURCES
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/platform/connected_mcu/fwk_platform_ble.c

        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/application/common/gatt_db/gatt_database.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/application/common/ble_conn_manager.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/application/common/ble_host_tasks.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/application/common/ble_service_discovery.c

        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/hci_transport/source/hcit_generic_adapter_interface.c

        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/host/config/ble_globals.c

        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/port/fwk_generic_list.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/port/fwk_messaging.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/port/fwk_os_abs.c
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/port/fwk_timer_manager.c

        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ble_controller/src/controller_api.c
)

add_library(nxp-mcxw72-ble
        ${BLE_LIB_SOURCES}
)

target_include_directories(nxp-mcxw72-ble
    PUBLIC
            $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/ModuleInfo
            $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/framework/DBG

            $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/host/interface
            $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/host/config
            $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/application/common/gatt_db/macros
            $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/port
            $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/bluetooth/hci_transport/interface

            $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ble_controller/interface
)

target_link_libraries(nxp-mcxw72-ble PUBLIC ${NXP_DRIVER_LIB})
