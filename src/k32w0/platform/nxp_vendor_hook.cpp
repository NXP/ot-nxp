/*
 *    Copyright (c) 2016-2022, The OpenThread Authors.
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 *    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file allows to support HCI and spinel on a single UART
 *   Both HCI and spinel are using hdlc to encapsulate spinel or hci frame
 */

#if OPENTHREAD_ENABLE_NCP_VENDOR_HOOK

#include "ble_general.h"
#include "fsl_debug_console.h"
#include "ncp_base.hpp"
#include "ncp_hdlc.hpp"
#include "../examples/platforms/utils/uart.h"
#include "common/new.hpp"
#include "lib/spinel/spinel.h"

/*
 * Spinel header format:
 *
 * 0   1   2   3   4   5   6   7
 * +---+---+---+---+---+---+---+---+
 * |  FLG  |  NLI  |      TID      |
 * +---+---+---+---+---+---+---+---+
 *
 * The Flag (FLG) field in the two most significant bits of the header byte (FLG) is always set to the value two (or 10
 * in binary).
 */
#define SPINEL_HEADER_FLAG_MASK (3 << 6)

#define OT_NXP_SPINEL_PROP_VENDOR_BLE_STATE SPINEL_PROP_VENDOR__BEGIN

extern "C" void hci_processReceivedChar(uint8_t recvChar);
extern "C" void BleAppInactivityCallback(uint32_t inactive_time);

namespace ot {
namespace Ncp {

class NcpNxpVendorUart : public NcpHdlc
{
    static int SendHdlcHook(const uint8_t *aBuf, uint16_t aBufLength);

public:
    explicit NcpNxpVendorUart(Instance *aInstance);

    void HandleHdlcReceiveDoneHook(const uint8_t *aBuf, uint16_t aBufLength);
    bool GenerateHdlcMessageAndSent(uint8_t hciPacketType, void *pHciPacket, uint16_t hciPacketLength);

private:
    enum
    {
        kRxBufferSize = OPENTHREAD_CONFIG_NCP_HDLC_RX_BUFFER_SIZE + // Rx buffer size (should be large enough to fit
                        OPENTHREAD_CONFIG_NCP_SPINEL_ENCRYPTER_EXTRA_DATA_SIZE, // one whole (decoded) received frame).
    };

    ot::Hdlc::Decoder                    mFrameDecoderHook;
    ot::Hdlc::FrameBuffer<kRxBufferSize> mRxBufferHook;

    static void HandleFrameHook(void *aContext, otError aError);

