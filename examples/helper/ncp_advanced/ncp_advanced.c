/*
 *  Copyright (c) 2025, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include "fsl_component_serial_manager.h"
#include "ncp_serial_intf.h"
#include "w_uart_application.h"

#define BLE_CMD_PREFIX_SIZE 6

void                    __real_APP_InitServices();
void                    __real_OSA_ProcessTasks();
void                    __real_serial_rpmsg_tx(uint8_t *data, uint32_t len);
serial_manager_status_t __real_SerialManager_WriteBlocking(serial_write_handle_t wh, uint8_t *b, uint32_t l);
serial_manager_status_t __real_SerialManager_WriteNonBlocking(serial_write_handle_t wh, uint8_t *b, uint32_t l);

static void *uart_write_handle  = NULL;
static void *rpmsg_write_handle = NULL;

/* add init for OT side */
void __wrap_APP_InitServices()
{
    ncp_basic_init_1();

    __real_APP_InitServices();

    ncp_basic_init_2();

    uart_write_handle  = serial_uart_get_write_handle();
    rpmsg_write_handle = serial_rpmsg_get_write_handle();
}

/* add event processing for OT side */
void __wrap_OSA_ProcessTasks()
{
    ncp_process_events();

    __real_OSA_ProcessTasks();
}

void __wrap_serial_rpmsg_tx(uint8_t *data, uint32_t len)
{
    /* send data (OT command) over BLE too */
    BleApp_SendUartStream(data, len);

    __real_serial_rpmsg_tx(data, len);
}

static serial_manager_status_t write_ble_msg(uint8_t *data, uint32_t len)
{
    serial_uart_tx((uint8_t *)"\nble: ", BLE_CMD_PREFIX_SIZE);
    serial_uart_tx(data, len);
    serial_uart_tx((uint8_t *)"\n", 1);

    /* the BLE app frees the buffer if error is returned */
    return kStatus_SerialManager_Busy;
}

serial_manager_status_t __wrap_SerialManager_WriteBlocking(serial_write_handle_t wh, uint8_t *b, uint32_t l)
{
    if ((!uart_write_handle && !rpmsg_write_handle) || (uart_write_handle == wh) || (rpmsg_write_handle == wh))
    {
        return __real_SerialManager_WriteBlocking(wh, b, l);
    }

    return write_ble_msg(b, l);
}

serial_manager_status_t __wrap_SerialManager_WriteNonBlocking(serial_write_handle_t wh, uint8_t *b, uint32_t l)
{
    if ((!uart_write_handle && !rpmsg_write_handle) || (uart_write_handle == wh) || (rpmsg_write_handle == wh))
    {
        return __real_SerialManager_WriteNonBlocking(wh, b, l);
    }

    return write_ble_msg(b, l);
}

void __wrap_Shell_Init()
{
    /* skip any SerialManager API calls from BLE side */
}

void __wrap_BOARD_InitSerialManager(void *p)
{
    (void)p;
    /* skip any SerialManager API calls from BLE side */
}

serial_manager_status_t __wrap_SerialManager_InstallTxCallback(serial_write_handle_t     wh,
                                                               serial_manager_callback_t cb,
                                                               void                     *p)
{
    (void)wh;
    (void)cb;
    (void)p;
    /* skip any SerialManager API calls from BLE side */

    return kStatus_SerialManager_Success;
}

void ScanningTimerCallback(void *p)
{
    (void)p;
}
