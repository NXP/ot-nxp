/*
 *  Copyright 2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

/* -------------------------------------------------------------------------- */
/*                                Includes                                    */
/* -------------------------------------------------------------------------- */

#include "FreeRTOS.h"
#include "ncp_adapter.h"
#include "ot_ncp_host_app.h"
#include "ot_ncp_host_cli.h"
#include "task.h"

/* -------------------------------------------------------------------------- */
/*                              Constants                                     */
/* -------------------------------------------------------------------------- */

#define TASK_MAIN_PRIO (configMAX_PRIORITIES - 2)
#define TASK_MAIN_STACK_SIZE (configSTACK_DEPTH_TYPE)1024

/* -------------------------------------------------------------------------- */
/*                              Variable                                      */
/* -------------------------------------------------------------------------- */

TaskHandle_t task_main_task_handler = NULL;

/* -------------------------------------------------------------------------- */
/*                              Function Prototypes                           */
/* -------------------------------------------------------------------------- */

#if configAPPLICATION_ALLOCATED_HEAP
__attribute__((weak)) uint8_t __attribute__((section(".heap"))) ucHeap[configTOTAL_HEAP_SIZE];
#endif

/* -------------------------------------------------------------------------- */
/*                              Functions                                     */
/* -------------------------------------------------------------------------- */
void vApplicationIdleHook(void)
{
    return;
}
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    assert(0);
}

void vApplicationMallocFailedHook(void)
{
    assert(0);
}

void ot_host_task(void *param)
{
    int32_t result = 0;

    result = ot_ncp_host_app_init();
    if (result != 0)
    {
        return;
    }

    result = ncp_adapter_init();
    if (result != 0)
    {
        return;
    }

    result = ot_ncp_host_cli_init();
    if (result != 0)
    {
        return;
    }

    while (1)
    {
        vTaskDelay(1);
    }
}

void appOtStart(int argc, char *argv[])
{
    BaseType_t result = 0;

    /* Print first input tips '> ' */
    PRINTF("\n >>> OT NCP Host Demo <<< \n\n> ");

    result = xTaskCreate(ot_host_task, "main", TASK_MAIN_STACK_SIZE, NULL, TASK_MAIN_PRIO, &task_main_task_handler);
    if (result != pdPASS)
    {
        return;
    }

    vTaskStartScheduler();
    for (;;)
        ;
}
