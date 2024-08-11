/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _NCP_LP_SYS_H_
#define _NCP_LP_SYS_H_

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

/* Leave AON modules on in PM2 */
#define NCP_PM2_MEM_PU_CFG \
    ((uint32_t)kPOWER_Pm2MemPuAon1 | (uint32_t)kPOWER_Pm2MemPuAon0 | (uint32_t)kPOWER_Pm2MemPuSdio)
/* All ANA in low power mode in PM2 */
#if 1
#define NCP_PM2_ANA_PU_CFG (10U)
#else
#define NCP_PM2_ANA_PU_CFG (0U)
#endif
/* Buck18 and Buck11 both in sleep level in PM3 */
#define NCP_PM3_BUCK_CFG (0U)
/* All clock gated */
#define NCP_SOURCE_CLK_GATE ((uint32_t)kPOWER_ClkGateAll)
/* All SRAM kept in retention in PM3, AON SRAM shutdown in PM4 */
#define NCP_MEM_PD_CFG (1UL << 8)

#define CONFIG_NCP_SUSPEND_STACK_SIZE 1024

#define SUSPEND_EVENT_TRIGGERS (1U << 0U)

#define NCP_NOTIFY_HOST_GPIO 27
#define NCP_NOTIFY_HOST_GPIO_MASK 0x8000000

/*******************************************************************************
 * API
 ******************************************************************************/
void lpm_pm3_exit_hw_reinit();

int ncp_config_suspend_mode(int mode);

int host_sleep_pre_cfg(int mode);

void host_sleep_post_cfg(int mode);

void ncp_pm_init(void);

void ncp_gpio_init(void);

void ncp_notify_host_gpio_init(void);
void ncp_notify_host_gpio_output(void);

#endif /* _NCP_LP_SYS_H_ */