    void HandleFrameHook(otError aError);
};

extern "C" void __wrap_otNcpHdlcReceive(const uint8_t *aBuf, uint16_t aBufLength)
{
    NcpNxpVendorUart *ncpHdlc = static_cast<NcpNxpVendorUart *>(NcpBase::GetNcpInstance());

    if (ncpHdlc != nullptr)
    {
        ncpHdlc->HandleHdlcReceiveDoneHook(aBuf, aBufLength);
    }
}

extern "C" bool otGenerateHdlcMessageAndSent(uint8_t hciPacketType, void *pHciPacket, uint16_t hciPacketLength)
{
    NcpNxpVendorUart *ncpHdlc = static_cast<NcpNxpVendorUart *>(NcpBase::GetNcpInstance());

    if (ncpHdlc != nullptr)
    {
        return ncpHdlc->GenerateHdlcMessageAndSent(hciPacketType, pHciPacket, hciPacketLength);
    }
    return false;
}

extern "C" bleResult_t __wrap_Hcit_SendPacket(hciPacketType_t packetType, void *pHciPacket, uint16_t hciPacketLength)
{
    bleResult_t bleResult = gBleSuccess_c;
    bool        result    = otGenerateHdlcMessageAndSent((uint8_t)packetType, pHciPacket, hciPacketLength);

    if (!result)
    {
        bleResult = gHciTransportError_c;
    }
    return bleResult;
}

bool NcpNxpVendorUart::GenerateHdlcMessageAndSent(uint8_t hciPacketType, void *pHciPacket, uint16_t hciPacketLength)
{
    bool result = false;
    do
    {
        if (mEncoder.BeginFrame((Spinel::Buffer::Priority)0) != OT_ERROR_NONE)
            break;
        /* Write the hciPacketType */
        if (mEncoder.WriteUint8(hciPacketType) != OT_ERROR_NONE)
            break;
        if (mEncoder.WriteData((const uint8_t *)pHciPacket, hciPacketLength) != OT_ERROR_NONE)
            break;
        if (mEncoder.EndFrame() != OT_ERROR_NONE)
            break;
        result = true;

    } while (0);

    return result;
}

NcpNxpVendorUart::NcpNxpVendorUart(ot::Instance *aInstance)
    : NcpHdlc(aInstance, &NcpNxpVendorUart::SendHdlcHook)
    , mFrameDecoderHook(mRxBufferHook, &NcpNxpVendorUart::HandleFrameHook, this)
{
}

int NcpNxpVendorUart::SendHdlcHook(const uint8_t *aBuf, uint16_t aBufLength)
{
    IgnoreError(otPlatUartSend(aBuf, aBufLength));
    return aBufLength;
}

void NcpNxpVendorUart::HandleHdlcReceiveDoneHook(const uint8_t *aBuf, uint16_t aBufLength)
{
    mFrameDecoderHook.Decode(aBuf, aBufLength);
}

void NcpNxpVendorUart::HandleFrameHook(void *aContext, otError aError)
{
    static_cast<NcpNxpVendorUart *>(aContext)->HandleFrameHook(aError);
}

void NcpNxpVendorUart::HandleFrameHook(otError aError)
{
    OT_UNUSED_VARIABLE(aError);
    uint8_t *buf       = mRxBufferHook.GetFrame();
    uint16_t bufLength = mRxBufferHook.GetLength();

    if (aError == OT_ERROR_NONE && bufLength > 0)
    {
        // Check if it is a spinel frame
        if ((buf[0] & SPINEL_HEADER_FLAG_MASK) == SPINEL_HEADER_FLAG)
        {
#if OPENTHREAD_ENABLE_NCP_SPINEL_ENCRYPTER
            size_t dataLen = bufLength;
            if (SpinelEncrypter::DecryptInbound(buf, kRxBufferSize, &dataLen))
            {
                NcpBase::HandleReceive(buf, dataLen);
            }
#else
            NcpBase::HandleReceive(buf, bufLength);
#endif // OPENTHREAD_ENABLE_NCP_SPINEL_ENCRYPTER
        }
        else
        {
            for (int i = 0; i < bufLength; i++)
            {
                hci_processReceivedChar(buf[i]);
            }
        }
    }
    mRxBufferHook.Clear();
}

otError NcpBase::VendorCommandHandler(uint8_t aHeader, unsigned int aCommand)
{
    otError error = OT_ERROR_NONE;

    switch (aCommand)
    {
    default:
        error = PrepareLastStatusResponse(aHeader, SPINEL_STATUS_INVALID_COMMAND);
    }

    return error;
}

void NcpBase::VendorHandleFrameRemovedFromNcpBuffer(Spinel::Buffer::FrameTag aFrameTag)
{
    // This method is a callback which mirrors `NcpBase::HandleFrameHookRemovedFromNcpBuffer()`.
    // It is called when a spinel frame is sent and removed from NCP buffer.
    //
    // (a) This can be used to track and verify that a vendor spinel frame response is
    //     delivered to the host (tracking the frame using its tag).
    //
    // (b) It indicates that NCP buffer space is now available (since a spinel frame is
    //     removed). This can be used to implement reliability mechanisms to re-send
    //     a failed spinel command response (or an async spinel frame) transmission
    //     (failed earlier due to NCP buffer being full).

    OT_UNUSED_VARIABLE(aFrameTag);
}

otError NcpBase::VendorGetPropertyHandler(spinel_prop_key_t aPropKey)
{
    otError error = OT_ERROR_NONE;

    switch (aPropKey)
    {
        // TODO: Implement your property get handlers here.
        //
        // Get handler should retrieve the property value and then encode and write the
        // value into the NCP buffer. If the "get" operation itself fails, handler should
        // write a `LAST_STATUS` with the error status into the NCP buffer. `OT_ERROR_NO_BUFS`
        // should be returned if NCP buffer is full and response cannot be written.

    default:
        error = OT_ERROR_NOT_FOUND;
        break;
    }

    return error;
}

otError NcpBase::VendorSetPropertyHandler(spinel_prop_key_t aPropKey)
{
    otError error = OT_ERROR_NONE;

    switch (aPropKey)
    {
    // TODO: Implement your property set handlers here.
    //
    // Set handler should first decode the value from the input Spinel frame and then
    // perform the corresponding set operation. The handler should not prepare the
    // spinel response and therefore should not write anything to the NCP buffer.
    // The error returned from handler (other than `OT_ERROR_NOT_FOUND`) indicates the
    // error in either parsing of the input or the error of the set operation. In case
    // of a successful "set", `NcpBase` set command handler will invoke the
    // `VendorGetPropertyHandler()` for the same property key to prepare the response.
    case OT_NXP_SPINEL_PROP_VENDOR_BLE_STATE:
        BleAppInactivityCallback(0xFFFFFFFF);
        PRINTF("BLE STATE set to off\n");
        break;

    default:
        error = OT_ERROR_NOT_FOUND;
        break;
    }

    return error;
}

} // namespace Ncp
} // namespace ot

static OT_DEFINE_ALIGNED_VAR(sNcpVendorRaw, sizeof(ot::Ncp::NcpNxpVendorUart), uint64_t);

extern "C" void otAppNcpInit(otInstance *aInstance)
{
    ot::Ncp::NcpNxpVendorUart *ncpVendor = nullptr;
    ot::Instance *             instance  = static_cast<ot::Instance *>(aInstance);

    IgnoreError(otPlatUartEnable());

    ncpVendor = new (&sNcpVendorRaw) ot::Ncp::NcpNxpVendorUart(instance);

    if (ncpVendor == nullptr || ncpVendor != ot::Ncp::NcpBase::GetNcpInstance())
    {
        // assert(false);
    }
}

#endif // #if OPENTHREAD_ENABLE_NCP_VENDOR_HOOK
