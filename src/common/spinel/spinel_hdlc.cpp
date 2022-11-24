/*
 *  Copyright (c) 2021, The OpenThread Authors.
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

#include "spinel_hdlc.hpp"
#include "board.h"
#include "fsl_os_abstraction.h"
#include "fwk_platform_hdlc.h"

#include <openthread/tasklet.h>
#include <openthread/platform/alarm-milli.h>
#include "common/logging.hpp"

namespace ot {

namespace NXP {

HdlcInterface::HdlcInterface(ot::Spinel::SpinelInterface::ReceiveFrameCallback aCallback,
                             void *                                            aCallbackContext,
                             ot::Spinel::SpinelInterface::RxFrameBuffer &      aFrameBuffer)
    : encoderBuffer()
    , mHdlcEncoder(encoderBuffer)
    , hdlcRxCallbackField(nullptr)
    , mReceiveFrameBuffer(aFrameBuffer)
    , mReceiveFrameCallback(aCallback)
    , mReceiveFrameContext(aCallbackContext)
    , isInitialized(false)

{
}

HdlcInterface::~HdlcInterface(void)
{
}

void HdlcInterface::Init(void)
{
    int status;

    if (!isInitialized)
    {
        if (OSA_MutexCreate(writeMutexHandle) != KOSA_StatusSuccess)
        {
            assert(0);
        }

        /* Initialize the HDLC interface */
        status = PLATFORM_InitHdlcInterface(hdlcRxCallbackField, this);
        if (status == 0)
        {
            isInitialized = true;
        }
        else
        {
            otLogDebgPlat("Failed to initialize HDLC interface %d", status);
        }
    }
}

void HdlcInterface::Deinit(void)
{
    int status;

    status = PLATFORM_TerminateHdlcInterface();
    if (status == 0)
    {
        isInitialized = false;
    }
    else
    {
        otLogDebgPlat("Failed to terminate HDLC interface %d", status);
    }
}

otError HdlcInterface::SendFrame(const uint8_t *aFrame, uint16_t aLength)
{
    otError error = OT_ERROR_NONE;
    assert(encoderBuffer.IsEmpty());

    /* Protect concurrent Send operation to avoid any buffer corruption */
    if (OSA_MutexLock(writeMutexHandle, osaWaitForever_c) != KOSA_StatusSuccess)
    {
        error = OT_ERROR_FAILED;
        goto exit;
    }

    SuccessOrExit(error = mHdlcEncoder.BeginFrame());
    SuccessOrExit(error = mHdlcEncoder.Encode(aFrame, aLength));
    SuccessOrExit(error = mHdlcEncoder.EndFrame());
    otLogDebgPlat("frame len to send = %d/%d", encoderBuffer.GetLength(), aLength);
    SuccessOrExit(error = Write(encoderBuffer.GetFrame(), encoderBuffer.GetLength()));

exit:
    if (OSA_MutexUnlock(writeMutexHandle) != KOSA_StatusSuccess)
    {
        error = OT_ERROR_FAILED;
    }
    if (error != OT_ERROR_NONE)
    {
        OT_PLAT_ERR("error = 0x%x", error);
    }

    return error;
}

void HdlcInterface::Process(const otInstance &aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    TryReadAndDecode(false);
}

otError HdlcInterface::Write(const uint8_t *aFrame, uint16_t aLength)
{
    otError otResult = OT_ERROR_NONE;
    int     ret;

    otLogDebgPlat("Send tx frame len = %d", aLength);

    ret = PLATFORM_SendHdlcMessage((uint8_t *)aFrame, aLength);
    if (ret != 0)
    {
        otResult = OT_ERROR_FAILED;
        otLogCritPlat("Error send %d", ret);
    }

    /* Always clear the encoder */
    encoderBuffer.Clear();
    return otResult;
}

otError HdlcInterface::WaitForFrame(uint64_t aTimeoutUs)
{
    uint32_t now   = otPlatAlarmMilliGetNow();
    uint32_t start = now;
    otError  error = OT_ERROR_RESPONSE_TIMEOUT;
    do
    {
        /* Poll the frame receive flag */
        if (TryReadAndDecode(true) != 0)
        {
            error = OT_ERROR_NONE;
            break;
        }
        OSA_TimeDelay(200);
        now = otPlatAlarmMilliGetNow();
    } while ((now - start) < aTimeoutUs / 1000);
    return error;
}

} // namespace NXP

} // namespace ot
