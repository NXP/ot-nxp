/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_os_abstraction.h"

#ifndef CONFIG_TICK_RATE_HZ
#define CONFIG_TICK_RATE_HZ 1000U
#endif

uint32_t BOARD_GetSystemCoreClockFreq();
void     create_ot_task();

static void init_sys_tick()
{
    SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk);
    SysTick->LOAD |= (BOARD_GetSystemCoreClockFreq() / CONFIG_TICK_RATE_HZ) - 1U;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void tx_application_define(void *p)
{
    (void)p;

    create_ot_task();

    init_sys_tick();
}

void DispatchIRQ()
{
}

void BleDispatchIRQ()
{
}
