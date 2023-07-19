/*
 *  Copyright (c) 2022, The OpenThread Authors.
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

#include "spinel_hci_hdlc.hpp"
#include <openthread/tasklet.h>
#include "common/logging.hpp"
#include "lib/spinel/spinel.h"

/*
 *   This class allows to support hci and spinel on a single UART.
 *   For that the HDLC-Lite framing protocol is used to transfert spinel and hci frames
 *   In addition, HCI and spinel frames can be distinguished by using the Spinel convention which is
 *   line compatible with BTBLE HCI. The first two bit of the spinel header are checked (HCI frames, which always start
 *   with either "0x01"or "0x04").
 *
 */

/*********
 * NXP hook to support HCI/SPINEL over HDLC on a single UART
 *********/

extern void         otPlatRadioSendFrameToSpinelInterface(uint8_t *buf, uint16_t length);
extern "C" uint16_t hci_transport_enqueue(uint8_t *packet, uint16_t packet_len, uint16_t *enqueued_len);

/*
 * Allows to wrap the function hci_uart_write_data.
 * In this case the hci frame will be encapsulated in a hdlc frame (a call to otPlatRadioSendFrameToSpinelInterface
 * allows to do it).
 */
extern "C" void __wrap_hci_uart_write_data(uint8_t *buf, uint16_t length)
{
    otPlatRadioSendFrameToSpinelInterface(buf, length);
}

/*
 * Wrap the function hci_uart_bt_init with an empty content.
 * In this case we assume that spinel is in charge of managing the UART interface
 */
extern "C" void __wrap_hci_uart_bt_init(void)
{
}

/*
 * Wrap the function hci_uart_bt_shutdown with an empty content.
 * In this case we assume that spinel is in charge of managing the UART interface
 */
extern "C" void __wrap_hci_uart_bt_shutdown(void)
{
}

/*
 * Wrap the function hci_uart_init, the goal of the new implemetation is to set the value of hci_uart_state to
 * "initilized", so that then a call to hci_transport_enqueue can be done.
 *
 */
extern "C" uint8_t hci_uart_state;
extern "C" void    __wrap_hci_uart_init(void)
{
    hci_uart_state = 0x1U;
}

/*********
 * End - NXP hook to support HCI/SPINEL over HDLC on a single UART
 *********/

