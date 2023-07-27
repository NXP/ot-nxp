/*
 *  Copyright 2023 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_WW.h"
#define IW61x
#define SDMMCHOST_OPERATION_VOLTAGE_1V8
#define SD_TIMING_MAX kSD_TimingDDR50Mode

#ifdef BOARD_USE_M2
#define WIFI_BT_USE_M2_INTERFACE
#define WIFI_IW61x_BOARD_MURATA_2EL_M2
#else
#define WIFI_BT_USE_USD_INTERFACE
#define WIFI_IW61x_BOARD_MURATA_2EL_USD
#endif

#define WLAN_ED_MAC_CTRL                                                               \
    {                                                                                  \
        .ed_ctrl_2g = 0x1, .ed_offset_2g = 0x6, .ed_ctrl_5g = 0x1, .ed_offset_5g = 0x6 \
    }

#include "wifi_config.h"