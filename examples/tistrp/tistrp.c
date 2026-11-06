/*
 *  Copyright 2026 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TIS/TRP DUT role (Zigbee RF Performance Test Plan 19-01701, Annex B) on the OT host.
 *
 * OTA frames are raw PHY payloads (no MAC header): [0xDE][0xDE][0x00][cmdId][payload...] + FCS.
 * The host receives every frame through the OpenThread raw link API in promiscuous mode,
 * counts RX statistics and answers the command module (CMD) with raw transmits.
 * The modulated STREAM uses the existing NXP MFG continuous TX vendor command.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openthread/cli.h>
#include <openthread/instance.h>
#include <openthread/link.h>
#include <openthread/link_raw.h>
#include <openthread/platform/radio.h>

#include "FreeRTOS.h"
#include "timers.h"

#include "app_ot.h"
#include "ot_platform_common.h"
#include "tistrp.h"
#include "lib/spinel/spinel.h"

#if !OPENTHREAD_CONFIG_LINK_RAW_ENABLE
#error "tistrp addon requires OPENTHREAD_CONFIG_LINK_RAW_ENABLE (build with -DOT_LINK_RAW=ON)"
#endif

/* -------------------------------------------------------------------------- */
/*                               Definitions                                  */
/* -------------------------------------------------------------------------- */
#define TISTRP_DELIM 0xDE
#define TISTRP_HDR_LEN 4U
#define TISTRP_FCS_LEN 2U

#define TISTRP_CMD_PING 0x00
#define TISTRP_CMD_ACK 0x01
#define TISTRP_CMD_SET_CHANNEL 0x02
#define TISTRP_CMD_SET_POWER 0x05
#define TISTRP_CMD_GET_POWER 0x06
#define TISTRP_CMD_GET_POWER_RSP 0x07
#define TISTRP_CMD_STREAM 0x09
#define TISTRP_CMD_START_RX_TEST 0x0A
#define TISTRP_CMD_END_RX_TEST 0x0B
#define TISTRP_CMD_REPORT 0x0C
#define TISTRP_CMD_REBOOT 0x0D
#define TISTRP_CMD_GET_HW_VER 0x11
#define TISTRP_CMD_GET_HW_VER_RSP 0x12
#define TISTRP_CMD_GET_SW_VER 0x13
#define TISTRP_CMD_GET_SW_VER_RSP 0x14

#ifndef TISTRP_DEFAULT_CHANNEL
#define TISTRP_DEFAULT_CHANNEL 12 /* same default as the MCXW72 TISTRP firmware */
#endif
#ifndef TISTRP_HW_VERSION
#define TISTRP_HW_VERSION 0x01
#endif
#ifndef TISTRP_SW_VERSION
#define TISTRP_SW_VERSION 0x01
#endif
#ifndef TISTRP_DEFAULT_TX_POWER
#define TISTRP_DEFAULT_TX_POWER 10 /* dBm, same default as the MCXW72 TISTRP firmware */
#endif

#ifndef TISTRP_REPORT_BIG_ENDIAN
#define TISTRP_REPORT_BIG_ENDIAN 0 /* 0: little-endian, as the NXP MCXW72 TISTRP CMD/DUT reference */
#endif

#define TISTRP_MIN_CHANNEL 11
#define TISTRP_MAX_CHANNEL 26
#define TISTRP_REBOOT_DELAY_MS 50

/* NXP MFG vendor command (same encoding as radio_cli.c) */
#define SPINEL_CMD_VENDOR_NXP_MFG (SPINEL_CMD_VENDOR__BEGIN + 0x3FF)
#define MFG_CMD_ACTION_SET 1
#define MFG_CMD_CONTINUOUS_TX 0x11

