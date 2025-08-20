/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_component_mem_manager.h"
#include "fsl_os_abstraction.h"

void create_ot_task();

int main()
{
    OSA_Init();
    MEM_Init();

#ifndef FSL_RTOS_THREADX
    create_ot_task();
#endif

    OSA_Start();

    /* Won't run here */
    return 0;
}
