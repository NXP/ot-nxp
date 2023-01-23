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

/**
 * @file
 *   This file includes the implementation for the SPI interface to radio (RCP).
 */

#include "spi_interface.hpp"

#include <inttypes.h>

#include "fsl_adapter_spi.h"
#include "fsl_lpspi.h"
#include "fsl_os_abstraction.h"
#include "fwk_platform_ot.h"
#include <FreeRTOS.h>
#include <timers.h>
#include "common/logging.hpp"

#define LPSPI_MASTER_CLK_FREQ (CLOCK_GetFreqFromObs(CCM_OBS_LPSPI1_CLK_ROOT))
#define OT_PLATFORM_CONFIG_SPI_DEFAULT_SPEED_HZ 1000000 ///< Default SPI speed in hertz.

#define OT_PLATFORM_CONFIG_SPI_DEFAULT_ALIGN_ALLOWANCE \
    16 ///< Default maximum number of 0xFF bytes to clip from start of MISO frame.
#define OT_PLATFORM_CONFIG_SPI_DEFAULT_SMALL_PACKET_SIZE \
    64 ///< Default smallest SPI packet size we can receive in a single transaction.

using ot::Spinel::SpinelInterface;

static void SpiIntGpioCallback(void *pData)
{
    static_cast<ot::NXP::SpiInterface *>(pData)->IncreasePendingSpiRxDataCounter();
}

