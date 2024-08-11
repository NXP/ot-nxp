/* @file app_notify.h
 *
 *  @brief This file contains ncp device API functions definitions
 *
 *  Copyright 2008-2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_NOTIFY_H__
#define __APP_NOTIFY_H__

#include <stdint.h>

typedef enum
{
    /** Event for MCU enter sleep */
    APP_EVT_MCU_SLEEP_ENTER = 0,
    /** Event for MCU exit sleep */
    APP_EVT_MCU_SLEEP_EXIT,
    /** Event for invalid command */
    APP_EVT_INVALID_CMD,
} app_notify_event_t;

typedef enum
{
    APP_EVT_REASON_SUCCESS = 0,
    APP_EVT_REASON_FAILURE,
} app_event_reason_t;

#define APP_NOTIFY_SUSPEND_EVT 0x1U
#define APP_NOTIFY_SUSPEND_CFM 0x2U

/* app notify event queue message */
typedef struct
{
    uint16_t event;
    int      reason;
    int      data_len;
    void    *data;
} app_notify_msg_t;

int app_notify_event(uint16_t event, int result, void *data, int len);

int app_notify_init(void);

#endif /* __APP_NOTIFY_H__ */