namespace ot {

namespace RT {

void HdlcSpinelHciInterface::Init(void)
{
    if (!isInitialized)
    {
        mutexHandle = xSemaphoreCreateMutex();
        msgQueue    = xQueueCreate(1, sizeof(uint8_t));
        assert(mutexHandle != NULL && msgQueue != NULL);
    }
    HdlcInterface::Init();
}

void HdlcSpinelHciInterface::HandleHdlcFrame(otError aError)
{
    uint8_t *buf          = RxHciSpinelFrameBuffer.GetFrame();
    uint16_t bufLength    = RxHciSpinelFrameBuffer.GetLength();
    uint16_t enqueued_len = 0;

    otDumpDebgPlat("RX FRAME", buf, bufLength);

    if (aError == OT_ERROR_NONE && bufLength > 0)
    {
        /* Is it a spinel frame ? */
        if ((buf[0] & SPINEL_HEADER_FLAG) == SPINEL_HEADER_FLAG)
        {
            otLogDebgPlat("Frame correctly received %d", bufLength);
            /* Save the frame */
            RxHciSpinelFrameBuffer.SaveFrame();
            /* Send a signal to the openthread task to indicate that a spinel data is pending */
            otTaskletsSignalPending(NULL);
        }
        else
        {
            otLogDebgPlat("Frame dropped %d", bufLength);
            /* Transfert the frame to the HCI decoder */
            hci_transport_enqueue(buf, bufLength, &enqueued_len);
            assert(enqueued_len == bufLength);
            RxHciSpinelFrameBuffer.DiscardFrame();
        }
    }
    else
    {
        otLogCritPlat("Frame will be discarded error = 0x%x", aError);
        RxHciSpinelFrameBuffer.DiscardFrame();
    }
}

void HdlcSpinelHciInterface::HdlcHciSerialManagerRxCallback(void *                             callbackParam,
                                                            serial_manager_callback_message_t *message,
                                                            serial_manager_status_t            status)
{
    static_cast<HdlcSpinelHciInterface *>(callbackParam)->ProcessSerialManagerRxData();
}

void HdlcSpinelHciInterface::ProcessSerialManagerRxData(void)
{
    uint8_t                 mUartRxBuffer[256];
    uint32_t                bytesRead = 0;
    uint8_t                 event;
    uint32_t                remainingRxBufferSize = 0;
    serial_manager_status_t smStatus;
    xSemaphoreTake(mutexHandle, portMAX_DELAY);

    do
    {
        /* Check if we have enough space to store the frame in the buffer */
        remainingRxBufferSize = RxHciSpinelFrameBuffer.GetFrameMaxLength() - RxHciSpinelFrameBuffer.GetLength();
        otLogDebgPlat("remainingRxBufferSize = %d", remainingRxBufferSize);
        if (remainingRxBufferSize >= sizeof(mUartRxBuffer))
        {
            OSA_InterruptDisable();
            smStatus = SerialManager_TryRead((serial_read_handle_t)otTransceiverSerialReadHandle, mUartRxBuffer,
                                             sizeof(mUartRxBuffer), &bytesRead);
            OSA_InterruptEnable();
            if (bytesRead > 0 && smStatus == kStatus_SerialManager_Success)
            {
                // otDumpDebgPlat("Serial", mUartRxBuffer, bytesRead);
                mHdlcHciSpinelDecoder.Decode(mUartRxBuffer, bytesRead);
            }
            else
                break;
        }
        else
        {
            spinelBufferFull = true;
            otLogDebgPlat("Spinel buffer full remainingRxLen = %d", remainingRxBufferSize);
            /* Send a signal to the openthread task to indicate to empty the spinel buffer */
            otTaskletsSignalPending(NULL);
            /* Give the mutex */
            xSemaphoreGive(mutexHandle);
            /* Look the task here until the spinel buffer becomes empty */
            xQueueReceive(msgQueue, &event, portMAX_DELAY);
            /* take the mutex again */
            xSemaphoreTake(mutexHandle, portMAX_DELAY);
        }

    } while (bytesRead != 0);

    xSemaphoreGive(mutexHandle);
}

uint32_t HdlcSpinelHciInterface::TryReadAndDecode(bool fullRead)
{
    uint32_t totalBytesRead  = 0;
    uint32_t i               = 0;
    uint8_t  event           = 1;
    uint8_t *oldFrame        = savedFrame;
    uint16_t oldLen          = savedFrameLen;
    otError  savedFrameFound = OT_ERROR_NONE;

    xSemaphoreTake(mutexHandle, portMAX_DELAY);

    savedFrameFound = RxHciSpinelFrameBuffer.GetNextSavedFrame(savedFrame, savedFrameLen);

    while (savedFrameFound == OT_ERROR_NONE)
    {
        /* Copy the data to the ot frame buffer */
        for (i = 0; i < savedFrameLen; i++)
        {
            if (mReceiveFrameBuffer.WriteByte(savedFrame[i]) != OT_ERROR_NONE)
            {
                mReceiveFrameBuffer.UndoLastWrites(i);
                /* No more space restore the savedFrame to the previous frame */
                savedFrame    = oldFrame;
                savedFrameLen = oldLen;
                /* Signal the ot task to re-try later */
                otTaskletsSignalPending(NULL);
                otLogDebgPlat("No more space");
                xSemaphoreGive(mutexHandle);
                return totalBytesRead;
            }
            totalBytesRead++;
        }
        otLogDebgPlat("Frame len %d consumed", savedFrameLen);
        mReceiveFrameCallback(mReceiveFrameContext);
        oldFrame        = savedFrame;
        oldLen          = savedFrameLen;
        savedFrameFound = RxHciSpinelFrameBuffer.GetNextSavedFrame(savedFrame, savedFrameLen);
    }

    if (savedFrameFound != OT_ERROR_NONE)
    {
        /* No more frame saved clear the buffer */
        RxHciSpinelFrameBuffer.ClearSavedFrames();
        /* If the spinel queue was locked */
        if (spinelBufferFull)
        {
            spinelBufferFull = false;
            /* Send an event to unlock the task */
            xQueueOverwrite(msgQueue, (void *)&event);
        }
    }
    xSemaphoreGive(mutexHandle);
    return totalBytesRead;
}

void HdlcSpinelHciInterface::HandleHdlcFrame(void *aContext, otError aError)
{
    static_cast<HdlcSpinelHciInterface *>(aContext)->HandleHdlcFrame(aError);
}

} // namespace RT

} // namespace ot
