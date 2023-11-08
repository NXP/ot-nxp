/*
 *  Copyright 2023 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef BOARD_USE_M2
#define WIFI_BT_USE_M2_INTERFACE
#define WIFI_IW612_BOARD_MURATA_2EL_M2
#else
#define WIFI_BT_USE_USD_INTERFACE
#define WIFI_IW612_BOARD_MURATA_2EL_USD
#endif

// #define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_2EL_CA_RU_Tx_power.h"
// #define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_2EL_EU_RU_Tx_power.h"
// #define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_2EL_JP_RU_Tx_power.h"
// #define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_2EL_US_RU_Tx_power.h"
#define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_2EL_WW.h"
#define SD9177
#define SDMMCHOST_OPERATION_VOLTAGE_1V8
#define SD_TIMING_MAX kSD_TimingDDR50Mode

#define WLAN_ED_MAC_CTRL                                                               \
    {                                                                                  \
        .ed_ctrl_2g = 0x1, .ed_offset_2g = 0xA, .ed_ctrl_5g = 0x1, .ed_offset_5g = 0xA \
    }

#define UAP_SUPPORT 1

#include "wifi_config.h"