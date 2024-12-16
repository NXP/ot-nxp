/*
 *  Copyright 2020-2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _WIFI_CONFIG_H_
#define _WIFI_CONFIG_H_

#include "app_config.h"

#include "wifi_bt_module_config.h"

#ifdef WIFI_USER_CONFIG_FILE
#include WIFI_USER_CONFIG_FILE
#endif /* WIFI_USER_CONFIG_FILE */

#define CONFIG_IPV6 1

#ifndef CONFIG_MAX_IPV6_ADDRESSES
#define CONFIG_MAX_IPV6_ADDRESSES 8
#endif /* CONFIG_MAX_IPV6_ADDRESSES */

#ifndef CONFIG_WIFI_MAX_PRIO
#define CONFIG_WIFI_MAX_PRIO 8
#endif

#ifndef CONFIG_WIFI_AUTO_POWER_SAVE
#define CONFIG_WIFI_AUTO_POWER_SAVE 0
#endif

#define CONFIG_WIFI_GET_LOG 1 /* missing in wifi_config_default.h for rt1060 and rt1170 */

/*
 * Wifi extra debug options
 */
#define CONFIG_WIFI_EXTRA_DEBUG 0
#define CONFIG_WIFI_EVENTS_DEBUG 0
#define CONFIG_WIFI_CMD_RESP_DEBUG 0
#define CONFIG_WIFI_PKT_DEBUG 0
#define CONFIG_WIFI_SCAN_DEBUG 0
#define CONFIG_WIFI_IO_INFO_DUMP 0
#define CONFIG_WIFI_IO_DEBUG 0
#define CONFIG_WIFI_IO_DUMP 0
#define CONFIG_WIFI_MEM_DEBUG 0
#define CONFIG_WIFI_AMPDU_DEBUG 0
#define CONFIG_WIFI_TIMER_DEBUG 0
#define CONFIG_WIFI_SDIO_DEBUG 0
#define CONFIG_WIFI_FW_DEBUG 0
#define CONFIG_WIFI_UAP_DEBUG 0
#define CONFIG_WPS_DEBUG 0
#define CONFIG_FW_VDLL_DEBUG 0
#define CONFIG_DHCP_SERVER_DEBUG 0
#define CONFIG_WIFI_SDIO_DEBUG 0
#define CONFIG_FWDNLD_IO_DEBUG 0

/*
 * Heap debug options
 */
#define CONFIG_HEAP_DEBUG 0
#define CONFIG_HEAP_STAT 0

#endif /* _WIFI_CONFIG_H_ */