typedef struct
{
    uint32_t totalPackets;
    uint32_t protocolPackets;
    uint32_t totalLqi;
    uint32_t totalRssiMagnitude;
} tistrpStats_t;

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */
static otInstance   *sInstance;
static bool          sActive;
static bool          sStreaming;
static bool          sTxBusy;
static uint8_t       sChannel = TISTRP_DEFAULT_CHANNEL;
static tistrpStats_t sStats;
static TimerHandle_t sStreamTimer;
static TimerHandle_t sRebootTimer;

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */
static otError tistrpStart(uint8_t aChannel);
static otError tistrpStop(void);
static void    tistrpReceiveDone(otInstance *aInstance, otRadioFrame *aFrame, otError aError);
static void    tistrpTransmitDone(otInstance *aInstance, otRadioFrame *aFrame, otRadioFrame *aAck, otError aError);
static void    tistrpHandleCommand(const uint8_t *aPsdu, uint8_t aPsduLen);
static otError tistrpSend(uint8_t aCmdId, const uint8_t *aPayload, uint8_t aPayloadLen);
static otError tistrpSetChannel(uint8_t aChannel);
static otError tistrpContinuousTx(bool aEnable);
static void    tistrpStreamTimerCb(TimerHandle_t aTimer);
static void    tistrpRebootTimerCb(TimerHandle_t aTimer);
static void    putU32(uint8_t *aBuf, uint32_t aValue);

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */
void otAppTistrpInit(otInstance *aInstance)
{
    sInstance = aInstance;

    sStreamTimer = xTimerCreate("tistrp_stream", 1, pdFALSE, NULL, tistrpStreamTimerCb);
    sRebootTimer =
        xTimerCreate("tistrp_reboot", pdMS_TO_TICKS(TISTRP_REBOOT_DELAY_MS), pdFALSE, NULL, tistrpRebootTimerCb);

#if defined(OT_APP_TISTRP_AUTOSTART) && OT_APP_TISTRP_AUTOSTART
    /* Chamber use: the DUT is not wired to a PC, enter DUT mode at boot */
    (void)tistrpStart(TISTRP_DEFAULT_CHANNEL);
#endif
}