namespace ot {
namespace NXP {

SpiInterface::SpiInterface(SpinelInterface::ReceiveFrameCallback aCallback,
                           void *                                aCallbackContext,
                           SpinelInterface::RxFrameBuffer &      aFrameBuffer)
    : mReceiveFrameCallback(aCallback)
    , mReceiveFrameContext(aCallbackContext)
    , mRxFrameBuffer(aFrameBuffer)
    , mSlaveResetCount(0)
    , mSpiFrameCount(0)
    , mSpiValidFrameCount(0)
    , mSpiGarbageFrameCount(0)
    , mSpiDuplexFrameCount(0)
    , mSpiUnresponsiveFrameCount(0)
    , mSpiRxFrameCount(0)
    , mSpiRxFrameByteCount(0)
    , mSpiTxFrameCount(0)
    , mSpiTxFrameByteCount(0)
    , mSpiTxIsReady(false)
    , mSpiTxRefusedCount(0)
    , mSpiTxPayloadSize(0)
    , mSpiSlaveDataLen(0)
    , mSpiSlaveAcceptLen(0)
    , isInitialized(false)
    , pendingSpiRxDataCounter(0)
{
}

void SpiInterface::OnRcpReset(void)
{
}

void SpiInterface::Init(void)
{
    hal_gpio_status_t status;

    hal_gpio_pin_config_t spiIntPinConfig = {
        .direction = kHAL_GpioDirectionIn,
        .level     = 0U,
        .port      = 3,
        .pin       = 10,
    };

    if (!isInitialized)
    {
        mSpiSmallPacketSize = OT_PLATFORM_CONFIG_SPI_DEFAULT_SMALL_PACKET_SIZE;
        mSpiAlignAllowance  = OT_PLATFORM_CONFIG_SPI_DEFAULT_ALIGN_ALLOWANCE;

        InitSpi();

        /* Init SPI INT IRQ */
        status = HAL_GpioInit((hal_gpio_handle_t)otGpioSpiIntHandle, &spiIntPinConfig);
        assert(status == kStatus_HAL_GpioSuccess);
        status = HAL_GpioInstallCallback((hal_gpio_handle_t)otGpioSpiIntHandle, SpiIntGpioCallback, this);
        assert(status == kStatus_HAL_GpioSuccess);
        status = HAL_GpioSetTriggerMode((hal_gpio_handle_t)otGpioSpiIntHandle, kHAL_GpioInterruptFallingEdge);
        assert(status == kStatus_HAL_GpioSuccess);

        isInitialized = true;
    }

    OT_UNUSED_VARIABLE(status);
}

void SpiInterface::InitSpi(void)
{
    hal_spi_status_t status;

    hal_spi_master_config_t spiConfig = {
        .srcClock_Hz  = LPSPI_MASTER_CLK_FREQ,
        .baudRate_Bps = OT_PLATFORM_CONFIG_SPI_DEFAULT_SPEED_HZ,
        .polarity     = kHAL_SpiClockPolarityActiveHigh,
        .phase        = kHAL_SpiClockPhaseFirstEdge,
        .direction    = kHAL_SpiMsbFirst,
        .instance     = 1,
        .enableMaster = true,
    };

    status = HAL_SpiMasterInit((hal_spi_master_handle_t)otSpiMasterHandle, &spiConfig);
    assert(status == kStatus_HAL_SpiSuccess);

    OT_UNUSED_VARIABLE(status);
}

void SpiInterface::DeinitSpi(void)
{
    hal_spi_status_t  statusHalSpi;
    hal_gpio_status_t statusHalGpio;
    statusHalSpi = HAL_SpiMasterDeinit((hal_spi_master_handle_t)otSpiMasterHandle);
    assert(statusHalSpi == kStatus_HAL_SpiSuccess);
    statusHalGpio = HAL_GpioDeinit((hal_gpio_handle_t)otGpioSpiIntHandle);
    assert(statusHalGpio == kStatus_HAL_GpioSuccess);
    OT_UNUSED_VARIABLE(statusHalSpi);
    OT_UNUSED_VARIABLE(statusHalGpio);
}

SpiInterface::~SpiInterface(void)
{
    Deinit();
}

void SpiInterface::Deinit(void)
{
    DeinitSpi();
}

void SpiInterface::IncreasePendingSpiRxDataCounter(void)
{
    pendingSpiRxDataCounter++;
    /* Schedule a push pull */
    otSysEventSignalPending();
}

otError SpiInterface::PushPullSpi(void)
{
    otError       error               = OT_ERROR_FAILED;
    uint16_t      spiTransferBytes    = 0;
    uint8_t       successfulExchanges = 0;
    bool          discardRxFrame      = true;
    uint8_t *     spiRxFrameBuffer;
    uint8_t *     spiRxFrame;
    uint8_t       slaveHeader;
    Ncp::SpiFrame txFrame(mSpiTxFrameBuffer);
    uint16_t      skipAlignAllowanceLength;
    bool          decreaseSpiRxDataCounter = false;

    /* Do we need to do a SPI push pull */
    if (pendingSpiRxDataCounter == 0 && !mSpiTxIsReady)
    {
        error = OT_ERROR_NONE;
        ExitNow();
    }

    if (pendingSpiRxDataCounter > 0)
    {
        /* The value of pendingSpiRxDataCounter will be decreased only after receiving a valid Rx frame */
        decreaseSpiRxDataCounter = true;
    }

    if (mSpiValidFrameCount == 0)
    {
        // Set the reset flag to indicate to our slave that we are coming up from scratch.
        txFrame.SetHeaderFlagByte(true);
    }
    else
    {
        txFrame.SetHeaderFlagByte(false);
    }

    // Zero out our rx_accept and our data_len for now.
    txFrame.SetHeaderAcceptLen(0);
    txFrame.SetHeaderDataLen(0);

    // Sanity check.
    if (mSpiSlaveDataLen > kMaxFrameSize)
    {
        mSpiSlaveDataLen = 0;
    }

    if (mSpiTxIsReady)
    {
        // Go ahead and try to immediately send a frame if we have it queued up.
        txFrame.SetHeaderDataLen(mSpiTxPayloadSize);

        if (mSpiTxPayloadSize > spiTransferBytes)
        {
            spiTransferBytes = mSpiTxPayloadSize;
        }
    }

    if (mSpiSlaveDataLen != 0)
    {
        // In a previous transaction the slave indicated it had something to send us. Make sure our transaction
        // is large enough to handle it.
        if (mSpiSlaveDataLen > spiTransferBytes)
        {
            spiTransferBytes = mSpiSlaveDataLen;
        }
    }
    else
    {
        // Set up a minimum transfer size to allow small frames the slave wants to send us to be handled in a
        // single transaction.
        if (spiTransferBytes < mSpiSmallPacketSize)
        {
            spiTransferBytes = mSpiSmallPacketSize;
        }
    }

    txFrame.SetHeaderAcceptLen(spiTransferBytes);

    // Set skip length to make MultiFrameBuffer to reserve a space in front of the frame buffer.
    SuccessOrExit(error = mRxFrameBuffer.SetSkipLength(kSpiFrameHeaderSize));

    // Check whether the remaining frame buffer has enough space to store the data to be received.
    if (mRxFrameBuffer.GetFrameMaxLength() < spiTransferBytes + mSpiAlignAllowance)
    {
        error = OT_ERROR_NO_BUFS;
        otLogCritPlat("RX buffer is full");
        ExitNow();
    }

    // Point to the start of the reserved buffer.
    spiRxFrameBuffer = mRxFrameBuffer.GetFrame() - kSpiFrameHeaderSize;

    // Set the total number of bytes to be transmitted.
    spiTransferBytes += kSpiFrameHeaderSize + mSpiAlignAllowance;

    // Perform the SPI transaction.
    error = DoSpiTransfer(spiRxFrameBuffer, spiTransferBytes);

    if (error != OT_ERROR_NONE)
    {
        otLogCritPlat("PushPullSpi:DoSpiTransfer: error");
        LogStats();
        ExitNow();
    }

    // Account for misalignment (0xFF bytes at the start)
    spiRxFrame = GetRealRxFrameStart(spiRxFrameBuffer, mSpiAlignAllowance, skipAlignAllowanceLength);

    {
        Ncp::SpiFrame rxFrame(spiRxFrame);

        otLogDebgPlat("spi_transfer TX: H:%02X ACCEPT:%" PRIu16 " DATA:%" PRIu16, txFrame.GetHeaderFlagByte(),
                      txFrame.GetHeaderAcceptLen(), txFrame.GetHeaderDataLen());
        otLogDebgPlat("spi_transfer RX: H:%02X ACCEPT:%" PRIu16 " DATA:%" PRIu16, rxFrame.GetHeaderFlagByte(),
                      rxFrame.GetHeaderAcceptLen(), rxFrame.GetHeaderDataLen());

        // otDumpDebgPlat("SPI-TX", mSpiTxFrameBuffer, 5+txFrame.GetHeaderDataLen());
        // otDumpDebgPlat("SPI-RX", spiRxFrame, 5+rxFrame.GetHeaderDataLen());

        slaveHeader = rxFrame.GetHeaderFlagByte();
        if ((slaveHeader == 0xFF) || (slaveHeader == 0x00))
        {
            if ((slaveHeader == spiRxFrame[1]) && (slaveHeader == spiRxFrame[2]) && (slaveHeader == spiRxFrame[3]) &&
                (slaveHeader == spiRxFrame[4]))
            {
                // Device is off or in a bad state. In some cases may be induced by flow control.
                if (mSpiSlaveDataLen == 0)
                {
                    otLogDebgPlat("Slave did not respond to frame. (Header was all 0x%02X)", slaveHeader);
                }
                else
                {
                    otLogWarnPlat("Slave did not respond to frame. (Header was all 0x%02X)", slaveHeader);
                }

                mSpiUnresponsiveFrameCount++;
            }
            else
            {
                // Header is full of garbage
                mSpiGarbageFrameCount++;

                otLogWarnPlat("Garbage in header : %02X %02X %02X %02X %02X", spiRxFrame[0], spiRxFrame[1],
                              spiRxFrame[2], spiRxFrame[3], spiRxFrame[4]);
                otDumpDebgPlat("SPI-TX", mSpiTxFrameBuffer, spiTransferBytes);
                otDumpDebgPlat("SPI-RX", spiRxFrameBuffer, spiTransferBytes);
            }

            mSpiTxRefusedCount++;
            ExitNow();
        }

        mSpiSlaveAcceptLen = rxFrame.GetHeaderAcceptLen();
        mSpiSlaveDataLen   = rxFrame.GetHeaderDataLen();

        if (!rxFrame.IsValid() || (mSpiSlaveAcceptLen > kMaxFrameSize) || (mSpiSlaveDataLen > kMaxFrameSize))
        {
            mSpiGarbageFrameCount++;
            mSpiTxRefusedCount++;
            mSpiSlaveDataLen = 0;

            otLogWarnPlat("Garbage in header : %02X %02X %02X %02X %02X", spiRxFrame[0], spiRxFrame[1], spiRxFrame[2],
                          spiRxFrame[3], spiRxFrame[4]);
            otDumpDebgPlat("SPI-TX", mSpiTxFrameBuffer, spiTransferBytes);
            otDumpDebgPlat("SPI-RX", spiRxFrameBuffer, spiTransferBytes);

            ExitNow();
        }

        mSpiValidFrameCount++;

        if (rxFrame.IsResetFlagSet())
        {
            mSlaveResetCount++;

            otLogNotePlat("Slave did reset (%lld resets so far)", mSlaveResetCount);
            LogStats();
        }

        // Handle received packet, if any.
        if (mSpiSlaveDataLen != 0)
        {
            if (mSpiSlaveDataLen <= txFrame.GetHeaderAcceptLen())
            {
                mSpiRxFrameByteCount += mSpiSlaveDataLen;
                mSpiSlaveDataLen = 0;
                mSpiRxFrameCount++;
                successfulExchanges++;

                // Set the skip length to skip align bytes and SPI frame header.
                SuccessOrExit(error = mRxFrameBuffer.SetSkipLength(skipAlignAllowanceLength + kSpiFrameHeaderSize));
                // Set the received frame length.
                SuccessOrExit(error = mRxFrameBuffer.SetLength(rxFrame.GetHeaderDataLen()));

                // Upper layer will free the frame buffer.
                discardRxFrame = false;

                mReceiveFrameCallback(mReceiveFrameContext);
            }
            else
            {
                // in case no specific data expected, length of Rx data is set to default small packet size
                // if Rx data is bigger, frame is not handled and should be pulled again with appropriate expected data
                // length in this case, Spi Rx data counter should not be decreased as no additional interrupt will be
                // triggered
                decreaseSpiRxDataCounter = false;
            }
        }

        // A RX frame has been correclty received, now decrease the RX data counter
        if (decreaseSpiRxDataCounter)
            pendingSpiRxDataCounter--;
    }

    // Handle transmitted packet, if any.
    if (mSpiTxIsReady && (mSpiTxPayloadSize == txFrame.GetHeaderDataLen()))
    {
        if (txFrame.GetHeaderDataLen() <= mSpiSlaveAcceptLen)
        {
            // Our outbound packet has been successfully transmitted. Clear mSpiTxPayloadSize and mSpiTxIsReady so
            // that uplayer can pull another packet for us to send.
            successfulExchanges++;

            mSpiTxFrameCount++;
            mSpiTxFrameByteCount += mSpiTxPayloadSize;

            mSpiTxIsReady      = false;
            mSpiTxPayloadSize  = 0;
            mSpiTxRefusedCount = 0;
            memset(mSpiTxFrameBuffer, 0, sizeof(mSpiTxFrameBuffer));
            error = OT_ERROR_NONE;
        }
        else
        {
            // The slave wasn't ready for what we had to send them. Incrementing this counter will turn on rate
            // limiting so that we don't waste a ton of CPU bombarding them with useless SPI transfers.
            mSpiTxRefusedCount++;
        }
    }

    if (!mSpiTxIsReady)
    {
        mSpiTxRefusedCount = 0;
    }

    if (successfulExchanges == 2)
    {
        mSpiDuplexFrameCount++;
    }

exit:

    /* In case of any SPI issue that could have happened or if there is other pending SPI RX data :
     *  schedule an event to the Openthread task so that a new call to PushPull will be done in the next "process"
     * functions call
     */
    if (error != OT_ERROR_NONE || pendingSpiRxDataCounter > 0)
    {
        otLogDebgPlat("error = %d, pendingSpiRxDataCounter=%d", error, pendingSpiRxDataCounter);
        otTaskletsSignalPending(NULL);
    }

    if (discardRxFrame)
    {
        mRxFrameBuffer.DiscardFrame();
    }

    return error;
}

uint8_t *SpiInterface::GetRealRxFrameStart(uint8_t *aSpiRxFrameBuffer, uint8_t aAlignAllowance, uint16_t &aSkipLength)
{
    uint8_t *      start = aSpiRxFrameBuffer;
    const uint8_t *end   = aSpiRxFrameBuffer + aAlignAllowance;

    for (; start != end && (start[0] == 0xff || start[0] == 0x00); start++)
        ;

    aSkipLength = static_cast<uint16_t>(start - aSpiRxFrameBuffer);

    return start;
}

otError SpiInterface::DoSpiTransfer(uint8_t *aSpiRxFrameBuffer, uint32_t aTransferLength)
{
    hal_spi_status_t   status;
    hal_spi_transfer_t spiTransfer;

    memset(&spiTransfer, 0, sizeof(hal_spi_transfer_t));
    memset(aSpiRxFrameBuffer, 0, aTransferLength);
    spiTransfer.txData   = mSpiTxFrameBuffer;
    spiTransfer.rxData   = aSpiRxFrameBuffer;
    spiTransfer.dataSize = aTransferLength;
    spiTransfer.flags    = kLPSPI_MasterPcs0 | kLPSPI_MasterPcsContinuous;

    assert(mSpiTxFrameBuffer[0] & 0x2);

    status = HAL_SpiMasterTransferBlocking((hal_spi_master_handle_t)otSpiMasterHandle, &spiTransfer);

    if (status == kStatus_HAL_SpiSuccess)
    {
        mSpiFrameCount++;
    }

    return (status != kStatus_HAL_SpiSuccess) ? OT_ERROR_FAILED : OT_ERROR_NONE;
}

void SpiInterface::Process(const otInstance &aInstance)
{
    IgnoreError(PushPullSpi());
}

otError SpiInterface::WaitForFrame(uint64_t aTimeoutUs)
{
    uint32_t now   = otPlatAlarmMilliGetNow();
    uint32_t start = now;
    otError  error = OT_ERROR_RESPONSE_TIMEOUT;

    do
    {
        error = PushPullSpi();
        if (error == OT_ERROR_NONE)
            break;
        OSA_TimeDelay(100);
        now = otPlatAlarmMilliGetNow();
    } while ((now - start) < aTimeoutUs / 1000);

    return error;
}

otError SpiInterface::SendFrame(const uint8_t *aFrame, uint16_t aLength)
{
    otError error = OT_ERROR_NONE;

    otLogDebgPlat("SendFrame len = %d mSpiTxIsReady = %d", aLength, mSpiTxIsReady);

    VerifyOrExit(aLength < (kMaxFrameSize - kSpiFrameHeaderSize), error = OT_ERROR_NO_BUFS);

    VerifyOrExit(!mSpiTxIsReady, error = OT_ERROR_BUSY);

    memset(mSpiTxFrameBuffer, 0, sizeof(mSpiTxFrameBuffer));
    memcpy(&mSpiTxFrameBuffer[kSpiFrameHeaderSize], aFrame, aLength);

    mSpiTxIsReady     = true;
    mSpiTxPayloadSize = aLength;

    IgnoreError(PushPullSpi());

exit:
    return error;
}

void SpiInterface::LogStats(void)
{
    otLogInfoPlat("INFO: mSlaveResetCount=%lld", mSlaveResetCount);
    otLogInfoPlat("INFO: mSpiFrameCount=%lld", mSpiFrameCount);
    otLogInfoPlat("INFO: mSpiValidFrameCount=%lld", mSpiValidFrameCount);
    otLogInfoPlat("INFO: mSpiDuplexFrameCount=%lld", mSpiDuplexFrameCount);
    otLogInfoPlat("INFO: mSpiUnresponsiveFrameCount=%lld", mSpiUnresponsiveFrameCount);
    otLogInfoPlat("INFO: mSpiGarbageFrameCount=%lld", mSpiGarbageFrameCount);
    otLogInfoPlat("INFO: mSpiRxFrameCount=%lld", mSpiRxFrameCount);
    otLogInfoPlat("INFO: mSpiRxFrameByteCount=%lld", mSpiRxFrameByteCount);
    otLogInfoPlat("INFO: mSpiTxFrameCount=%lld", mSpiTxFrameCount);
    otLogInfoPlat("INFO: mSpiTxFrameByteCount=%lld", mSpiTxFrameByteCount);
}

} // namespace NXP
} // namespace ot
