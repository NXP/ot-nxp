/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_component_mem_manager.h"
#include "fsl_os_abstraction.h"
#include <openthread-system.h>
#include "openthread/tasklet.h"

#undef OT_TASK_STACK_SIZE
#define OT_TASK_STACK_SIZE 3712

void otAppCliInit(otInstance *aInstance);

static void ot_task();

static otInstance *ot_instance       = NULL;
static bool        ot_task_init_done = false;

static OSA_SEMAPHORE_HANDLE_DEFINE(ot_sem);
static OSA_TASK_HANDLE_DEFINE(ot_task_handle);
static OSA_TASK_DEFINE(ot_task, gMainThreadPriority_c + 1, 1, OT_TASK_STACK_SIZE, 0);

void ot_sys_init()
{
    if (!ot_task_init_done)
    {
        /* Initialize OpenThread timer, RNG and Radio */
        otSysInit(1, NULL);

        ot_instance = otInstanceInitSingle();

        /* otAppCliInit() enables UART */
        otAppCliInit(ot_instance);

        OSA_SemaphoreCreate((osa_semaphore_handle_t)ot_sem, 1);

        ot_task_init_done = true;
    }
}

static void ot_task()
{
    ot_sys_init();

    while (1)
    {
        if (OSA_SemaphoreWait((osa_semaphore_handle_t)ot_sem, osaWaitForever_c) == KOSA_StatusSuccess)
        {
            otSysProcessDrivers(ot_instance);

            while (otTaskletsArePending(ot_instance))
            {
                otTaskletsProcess(ot_instance);
                otSysProcessDrivers(ot_instance);
            }
        }

#if !USE_RTOS
        break;
#endif
    }
}

void create_ot_task()
{
    OSA_TaskCreate(ot_task_handle, OSA_TASK(ot_task), NULL);
}

void otSysEventSignalPending()
{
    if (ot_task_init_done)
    {
        OSA_SemaphorePost((osa_semaphore_handle_t)ot_sem);
    }
}

void otTaskletsSignalPending(otInstance *aInstance)
{
    (void)aInstance;
    otSysEventSignalPending();
}
