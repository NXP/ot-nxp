/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app.h"
#include "app_conn.h"
#include "fsl_os_abstraction.h"

#include <openthread-system.h>
#include "openthread/cli.h"
#include "openthread/tasklet.h"
#include "openthread/thread.h"

void OSA_TimeInit();

void otAppCliInit(otInstance *aInstance);

void *otPlatUartGetSerialHandle();
void *otPlatUartGetSerialReadHandle();

void *gSerMgrIf;

static uint32_t *ot_ser_handle;

static otInstance *gpOpenThreadInstance = NULL;

static void App_Thread()
{
    while (1)
    {
        otSysProcessDrivers(gpOpenThreadInstance);

        while (otTaskletsArePending(gpOpenThreadInstance))
        {
            otTaskletsProcess(gpOpenThreadInstance);
            otSysProcessDrivers(gpOpenThreadInstance);
        }

        /* For BareMetal break the while(1) after 1 run */
        if (gUseRtos_c == 0)
        {
            break;
        }
    }
}

void main()
{
    OSA_Init();

    /* Initialize OpenThread timer, RNG and Radio */
    otSysInit(0, NULL);
    gpOpenThreadInstance = otInstanceInitSingle();

    /* otAppCliInit() enables UART */
    otAppCliInit(gpOpenThreadInstance);

    gSerMgrIf     = otPlatUartGetSerialHandle();
    ot_ser_handle = otPlatUartGetSerialReadHandle();

    /* Start BLE Platform related resources such as clocks, Link layer and HCI transport to Link Layer */
    (void)APP_InitBle();

    /* Example of baremetal loop if user doesn't want to use OSA API */
#if (FSL_OSA_BM_TIMER_CONFIG != FSL_OSA_BM_TIMER_NONE)
    OSA_TimeInit();
#endif

    /* Start Host stack */
    BluetoothLEHost_AppInit();

    otCliOutputFormat("\r\n Welcome to OT_cli_ftd + BLE_loc_reader host dual app \r\n");

    while (TRUE)
    {
        OSA_ProcessTasks();
        BluetoothLEHost_HandleMessages();

        /* Before executing WFI, need to execute some connectivity background tasks
            (usually done in Idle thread) such as NVM save in Idle, etc.. */
        BluetoothLEHost_ProcessIdleTask();

        /* OT task */
        App_Thread();
    }

    /* Won't run here */
    return;
}

#define BLE_CMD_PREFIX_SIZE 5
#define BLE_CMD_SIZE SHELL_BUFFER_SIZE

uint8_t  ble_cmd[BLE_CMD_SIZE];
uint32_t ble_cmd_len;
uint32_t ble_cmd_pos;

static serial_manager_callback_t cust_rx_cb;
static void                     *cust_rx_param;

void                    __real_otCliInputLine(char *aBuf);
serial_manager_status_t __real_SerialManager_TryRead(serial_read_handle_t readHandle,
                                                     uint8_t             *buffer,
                                                     uint32_t             length,
                                                     uint32_t            *receivedLength);
serial_manager_status_t __real_SerialManager_InstallRxCallback(serial_read_handle_t      readHandle,
                                                               serial_manager_callback_t callback,
                                                               void                     *callbackParam);

void __wrap_otCliInputLine(char *aBuf)
{
    if (strncmp(aBuf, "ble: ", BLE_CMD_PREFIX_SIZE) == 0)
    {
        uint32_t len = strlen(aBuf);

        if (len - BLE_CMD_PREFIX_SIZE + 1 <= BLE_CMD_SIZE)
        {
            memcpy(ble_cmd, aBuf + BLE_CMD_PREFIX_SIZE, len - BLE_CMD_PREFIX_SIZE);

            ble_cmd[len - BLE_CMD_PREFIX_SIZE] = '\n';

            ble_cmd_pos = 0;
            ble_cmd_len = len - BLE_CMD_PREFIX_SIZE + 1;

            if (cust_rx_cb)
            {
                cust_rx_cb(cust_rx_param, cust_rx_param, 0); /* calls SHELL_Task() */
            }
        }
        return;
    }

    __real_otCliInputLine(aBuf);
}

serial_manager_status_t __wrap_SerialManager_TryRead(serial_read_handle_t readHandle,
                                                     uint8_t             *buffer,
                                                     uint32_t             length,
                                                     uint32_t            *receivedLength)
{
    if (!ot_ser_handle || (ot_ser_handle == (uint32_t *)readHandle))
    {
        return __real_SerialManager_TryRead(readHandle, buffer, length, receivedLength);
    }

    /* SHELL_Task() => SHELL_GetChar() => SerialManager_TryRead() in a loop.
       SHELL_GetChar() requests just 1 byte */
    if (!buffer || (length != 1) || !receivedLength)
    {
        return kStatus_SerialManager_Error;
    }

    if (ble_cmd_pos >= ble_cmd_len)
    {
        return kStatus_SerialManager_Error;
    }

    buffer[0] = ble_cmd[ble_cmd_pos];

    /* avoid endless loop */
    ble_cmd_pos++;

    *receivedLength = 1;

    return kStatus_SerialManager_Success;
}

serial_manager_status_t __wrap_SerialManager_InstallRxCallback(serial_read_handle_t      readHandle,
                                                               serial_manager_callback_t callback,
                                                               void                     *callbackParam)
{
    if (!ot_ser_handle || (ot_ser_handle == (uint32_t *)readHandle))
    {
        return __real_SerialManager_InstallRxCallback(readHandle, callback, callbackParam);
    }

    cust_rx_cb    = callback;
    cust_rx_param = callbackParam;

    return 0;
}