otError ProcessTistrp(void *aContext, uint8_t aArgsLength, char *aArgs[])
{
    otError error = OT_ERROR_INVALID_ARGS;

    OT_UNUSED_VARIABLE(aContext);

    if (aArgsLength == 0)
    {
        otCliOutputFormat("tistrp start [channel] | stop | status | reboot\r\n");
        error = OT_ERROR_NONE;
    }
    else if (strcmp(aArgs[0], "start") == 0)
    {
        unsigned long channel = TISTRP_DEFAULT_CHANNEL;
        char         *end     = NULL;

        if (aArgsLength > 1)
        {
            channel = strtoul(aArgs[1], &end, 0);
        }

        if ((aArgsLength > 1) && ((end == aArgs[1]) || (*end != '\0')))
        {
            error = OT_ERROR_INVALID_ARGS;
        }
        else if ((channel < TISTRP_MIN_CHANNEL) || (channel > TISTRP_MAX_CHANNEL))
        {
            error = OT_ERROR_INVALID_ARGS;
        }
        else
        {
            error = tistrpStart((uint8_t)channel);
        }
    }
    else if (strcmp(aArgs[0], "stop") == 0)
    {
        error = tistrpStop();
    }
    else if (strcmp(aArgs[0], "reboot") == 0)
    {
        /* same path as the OTA REBOOT command */
        error = (xTimerStart(sRebootTimer, 0) == pdPASS) ? OT_ERROR_NONE : OT_ERROR_FAILED;
    }
    else if (strcmp(aArgs[0], "status") == 0)
    {
        int8_t power = 0;

        (void)otPlatRadioGetTransmitPower(sInstance, &power);
        otCliOutputFormat("active: %d  channel: %d  power: %d dBm  stream: %d\r\n", sActive, sChannel, power,
                          sStreaming);
        otCliOutputFormat("total: %lu  protocol: %lu  lqi: %lu  rssiMag: %lu\r\n", (unsigned long)sStats.totalPackets,
                          (unsigned long)sStats.protocolPackets, (unsigned long)sStats.totalLqi,
                          (unsigned long)sStats.totalRssiMagnitude);
        error = OT_ERROR_NONE;
    }

    return error;
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */
static otError tistrpStart(uint8_t aChannel)
{
    otError error;

    if ((aChannel < TISTRP_MIN_CHANNEL) || (aChannel > TISTRP_MAX_CHANNEL))
    {
        return OT_ERROR_INVALID_ARGS;
    }

    /* a previous session may have been stopped during a STREAM or a pending TX */
    sStreaming = false;
    sTxBusy    = false;

    /* Raw link can only be enabled when the Thread interface is down (ifconfig down) */
    error = otLinkRawSetReceiveDone(sInstance, tistrpReceiveDone);
    if (error == OT_ERROR_NONE)
    {
        /* On FTD builds enabling raw link disables the MAC/SubMac, which disables the RCP PHY.
         * Re-enable it: the RCP rejects promiscuous/receive requests while its PHY is disabled. */
        error = otPlatRadioEnable(sInstance);
    }
    if (error == OT_ERROR_NONE)
    {
        (void)otLinkRawSetPromiscuous(sInstance, true);
        (void)otPlatRadioSetTransmitPower(sInstance, TISTRP_DEFAULT_TX_POWER);
        sActive = true;
        memset(&sStats, 0, sizeof(sStats));
        error = tistrpSetChannel(aChannel);
    }

    return error;
}

static otError tistrpStop(void)
{
    if (sStreaming)
    {
        (void)xTimerStop(sStreamTimer, 0);
        (void)tistrpContinuousTx(false);
    }
    sStreaming = false; /* the stream timer callback will not run anymore */
    sTxBusy    = false; /* TX done may never be reported once the raw link is disabled */
    sActive    = false;
    (void)otLinkRawSetPromiscuous(sInstance, false);
    (void)otLinkRawSleep(sInstance);

    return otLinkRawSetReceiveDone(sInstance, NULL);
}

static otError tistrpSetChannel(uint8_t aChannel)
{
    otError error;

    sChannel = aChannel;
    error    = otLinkSetChannel(sInstance, aChannel); /* routed to LinkRaw::SetChannel when raw link is on */
    if (error == OT_ERROR_NONE)
    {
        error = otLinkRawReceive(sInstance);
    }

    return error;
}

static void tistrpReceiveDone(otInstance *aInstance, otRadioFrame *aFrame, otError aError)
{
    bool    isProtocol;
    uint8_t psduLen;

    OT_UNUSED_VARIABLE(aInstance);

    if ((aError != OT_ERROR_NONE) || (aFrame == NULL) || !sActive || sStreaming)
    {
        return;
    }

    psduLen = (aFrame->mLength > TISTRP_FCS_LEN) ? (uint8_t)(aFrame->mLength - TISTRP_FCS_LEN) : 0U;
    isProtocol =
        (psduLen >= TISTRP_HDR_LEN) && (aFrame->mPsdu[0] == TISTRP_DELIM) && (aFrame->mPsdu[1] == TISTRP_DELIM);

    /* as the MCXW72 reference: every frame is counted, RSSI accumulated as its raw 8-bit value */
    sStats.totalPackets++;
    sStats.totalLqi += aFrame->mInfo.mRxInfo.mLqi;
    sStats.totalRssiMagnitude += (uint8_t)aFrame->mInfo.mRxInfo.mRssi;
    if (isProtocol)
    {
        sStats.protocolPackets++;
    }

    if (isProtocol)
    {
        tistrpHandleCommand(aFrame->mPsdu, psduLen);
    }
}

static void tistrpHandleCommand(const uint8_t *aPsdu, uint8_t aPsduLen)
{
    const uint8_t *body    = &aPsdu[TISTRP_HDR_LEN];
    uint8_t        bodyLen = aPsduLen - TISTRP_HDR_LEN;
    uint8_t        rsp[16];

    switch (aPsdu[3])
    {
    case TISTRP_CMD_PING:
        (void)tistrpSend(TISTRP_CMD_ACK, NULL, 0);
        break;

    case TISTRP_CMD_SET_CHANNEL:
        if (bodyLen >= 4)
        {
            uint32_t mask = ((uint32_t)body[0] << 24) | ((uint32_t)body[1] << 16) | ((uint32_t)body[2] << 8) | body[3];
            uint8_t  ch   = 0;

            /* as the MCXW72 reference: channel = highest bit set in the 32-bit mask */
            while (mask > 1UL)
            {
                mask >>= 1;
                ch++;
            }
            if ((ch >= TISTRP_MIN_CHANNEL) && (ch <= TISTRP_MAX_CHANNEL))
            {
                (void)tistrpSetChannel(ch); /* no answer: CMD follows and PINGs on the new channel */
            }
        }
        break;

    case TISTRP_CMD_SET_POWER:
        if (bodyLen >= 1)
        {
            (void)otPlatRadioSetTransmitPower(sInstance, (int8_t)body[0]);
        }
        break;

    case TISTRP_CMD_GET_POWER:
    {
        int8_t power = 0;

        (void)otPlatRadioGetTransmitPower(sInstance, &power);
        rsp[0] = (uint8_t)power;
        (void)tistrpSend(TISTRP_CMD_GET_POWER_RSP, rsp, 1);
        break;
    }

    case TISTRP_CMD_STREAM:
        if ((bodyLen >= 2) && !sTxBusy)
        {
            uint16_t durationMs = (uint16_t)((body[0] << 8) | body[1]);

            if ((durationMs > 0U) && (tistrpContinuousTx(true) == OT_ERROR_NONE))
            {
                sStreaming = true;
                (void)xTimerChangePeriod(sStreamTimer, pdMS_TO_TICKS(durationMs), 0);
                (void)xTimerStart(sStreamTimer, 0);
            }
        }
        break;

    case TISTRP_CMD_START_RX_TEST:
        memset(&sStats, 0, sizeof(sStats));
        break;

    case TISTRP_CMD_END_RX_TEST:
        putU32(&rsp[0], sStats.totalPackets);
        putU32(&rsp[4], sStats.protocolPackets);
        putU32(&rsp[8], sStats.totalLqi);
        putU32(&rsp[12], sStats.totalRssiMagnitude);
        (void)tistrpSend(TISTRP_CMD_REPORT, rsp, 16);
        break;

    case TISTRP_CMD_REBOOT:
        (void)xTimerStart(sRebootTimer, 0);
        break;

    case TISTRP_CMD_GET_HW_VER:
        rsp[0] = TISTRP_HW_VERSION;
        (void)tistrpSend(TISTRP_CMD_GET_HW_VER_RSP, rsp, 1);
        break;

    case TISTRP_CMD_GET_SW_VER:
        rsp[0] = TISTRP_SW_VERSION;
        (void)tistrpSend(TISTRP_CMD_GET_SW_VER_RSP, rsp, 1);
        break;

    default:
        /* ACK, GET_POWER_RSP, REPORT, *_RSP are CMD-bound, other IDs unknown: ignore */
        break;
    }
}

static otError tistrpSend(uint8_t aCmdId, const uint8_t *aPayload, uint8_t aPayloadLen)
{
    otRadioFrame *frame = otLinkRawGetTransmitBuffer(sInstance);
    otError       error = OT_ERROR_BUSY;

    if ((frame != NULL) && !sTxBusy && !sStreaming)
    {
        frame->mPsdu[0] = TISTRP_DELIM;
        frame->mPsdu[1] = TISTRP_DELIM;
        frame->mPsdu[2] = 0x00;
        frame->mPsdu[3] = aCmdId;
        if (aPayloadLen > 0U)
        {
            memcpy(&frame->mPsdu[TISTRP_HDR_LEN], aPayload, aPayloadLen);
        }
        frame->mLength  = (uint16_t)(TISTRP_HDR_LEN + aPayloadLen + TISTRP_FCS_LEN);
        frame->mChannel = sChannel;

        /* 0xDEDE is not a valid MAC header: no CSMA retries, no ACK, no MAC security processing */
        frame->mInfo.mTxInfo.mMaxCsmaBackoffs      = 0;
        frame->mInfo.mTxInfo.mMaxFrameRetries      = 0;
        frame->mInfo.mTxInfo.mCsmaCaEnabled        = false;
        frame->mInfo.mTxInfo.mIsHeaderUpdated      = true;
        frame->mInfo.mTxInfo.mIsSecurityProcessed  = true;
        frame->mInfo.mTxInfo.mIsARetx              = false;
        frame->mInfo.mTxInfo.mCslPresent           = false;
        frame->mInfo.mTxInfo.mTxDelay              = 0;
        frame->mInfo.mTxInfo.mTxDelayBaseTime      = 0;
        frame->mInfo.mTxInfo.mRxChannelAfterTxDone = sChannel;
        frame->mInfo.mTxInfo.mTxPower              = OT_RADIO_POWER_INVALID;

        error   = otLinkRawTransmit(sInstance, tistrpTransmitDone);
        sTxBusy = (error == OT_ERROR_NONE);
    }

    return error;
}

static void tistrpTransmitDone(otInstance *aInstance, otRadioFrame *aFrame, otRadioFrame *aAck, otError aError)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aFrame);
    OT_UNUSED_VARIABLE(aAck);
    OT_UNUSED_VARIABLE(aError);

    sTxBusy = false;
}

