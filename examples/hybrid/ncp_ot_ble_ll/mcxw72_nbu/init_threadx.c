/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_component_mem_manager.h"

void ot_sys_init();
void create_ot_task();
void __real_tx_application_define(void *p);

void __wrap_tx_application_define(void *p)
{
    (void)p;

    MEM_Init();
    ot_sys_init();
    create_ot_task();

    RADIO_CTRL->RF_CLK_CTRL |= RADIO_CTRL_RF_CLK_CTRL_ZBLL_CLK_EN_OVRD(1);

    __real_tx_application_define(p);
}
