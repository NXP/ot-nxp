/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "board.h"
#include "clock_config.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"
#include "fsl_power.h"
#include "ncp_intf_pm.h"

#include "fwk_platform.h"
#include "fwk_platform_lowpower.h"

#include "ncp_glue_ot.h"
#include "ncp_lp_sys.h"
#include "ncp_lpm.h"

#if CONFIG_NCP_SPI
#include "ncp_intf_spi_slave.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
power_init_config_t initCfg = {
    /* VCORE AVDD18 supplied from iBuck on RD board. */
    .iBuck = true,
    /* Keep CAU_SOC_SLP_REF_CLK for LPOSC. */
    .gateCauRefClk = false,
};

status_t powerManager_PmNotify(pm_event_type_t eventType, uint8_t powerState, void *data);
AT_ALWAYS_ON_DATA_INIT(pm_notify_element_t ncp_pm_notify) = {
    .notifyCallback = powerManager_PmNotify,
    .data           = NULL,
};
status_t powerManager_BoardNotify(pm_event_type_t eventType, uint8_t powerState, void *data);
AT_ALWAYS_ON_DATA_INIT(pm_notify_element_t board_notify) = {
    .notifyCallback = powerManager_BoardNotify,
    .data           = NULL,
};

#if CONFIG_CRC32_HW_ACCELERATE
extern void ncp_tlv_chksum_init();
#endif

extern int32_t mflash_drv_init(void);

/* Default NCP host <-> NCP device low power handshake state */
static uint8_t ncpLowPowerHandshake = NCP_LMP_HANDSHAKE_NOT_START;

extern volatile uint8_t OtNcpDataHandle;

/*******************************************************************************
 * APIs
 ******************************************************************************/
uint8_t lpm_getHandshakeState(void)
{
    return ncpLowPowerHandshake;
}
void lpm_setHandshakeState(uint8_t state)
{
    ncpLowPowerHandshake = state;
}

void lpm_pm3_exit_hw_reinit()
{
    extern void BOARD_InitHardware(void);

    BOARD_InitHardware();

    POWER_InitPowerConfig(&initCfg);
#if CONFIG_CRC32_HW_ACCELERATE
    ncp_tlv_chksum_init();
#endif /* CONFIG_CRC32_HW_ACCELERATE */
    ncp_intf_pm_exit((int32_t)PM_LP_STATE_PM3);

    ncp_gpio_init();
    mflash_drv_init();
}

status_t powerManager_PmNotify(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    /* is ncp host <-> ncp device handshake done? */
    if (lpm_getHandshakeState() == NCP_LMP_HANDSHAKE_IN_PROCESS)
    {
        return kStatus_PMPowerStateNotAllowed;
    }

#if CONFIG_NCP_SPI
    if (ncp_spi_txrx_is_finish() == 0)
    {
        return kStatus_PMPowerStateNotAllowed;
    }
#endif /* CONFIG_NCP_SPI */
    if (eventType == kPM_EventEnteringSleep)
    {
        if (powerState >= 2)
        {
            host_sleep_pre_cfg(powerState);
        }
    }
    else if (eventType == kPM_EventExitingSleep)
    {
        if (powerState >= 2)
        {
            host_sleep_post_cfg(powerState);
        }
    }
    return kStatus_PMSuccess;
}

status_t powerManager_BoardNotify(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    /* is ncp host <-> ncp device handshake done? */
    if (lpm_getHandshakeState() == NCP_LMP_HANDSHAKE_IN_PROCESS)
    {
        return kStatus_PMPowerStateNotAllowed;
    }

    if (OtNcpDataHandle == OT_NCP_WAIT_RSP)
    {
        return kStatus_PMPowerStateNotAllowed;
    }

#if CONFIG_NCP_SPI
    if (ncp_spi_txrx_is_finish() == 0)
    {
        return kStatus_PMPowerStateNotAllowed;
    }
#endif /* CONFIG_NCP_SPI */
    if (eventType == kPM_EventEnteringSleep)
    {
        while (ncp_intf_pm_enter((int32_t)powerState) == NCP_PM_STATUS_NOT_READY)
            ;
        if (powerState == PM_LP_STATE_PM3)
        {
            ;
        }
        else
        {
            /* Do Nothing */
        }
    }
    else if (eventType == kPM_EventExitingSleep)
    {
        if (powerState == PM_LP_STATE_PM3)
        {
            lpm_pm3_exit_hw_reinit();
        }
        else if (powerState == PM_LP_STATE_PM2)
        {
            ncp_intf_pm_exit((int32_t)PM_LP_STATE_PM2);
        }
        else
        {
            /* Do Nothing */
        }
    }

    return kStatus_PMSuccess;
}

void ncp_pm_init(void)
{
    status_t status;

    status = PM_RegisterNotify(kPM_NotifyGroup1, &ncp_pm_notify);
    assert(status == kStatus_Success);
    /* Register board notifier */
    status = PM_RegisterNotify(kPM_NotifyGroup2, &board_notify);
    assert(status == kStatus_Success);
    (void)status;
}