static otError tistrpContinuousTx(bool aEnable)
{
    uint8_t payload[12] = {11};
    uint8_t outputLen   = sizeof(payload);
    otError error;

    payload[1] = MFG_CMD_CONTINUOUS_TX;
    payload[2] = MFG_CMD_ACTION_SET;
    payload[4] = aEnable ? 1U : 0U;

    error = otPlatRadioMfgCommand(sInstance, SPINEL_CMD_VENDOR_NXP_MFG, payload, sizeof(payload), &outputLen);
    if ((error == OT_ERROR_NONE) && ((outputLen < 4U) || (payload[3] != 0U)))
    {
        error = OT_ERROR_FAILED;
    }

    return error;
}

static void tistrpStreamTimerCb(TimerHandle_t aTimer)
{
    OT_UNUSED_VARIABLE(aTimer);

    appOtLockOtTask();
    (void)tistrpContinuousTx(false); /* RCP puts the radio to sleep */
    sStreaming = false;
    if (sActive)
    {
        /* MFG continuous TX off leaves the RCP radio asleep while the host still believes it is in
         * receive: go through sleep so that receive re-enables the RCP raw stream. */
        (void)otLinkRawSleep(sInstance);
        (void)otLinkRawReceive(sInstance);
    }
    appOtUnlockOtTask();
}

static void tistrpRebootTimerCb(TimerHandle_t aTimer)
{
    OT_UNUSED_VARIABLE(aTimer);

    appOtLockOtTask();
    otInstanceReset(sInstance);
    appOtUnlockOtTask();
}

static void putU32(uint8_t *aBuf, uint32_t aValue)
{
#if TISTRP_REPORT_BIG_ENDIAN
    aBuf[0] = (uint8_t)(aValue >> 24);
    aBuf[1] = (uint8_t)(aValue >> 16);
    aBuf[2] = (uint8_t)(aValue >> 8);
    aBuf[3] = (uint8_t)aValue;
#else
    aBuf[0] = (uint8_t)aValue;
    aBuf[1] = (uint8_t)(aValue >> 8);
    aBuf[2] = (uint8_t)(aValue >> 16);
    aBuf[3] = (uint8_t)(aValue >> 24);
#endif
}
