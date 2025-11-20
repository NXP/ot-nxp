/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_component_mem_manager.h"

void plat_init_mpu();
void create_ot_task();
void __real_tx_application_define(void *p);
void __real_fsciBleRegister(uint32_t t);

void __wrap_tx_application_define(void *p)
{
    plat_init_mpu();

    __real_tx_application_define(p);
}

void __wrap_fsciBleRegister(uint32_t t)
{
    RADIO_CTRL->RF_CLK_CTRL |= RADIO_CTRL_RF_CLK_CTRL_ZBLL_CLK_EN_OVRD(1);

    create_ot_task();

    __real_fsciBleRegister(t);
}
