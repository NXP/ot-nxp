/*
 *  Copyright (c) 2021, The OpenThread Authors.
 *  Copyright (c) 2022, NXP.
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

/**
 * @file
 *   This file implements the OpenThread platform abstraction for UART communication.
 *
 */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include "board.h"
#include "fsl_component_serial_manager.h"
#include "fsl_os_abstraction.h"

#include "openthread-system.h"
#include <utils/code_utils.h>
#include <utils/uart.h>
#include <openthread/tasklet.h>

/* -------------------------------------------------------------------------- */
/*                               Private macros                               */
/* -------------------------------------------------------------------------- */

#ifndef OT_PLAT_UART_INSTANCE
#define OT_PLAT_UART_INSTANCE 1
#endif
#ifndef OT_PLAT_UART_CLK_FREQ
#define OT_PLAT_UART_CLK_FREQ BOARD_BT_UART_CLK_FREQ
#endif
#ifndef OT_PLAT_UART_BAUDRATE
#define OT_PLAT_UART_BAUDRATE 115200
#endif
#ifndef OT_PLAT_UART_TYPE
#define OT_PLAT_UART_TYPE (kSerialPort_Uart)
#endif
#ifndef OT_PLAT_UART_SERIAL_MANAGER_RING_BUFFER_SIZE
#define OT_PLAT_UART_SERIAL_MANAGER_RING_BUFFER_SIZE (128U)
#endif
#ifndef OT_PLAT_UART_RECEIVE_BUFFER_SIZE
#define OT_PLAT_UART_RECEIVE_BUFFER_SIZE (128)
#endif
#ifndef OT_PLAT_UART_FLUSH_DELAY_MS
#define OT_PLAT_UART_FLUSH_DELAY_MS 2U
#endif

#ifdef CPU_RW610EVA0IK
#define BOARD_APP_UART_FRG_CLK \
    (&(const clock_frg_clk_config_t){3, kCLOCK_FrgPllDiv, 255, 0}) /*!< Select FRG3 mux as frg_pll */
#define BOARD_APP_UART_CLK_ATTACH kFRG_to_FLEXCOMM3
#endif

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

static void Uart_RxCallBack(void *pData, serial_manager_callback_message_t *message, serial_manager_status_t status);
static void Uart_TxCallBack(void *pBuffer, serial_manager_callback_message_t *message, serial_manager_status_t status);

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

static SERIAL_MANAGER_HANDLE_DEFINE(otCliSerialHandle);
static SERIAL_MANAGER_WRITE_HANDLE_DEFINE(otCliSerialWriteHandle);
static SERIAL_MANAGER_READ_HANDLE_DEFINE(otCliSerialReadHandle);
static bool         otPlatUartEnabled = false;
static volatile int txDone            = 0;
static volatile int txCount           = 0;

uint8_t                          rxBuffer[OT_PLAT_UART_RECEIVE_BUFFER_SIZE];
static serial_port_uart_config_t uartConfig = {.instance     = OT_PLAT_UART_INSTANCE,
                                               .baudRate     = OT_PLAT_UART_BAUDRATE,
                                               .parityMode   = kSerialManager_UartParityDisabled,
                                               .stopBitCount = kSerialManager_UartOneStopBit,
                                               .enableRx     = 1,
                                               .enableTx     = 1,
                                               .enableRxRTS  = 0,
                                               .enableTxCTS  = 0};

static uint8_t                       s_ringBuffer[OT_PLAT_UART_SERIAL_MANAGER_RING_BUFFER_SIZE];
static const serial_manager_config_t s_serialManagerConfig = {
    .type           = OT_PLAT_UART_TYPE,
    .ringBuffer     = &s_ringBuffer[0],
    .ringBufferSize = OT_PLAT_UART_SERIAL_MANAGER_RING_BUFFER_SIZE,
    .blockType      = kSerialManager_NonBlocking,
    .portConfig     = (serial_port_uart_config_t *)&uartConfig,
};

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void otPlatUartSetInstance(uint8_t newInstance)
{
    uartConfig.instance = newInstance;
}

