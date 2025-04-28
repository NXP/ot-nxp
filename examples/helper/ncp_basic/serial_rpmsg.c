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

#include "EmbeddedTypes.h"

#include "fsl_component_serial_manager.h"
#include "fsl_os_abstraction.h"
#include "ncp_serial_intf.h"
#include "rpmsg_config.h"

#define SERIAL_MANAGER_RING_BUFFER_SIZE 2048

#if (SERIAL_MANAGER_RING_BUFFER_SIZE < RL_BUFFER_PAYLOAD_SIZE)
#error "ncp_basic serial RPMSG rx buffer is to small"
#endif

static SERIAL_MANAGER_HANDLE_DEFINE(serial_handle);
static SERIAL_MANAGER_WRITE_HANDLE_DEFINE(serial_read_handle);
static SERIAL_MANAGER_WRITE_HANDLE_DEFINE(serial_write_handle);

static bool_t serial_init     = FALSE;
static bool_t rx_data_pending = FALSE;

static uint8_t rx_buff[SERIAL_MANAGER_RING_BUFFER_SIZE];
static uint8_t ring_buff[SERIAL_MANAGER_RING_BUFFER_SIZE];

static hal_rpmsg_config_t rpmsg_config = {
    .local_addr  = 13,
    .remote_addr = 23,
};

static const serial_manager_config_t config = {
    .type           = kSerialPort_Rpmsg,
    .ringBuffer     = ring_buff,
    .ringBufferSize = SERIAL_MANAGER_RING_BUFFER_SIZE,
    .portConfig     = &rpmsg_config,
};

static void rx_cb(void *pData, serial_manager_callback_message_t *message, serial_manager_status_t status)
{
    OSA_InterruptDisable();
    if (!rx_data_pending)
    {
        rx_data_pending = TRUE;
        // PWR_DisallowDeviceToSleep();
    }
    OSA_InterruptEnable();
}

void serial_rpmsg_init()
{
    if (serial_init)
    {
        return;
    }

    /* Init Serial Manager */
    SerialManager_Init(serial_handle, &config);

    SerialManager_OpenReadHandle(serial_handle, serial_read_handle);
    SerialManager_OpenWriteHandle(serial_handle, serial_write_handle);

    SerialManager_InstallRxCallback(serial_read_handle, rx_cb, NULL);

    serial_init = TRUE;
}

void serial_rpmsg_process()
{
    uint32_t bytes_read = 0U;
    bool_t   read_data  = FALSE;

    if (!serial_init)
    {
        return;
    }

    OSA_InterruptDisable();
    if (rx_data_pending)
    {
        read_data       = TRUE;
        rx_data_pending = FALSE;
        // PWR_AllowDeviceToSleep();
    }
    OSA_InterruptEnable();

    if (read_data)
    {
        if ((SerialManager_TryRead(serial_read_handle, rx_buff, sizeof(rx_buff), &bytes_read) ==
             kStatus_SerialManager_Success) &&
            (bytes_read != 0))
        {
            plat_cmd_process(rx_buff, bytes_read);
        }
    }
}

void serial_rpmsg_tx(uint8_t *data, uint32_t len)
{
    if (!serial_init || !data || !len)
    {
        return;
    }

    /* avoid HAL_RpmsgSend() failure */
    while (len > RL_BUFFER_PAYLOAD_SIZE)
    {
        SerialManager_WriteBlocking(serial_write_handle, data, RL_BUFFER_PAYLOAD_SIZE);

        len -= RL_BUFFER_PAYLOAD_SIZE;
        data += RL_BUFFER_PAYLOAD_SIZE;
    }

    if (len)
    {
        SerialManager_WriteBlocking(serial_write_handle, data, len);
    }
}
