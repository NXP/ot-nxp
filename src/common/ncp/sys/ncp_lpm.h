/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _NCP_LPM_H_
#define _NCP_LPM_H_

#include "stdint.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * NCP handshake process definition
 * 0 -> not start
 * 1 -> in process
 * 2 -> finish
 */
#define NCP_LMP_HANDSHAKE_NOT_START 0
#define NCP_LMP_HANDSHAKE_IN_PROCESS 1
#define NCP_LMP_HANDSHAKE_FINISH 2

#if defined(configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY)
#ifndef LPM_RTC_PIN1_PRIORITY
#define LPM_RTC_PIN1_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)
#endif
#else
#ifndef LPM_RTC_PIN1_PRIORITY
#define LPM_RTC_PIN1_PRIORITY (3U)
#endif
#endif

#define APP_NOTIFY_SUSPEND_EVT 0x1U
#define APP_NOTIFY_SUSPEND_CFM 0x2U

/*******************************************************************************
 * API
 ******************************************************************************/
void lpm_pm3_exit_hw_reinit();

uint8_t lpm_getHandshakeState(void);
void    lpm_setHandshakeState(uint8_t state);
void    ncp_pm_init(void);

#endif /* _NCP_LPM_H_ */