otError otPlatUartEnable(void)
{
    otError error        = OT_ERROR_FAILED;
    uartConfig.clockRate = OT_PLAT_UART_CLK_FREQ;
    do
    {
        if (SerialManager_Init((serial_handle_t)otCliSerialHandle, &s_serialManagerConfig) !=
            kStatus_SerialManager_Success)
            break;
        if (SerialManager_OpenWriteHandle((serial_handle_t)otCliSerialHandle,
                                          (serial_write_handle_t)otCliSerialWriteHandle) !=
            kStatus_SerialManager_Success)
            break;
        if (SerialManager_OpenReadHandle((serial_handle_t)otCliSerialHandle,
                                         (serial_read_handle_t)otCliSerialReadHandle) != kStatus_SerialManager_Success)
            break;
        if (SerialManager_InstallRxCallback((serial_read_handle_t)otCliSerialReadHandle, Uart_RxCallBack, NULL) !=
            kStatus_SerialManager_Success)
            break;
        if (SerialManager_InstallTxCallback((serial_write_handle_t)otCliSerialWriteHandle, Uart_TxCallBack, NULL) !=
            kStatus_SerialManager_Success)
            break;
        otPlatUartEnabled = true;
        error             = OT_ERROR_NONE;
    } while (0);
    return error;
}

otError otPlatUartDisable(void)
{
    otError error = OT_ERROR_FAILED;
    do
    {
        if (SerialManager_CloseWriteHandle((serial_write_handle_t)otCliSerialWriteHandle) !=
            kStatus_SerialManager_Success)
            break;
        if (SerialManager_CloseReadHandle((serial_read_handle_t)otCliSerialReadHandle) != kStatus_SerialManager_Success)
            break;
        if (SerialManager_Deinit((serial_handle_t)otCliSerialHandle) != kStatus_SerialManager_Success)
            break;
        otPlatUartEnabled = false;
        error             = OT_ERROR_NONE;
    } while (0);
    return error;
}

otError otPlatUartSend(const uint8_t *aBuf, uint16_t aBufLength)
{
    otError error = OT_ERROR_NONE;
    if (otPlatUartEnabled)
    {
        /* Track the ongoing TX count for otPlatUartFlush */
        txCount++;
        SerialManager_WriteNonBlocking((serial_write_handle_t)otCliSerialWriteHandle, (uint8_t *)aBuf, aBufLength);
    }
    else
    {
        error = OT_ERROR_NOT_CAPABLE;
    }

    return error;
}

otError otPlatUartFlush(void)
{
    uint32_t intMask;

    while (txCount)
    {
#if USE_RTOS || !defined(OSA_USED)
        /* Wait for the serial manager task to empty the TX buffer */
        OSA_TimeDelay(OT_PLAT_UART_FLUSH_DELAY_MS);
#else
#error On baremetal system, SerialManager TX callback must be called from IRQ context (OSA_USED not defined)
#endif /* USE_RTOS || !defined(OSA_USED) */

        intMask = DisableGlobalIRQ();
        while (txDone)
        {
            txDone--;
            txCount--;
        }
        EnableGlobalIRQ(intMask);
    }

    return OT_ERROR_NONE;
}

void otPlatCliUartProcess(void)
{
    uint32_t bytesRead = 0U;
    uint32_t intMask;

    if ((otPlatUartEnabled) &&
        (SerialManager_TryRead((serial_read_handle_t)otCliSerialReadHandle, rxBuffer, OT_PLAT_UART_RECEIVE_BUFFER_SIZE,
                               &bytesRead) == kStatus_SerialManager_Success) &&
        (bytesRead != 0))
    {
        otPlatUartReceived(rxBuffer, bytesRead);
    }

    intMask = DisableGlobalIRQ();
    while (txDone)
    {
        txDone--;
        txCount--;
        otPlatUartSendDone();
    }
    EnableGlobalIRQ(intMask);
}

/**
 * Week function in case the ot_cli is disabled
 *
 */
OT_TOOL_WEAK void otPlatUartSendDone(void)
{
}

OT_TOOL_WEAK void otPlatUartReceived(const uint8_t *aBuf, uint16_t aBufLength)
{
    OT_UNUSED_VARIABLE(aBuf);
    OT_UNUSED_VARIABLE(aBufLength);
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

static void Uart_RxCallBack(void *pData, serial_manager_callback_message_t *message, serial_manager_status_t status)
{
    /* notify the main loop that a RX buffer is available */
    otSysEventSignalPending();
}

static void Uart_TxCallBack(void *pBuffer, serial_manager_callback_message_t *message, serial_manager_status_t status)
{
    uint32_t intMask;

    assert(txCount > 0);

    intMask = DisableGlobalIRQ();
    txDone++;
    EnableGlobalIRQ(intMask);

    /* notify the main loop that the TX is done */
    otSysEventSignalPending();
}
