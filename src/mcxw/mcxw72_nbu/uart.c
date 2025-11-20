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

#include "fsl_adapter_rpmsg.h"
#include "fsl_component_serial_manager.h"
#include "fsl_os_abstraction.h"

#include "openthread-system.h"
#include <utils/code_utils.h>
#include <openthread/platform/alarm-milli.h>
#include "utils/uart.h"

#include "platform-mcxw72_nbu.h"
#include "rpmsg_config.h"

#define SERIAL_MANAGER_RING_BUFFER_SIZE 512

#if (SERIAL_MANAGER_RING_BUFFER_SIZE < RL_BUFFER_PAYLOAD_SIZE)
#error "mcxw72_nbu serial rx buffer is to small"
#endif

static SERIAL_MANAGER_HANDLE_DEFINE(otCliSerialHandle);
static SERIAL_MANAGER_WRITE_HANDLE_DEFINE(otCliSerialWriteHandle);
static SERIAL_MANAGER_READ_HANDLE_DEFINE(otCliSerialReadHandle);

static bool_t otPlatUartEnabled = FALSE;
static bool_t sUartRxFired      = FALSE;

static uint8_t rxBuffer[SERIAL_MANAGER_RING_BUFFER_SIZE];
static uint8_t ring_buff[SERIAL_MANAGER_RING_BUFFER_SIZE];

static hal_rpmsg_config_t rpmsg_config = {
    .local_addr  = 23,
    .remote_addr = 13,
};

static const serial_manager_config_t config = {
    .type           = kSerialPort_Rpmsg,
    .ringBuffer     = ring_buff,
    .ringBufferSize = SERIAL_MANAGER_RING_BUFFER_SIZE,
    .portConfig     = &rpmsg_config,
};

static void Uart_RxCallBack(void *pData, serial_manager_callback_message_t *message, serial_manager_status_t status)
{
    if (!otPlatUartEnabled)
    {
        return;
    }

    OSA_InterruptDisable();
    if (!sUartRxFired)
    {
        sUartRxFired = TRUE;
        PWR_DisallowDeviceToSleep();
    }
    OSA_InterruptEnable();

    otSysEventSignalPending();
}

otError otPlatUartEnable()
{
    serial_manager_status_t status;
    otError                 error = OT_ERROR_NONE;

    otEXPECT_ACTION(!otPlatUartEnabled, );

    /* Init Serial Manager */
    status = SerialManager_Init(otCliSerialHandle, &config);
    otEXPECT_ACTION(status == kStatus_SerialManager_Success, error = OT_ERROR_FAILED);

    SerialManager_OpenReadHandle(otCliSerialHandle, otCliSerialReadHandle);
    SerialManager_OpenWriteHandle(otCliSerialHandle, otCliSerialWriteHandle);

    SerialManager_InstallRxCallback(otCliSerialReadHandle, Uart_RxCallBack, NULL);

    otPlatUartEnabled = TRUE;

exit:
    return error;
}

void otPlatUartProcess()
{
    uint32_t bytesRead = 0U;
    bool_t   read_data = FALSE;

    if (!otPlatUartEnabled)
    {
        return;
    }

    OSA_InterruptDisable();
    if (sUartRxFired)
    {
        read_data    = TRUE;
        sUartRxFired = FALSE;
        PWR_AllowDeviceToSleep();
    }
    OSA_InterruptEnable();

    if (read_data)
    {
        if ((SerialManager_TryRead(otCliSerialReadHandle, rxBuffer, sizeof(rxBuffer), &bytesRead) ==
             kStatus_SerialManager_Success) &&
            (bytesRead != 0))
        {
            otPlatUartReceived(rxBuffer, bytesRead);
        }
    }
}

otError otPlatUartDisable()
{
    return OT_ERROR_NONE;
}

otError otPlatUartSend(const uint8_t *aBuf, uint16_t aBufLength)
{
    otError error = OT_ERROR_NONE;

    otEXPECT_ACTION(otPlatUartEnabled, error = OT_ERROR_FAILED);

    /* avoid HAL_RpmsgSend() failure */
    while (aBufLength > RL_BUFFER_PAYLOAD_SIZE)
    {
        SerialManager_WriteBlocking(otCliSerialWriteHandle, (uint8_t *)aBuf, RL_BUFFER_PAYLOAD_SIZE);

        aBufLength -= RL_BUFFER_PAYLOAD_SIZE;
        aBuf += RL_BUFFER_PAYLOAD_SIZE;
    }

    if (aBufLength)
    {
        SerialManager_WriteBlocking(otCliSerialWriteHandle, (uint8_t *)aBuf, aBufLength);
    }

exit:
    otPlatUartSendDone();

    return error;
}

otError otPlatUartFlush()
{
    return OT_ERROR_NONE;
}

OT_TOOL_WEAK void otPlatUartSendDone()
{
}

OT_TOOL_WEAK void otPlatUartReceived(const uint8_t *aBuf, uint16_t aBufLength)
{
    OT_UNUSED_VARIABLE(aBuf);
    OT_UNUSED_VARIABLE(aBufLength);
}

#if (OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_PLATFORM_DEFINED)
void otPlatLogInit()
{
}
#endif /* (OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_PLATFORM_DEFINED) */
