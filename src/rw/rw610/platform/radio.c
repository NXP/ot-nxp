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
 *   This file implements the OpenThread platform abstraction for radio communication.
 *
 */
#if (defined(__GNUC__))
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include "EmbeddedTypes.h"
#include "FunctionLib.h"
#include "Phy.h"
#include "PhyInterface.h"
#include "fsl_component_messaging.h"
#include "fsl_device_registers.h"
#include "fsl_os_abstraction.h"
#include "ot_platform_common.h"
#if defined(HDI_MODE) && (HDI_MODE == 1)
#include "hdi.h"
#endif
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <utils/code_utils.h>
#include <openthread/tasklet.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/diag.h>
#include <openthread/platform/radio.h>

// clang-format off
#define DOUBLE_BUFFERING             (1)
#define DEFAULT_CHANNEL              (11)
#define DEFAULT_CCA_MODE             (gPhyNoCCABeforeTx_c)   // (gPhyNoCCABeforeTx_c) (gPhyCCAMode1_c)
#define IEEE802154_ACK_REQUEST       (1 << 5)
#define IEEE802154_MAX_LENGTH        (127)
#define IEEE802154_MIN_LENGTH        (5)
#define IEEE802154_ACK_LENGTH        (IEEE802154_MIN_LENGTH)
#define IEEE802154_FRM_CTL_LO_OFFSET (0)
#define IEEE802154_DSN_OFFSET        (2)
#define IEEE802154_FRM_TYPE_MASK     (0x7)
#define IEEE802154_FRM_TYPE_ACK      (0x2)
#define IEEE802154_TURNAROUND_LEN    (12)
#define IEEE802154_CCA_LEN           (8)
#define IEEE802154_PHY_SHR_LEN       (10)
#define IEEE802154_ACK_WAIT          (54)
#define ZLL_IRQSTS_TMR_ALL_MSK_MASK  (ZLL_IRQSTS_TMR1MSK_MASK | \
                                      ZLL_IRQSTS_TMR2MSK_MASK | \
                                      ZLL_IRQSTS_TMR3MSK_MASK | \
                                      ZLL_IRQSTS_TMR4MSK_MASK )
#define ZLL_DEFAULT_RX_FILTERING     (ZLL_RX_FRAME_FILTER_FRM_VER_FILTER(3) | \
                                      ZLL_RX_FRAME_FILTER_CMD_FT_MASK | \
                                      ZLL_RX_FRAME_FILTER_DATA_FT_MASK | \
                                      ZLL_RX_FRAME_FILTER_ACK_FT_MASK | \
                                      ZLL_RX_FRAME_FILTER_BEACON_FT_MASK)
// clang-format on

static otRadioState sState = OT_RADIO_STATE_DISABLED;
static uint16_t     sPanId;
static uint8_t      sChannel = DEFAULT_CHANNEL;
static int8_t       sMaxED;
static int8_t       sAutoTxPwrLevel = 0;
static int8_t       sLastRssi       = 0;

/* ISR Signaling Flags */
static bool    sTxDone     = false;
static bool    sRxDone     = false;
static bool    sEdScanDone = false;
static otError sTxStatus;

static otRadioFrame sTxFrame;
static otRadioFrame sRxFrame;
static uint8_t      sTxData[OT_RADIO_FRAME_MAX_SIZE];
#if DOUBLE_BUFFERING
static uint8_t sRxData[OT_RADIO_FRAME_MAX_SIZE];
#endif
static otInstance *sInstance = NULL;

/* Private functions */
static void rf_abort(void);
static void rf_set_channel(uint8_t channel);
static void rf_set_tx_power(int8_t tx_power);

otRadioState otPlatRadioGetState(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return sState;
}

void otPlatRadioGetIeeeEui64(otInstance *aInstance, uint8_t *aIeeeEui64)
{
    OT_UNUSED_VARIABLE(aInstance);

    uint32_t addrLo;
    uint32_t addrHi;

    /* TODO: implement correct way to get UID */
    addrLo = 0xFFFFFFFF;
    addrHi = 0xFFFFFFFF;

    memcpy(aIeeeEui64, &addrLo, sizeof(addrLo));
    memcpy(aIeeeEui64 + sizeof(addrLo), &addrHi, sizeof(addrHi));
}

void otPlatRadioSetPanId(otInstance *aInstance, uint16_t aPanId)
{
    OT_UNUSED_VARIABLE(aInstance);

    macToPlmeMessage_t *pMacToPlmeMsg;

    pMacToPlmeMsg = (macToPlmeMessage_t *)MSG_Alloc(sizeof(macToPlmeMessage_t));
    assert(pMacToPlmeMsg != NULL);

    pMacToPlmeMsg->macInstance                      = 0;
    pMacToPlmeMsg->msgType                          = gPlmeSetReq_c;
    pMacToPlmeMsg->msgData.setReq.PibAttribute      = gPhyPibPanId_c;
    pMacToPlmeMsg->msgData.setReq.PibAttributeValue = (uint64_t)aPanId;

    /* Send request to Phy */
    (void)MAC_PLME_SapHandler(pMacToPlmeMsg, 0);

    MSG_Free(pMacToPlmeMsg);

    sPanId = aPanId;
}

void otPlatRadioSetExtendedAddress(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    OT_UNUSED_VARIABLE(aInstance);
    OSA_SR_ALLOC();
    macToPlmeMessage_t *pMacToPlmeMsg;

    pMacToPlmeMsg = (macToPlmeMessage_t *)MSG_Alloc(sizeof(macToPlmeMessage_t));
    assert(pMacToPlmeMsg != NULL);

    union bytesToQword
    {
        uint8_t  bytesArray[8];
        uint64_t qword;
    };

    union bytesToQword converter;

    OSA_ENTER_CRITICAL();
    FLib_MemCpy(converter.bytesArray, aExtAddress->m8, 8);
    OSA_EXIT_CRITICAL();

    pMacToPlmeMsg->macInstance                      = 0;
    pMacToPlmeMsg->msgType                          = gPlmeSetReq_c;
    pMacToPlmeMsg->msgData.setReq.PibAttribute      = gPhyPibLongAddress_c;
    pMacToPlmeMsg->msgData.setReq.PibAttributeValue = converter.qword;

    /* Send request to Phy */
    (void)MAC_PLME_SapHandler(pMacToPlmeMsg, 0);

    MSG_Free(pMacToPlmeMsg);
}

void otPlatRadioSetShortAddress(otInstance *aInstance, uint16_t aShortAddress)
{
    /* PhyPpSetShortAddr */
    OT_UNUSED_VARIABLE(aInstance);

    macToPlmeMessage_t *pMacToPlmeMsg;

    pMacToPlmeMsg = (macToPlmeMessage_t *)MSG_Alloc(sizeof(macToPlmeMessage_t));
    assert(pMacToPlmeMsg != NULL);

    pMacToPlmeMsg->macInstance                      = 0;
    pMacToPlmeMsg->msgType                          = gPlmeSetReq_c;
    pMacToPlmeMsg->msgData.setReq.PibAttribute      = gPhyPibShortAddress_c;
    pMacToPlmeMsg->msgData.setReq.PibAttributeValue = (uint64_t)aShortAddress;

    /* Send request to Phy */
    (void)MAC_PLME_SapHandler(pMacToPlmeMsg, 0);

    MSG_Free(pMacToPlmeMsg);
}

otError otPlatRadioEnable(otInstance *aInstance)
{
    otEXPECT(!otPlatRadioIsEnabled(aInstance));

    sInstance = aInstance;

    /* TODO: CONVERT TO PD/PLME HANDLER */
    // PHY_Enable();

    sState = OT_RADIO_STATE_SLEEP;

exit:
    return OT_ERROR_NONE;
}

otError otPlatRadioDisable(otInstance *aInstance)
{
    otEXPECT(otPlatRadioIsEnabled(aInstance));

    /* TODO: CONVERT TO PD/PLME HANDLER */
    // PHY_Disable();
    sState = OT_RADIO_STATE_DISABLED;

exit:
    return OT_ERROR_NONE;
}

bool otPlatRadioIsEnabled(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return sState != OT_RADIO_STATE_DISABLED;
}

otError otPlatRadioSleep(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError status = OT_ERROR_NONE;

    otEXPECT_ACTION(((sState != OT_RADIO_STATE_TRANSMIT) && (sState != OT_RADIO_STATE_DISABLED)),
                    status = OT_ERROR_INVALID_STATE);

    if (sState != OT_RADIO_STATE_SLEEP)
    {
        rf_abort();
        sState = OT_RADIO_STATE_SLEEP;
    }

exit:
    return status;
}

otError otPlatRadioReceive(otInstance *aInstance, uint8_t aChannel)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError             status = OT_ERROR_NONE;
    phyStatus_t         phy_status;
    macToPlmeMessage_t *pMacToPlmeMsg;

    otEXPECT_ACTION(((sState != OT_RADIO_STATE_TRANSMIT) && (sState != OT_RADIO_STATE_DISABLED)),
                    status = OT_ERROR_INVALID_STATE);

    sState = OT_RADIO_STATE_RECEIVE;

    if (sChannel != aChannel)
    {
        rf_abort();
        rf_set_channel(aChannel);
        sRxFrame.mChannel = aChannel;
    }

    pMacToPlmeMsg = (macToPlmeMessage_t *)MSG_Alloc(sizeof(macToPlmeMessage_t));
    assert(pMacToPlmeMsg != NULL);

    pMacToPlmeMsg->macInstance = 0;
    pMacToPlmeMsg->msgType     = gPlmeSetReq_c;
    /* Ask Phy to start RX sequence on idle */
    pMacToPlmeMsg->msgData.setReq.PibAttribute      = gPhyPibRxOnWhenIdle;
    pMacToPlmeMsg->msgData.setReq.PibAttributeValue = (uint64_t)1;

    /* Send request to Phy */
    phy_status = MAC_PLME_SapHandler(pMacToPlmeMsg, 0);

    MSG_Free(pMacToPlmeMsg);

    if (phy_status != gPhySuccess_c)
    {
        status = OT_ERROR_INVALID_STATE;
    }

exit:
    return status;
}

void otPlatRadioEnableSrcMatch(otInstance *aInstance, bool aEnable)
{
    OT_UNUSED_VARIABLE(aInstance);

    /* TODO: CONVERT TO PD/PLME HANDLER */
    // PhyPpSetSAMState(aEnable);
}

otError otPlatRadioAddSrcMatchShortEntry(otInstance *aInstance, const uint16_t aShortAddress)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError  status = OT_ERROR_NONE;
    uint16_t addr   = aShortAddress;

    /* TODO: CONVERT TO PD/PLME HANDLER */
    /*if(0 != PhyAddToNeighborTable((uint8_t*)&addr, 2, sPanId))
    {
        status = OT_ERROR_NO_BUFS;
    }*/

    return status;
}

otError otPlatRadioAddSrcMatchExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError status = OT_ERROR_NONE;

    /* TODO: CONVERT TO PD/PLME HANDLER */
    /*if(0 != PhyAddToNeighborTable((uint8_t *)aExtAddress->m8, 3, sPanId))
    {
        status = OT_ERROR_NO_BUFS;
    }*/

    return status;
}

otError otPlatRadioClearSrcMatchShortEntry(otInstance *aInstance, const uint16_t aShortAddress)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError  status = OT_ERROR_NONE;
    uint16_t addr   = aShortAddress;

    /* TODO: CONVERT TO PD/PLME HANDLER */
    /*
    if(0 != PhyRemoveFromNeighborTable((uint8_t*)&addr, 2, sPanId))
    {
        status = OT_ERROR_NO_ADDRESS;
    }*/

    return status;
}

otError otPlatRadioClearSrcMatchExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError status = OT_ERROR_NONE;

    /* TODO: CONVERT TO PD/PLME HANDLER */
    /*if(0 != PhyRemoveFromNeighborTable((uint8_t *)aExtAddress->m8, 3, sPanId))
    {
        status = OT_ERROR_NO_ADDRESS;
    }*/

    return status;
}

void otPlatRadioClearSrcMatchShortEntries(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    /* TODO: CONVERT TO PD/PLME HANDLER */
    // PhyResetNeighborTable();
}

void otPlatRadioClearSrcMatchExtEntries(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    /* TODO: CONVERT TO PD/PLME HANDLER */
    // PhyResetNeighborTable();
}

otRadioFrame *otPlatRadioGetTransmitBuffer(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return &sTxFrame;
}

otError otPlatRadioTransmit(otInstance *aInstance, otRadioFrame *aFrame)
{
    otError               status = OT_ERROR_NONE;
    uint32_t              timeout;
    phyStatus_t           phy_status;
    macToPdDataMessage_t *pMacToPdDataMsg;

    otEXPECT_ACTION(((sState != OT_RADIO_STATE_TRANSMIT) && (sState != OT_RADIO_STATE_DISABLED)),
                    status = OT_ERROR_INVALID_STATE);

    rf_set_channel(aFrame->mChannel);

    pMacToPdDataMsg = (macToPdDataMessage_t *)MSG_Alloc(sizeof(macToPdDataMessage_t));
    assert(pMacToPdDataMsg != NULL);

    pMacToPdDataMsg->macInstance                 = 0;
    pMacToPdDataMsg->msgType                     = gPdDataReq_c;
    pMacToPdDataMsg->msgData.dataReq.slottedTx   = gPhyUnslottedMode_c;
    pMacToPdDataMsg->msgData.dataReq.startTime   = gPhySeqStartAsap_c;
    pMacToPdDataMsg->msgData.dataReq.txDuration  = 0xFFFFFFFFU;
    pMacToPdDataMsg->msgData.dataReq.psduLength  = aFrame->mLength;
    pMacToPdDataMsg->msgData.dataReq.pPsdu       = sTxFrame.mPsdu;
    pMacToPdDataMsg->msgData.dataReq.CCABeforeTx = DEFAULT_CCA_MODE;

    if (aFrame->mPsdu[IEEE802154_FRM_CTL_LO_OFFSET] & IEEE802154_ACK_REQUEST)
    {
        pMacToPdDataMsg->msgData.dataReq.ackRequired = gPhyRxAckRqd_c;
    }
    else
    {
        pMacToPdDataMsg->msgData.dataReq.ackRequired = gPhyNoAckRqd_c;
    }

    phy_status = MAC_PD_SapHandler(pMacToPdDataMsg, 0);

    MSG_Free(pMacToPdDataMsg);

    if (phy_status == gPhySuccess_c)
    {
        sTxStatus = OT_ERROR_NONE;
        sState    = OT_RADIO_STATE_TRANSMIT;
        otPlatRadioTxStarted(aInstance, aFrame);
    }
    else
    {
        status = OT_ERROR_INVALID_STATE;
    }

exit:
    return status;
}

int8_t otPlatRadioGetRssi(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    /* TODO: CONVERT TO PD/PLME HANDLER */
    // PhyPlmeGetRSSILevelRequest();
    return sLastRssi;
}

otRadioCaps otPlatRadioGetCaps(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return OT_RADIO_CAPS_ACK_TIMEOUT | OT_RADIO_CAPS_ENERGY_SCAN;
}

bool otPlatRadioGetPromiscuous(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    /* TODO: CONVERT TO PD/PLME HANDLER */
    // return PhyPlmeGetPromiscuousRequest();
    return false;
}

void otPlatRadioSetPromiscuous(otInstance *aInstance, bool aEnable)
{
    OT_UNUSED_VARIABLE(aInstance);

    macToPlmeMessage_t *pMacToPlmeMsg;

    pMacToPlmeMsg = (macToPlmeMessage_t *)MSG_Alloc(sizeof(macToPlmeMessage_t));
    assert(pMacToPlmeMsg != NULL);

    pMacToPlmeMsg->macInstance                      = 0;
    pMacToPlmeMsg->msgType                          = gPlmeSetReq_c;
    pMacToPlmeMsg->msgData.setReq.PibAttribute      = gPhyPibPromiscuousMode_c;
    pMacToPlmeMsg->msgData.setReq.PibAttributeValue = (uint64_t)aEnable;

    /* Send request to Phy */
    (void)MAC_PLME_SapHandler(pMacToPlmeMsg, 0);

    MSG_Free(pMacToPlmeMsg);
}

otError otPlatRadioEnergyScan(otInstance *aInstance, uint8_t aScanChannel, uint16_t aScanDuration)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError             status = OT_ERROR_NONE;
    phyTime_t           timeout;
    phyStatus_t         phy_status;
    macToPlmeMessage_t *pMacToPlmeMsg;

    otEXPECT_ACTION(((sState != OT_RADIO_STATE_TRANSMIT) && (sState != OT_RADIO_STATE_DISABLED)),
                    status = OT_ERROR_INVALID_STATE);

    /* TODO: CONVERT TO PD/PLME HANDLER */
    /*if (PhyPpGetState() != gIdle_c)
    {
        rf_abort();
    }*/

    sMaxED = -128;
    rf_set_channel(aScanChannel);

    /* This message will be freed when we get indication from Phy */
    pMacToPlmeMsg = (macToPlmeMessage_t *)MSG_Alloc(sizeof(macToPlmeMessage_t));
    assert(pMacToPlmeMsg != NULL);

    pMacToPlmeMsg->macInstance = 0;
    pMacToPlmeMsg->msgType     = gPlmeEdReq_c;
    /* TODO: add scan duration field to program scan timeout */
    pMacToPlmeMsg->msgData.edReq.startTime = gPhySeqStartAsap_c;

    phy_status = MAC_PLME_SapHandler(pMacToPlmeMsg, 0);

    MSG_Free(pMacToPlmeMsg);

    if (phy_status != gPhySuccess_c)
    {
        status = OT_ERROR_INVALID_STATE;
    }
    else
    {
        /* Set Scan time-out */
        /* TODO: see previous TODO */
        // timeout = PhyTime_GetTimestamp();
        // timeout += (aScanDuration * 1000) / OT_RADIO_SYMBOL_TIME;
        // PhyTimeSetEventTimeout(&timeout);
    }

exit:
    return status;
}

otError otPlatRadioGetTransmitPower(otInstance *aInstance, int8_t *aPower)
{
    otError error = OT_ERROR_NONE;

    otEXPECT_ACTION(aPower != NULL, error = OT_ERROR_INVALID_ARGS);
    *aPower = sAutoTxPwrLevel;

exit:
    return error;
}

otError otPlatRadioSetTransmitPower(otInstance *aInstance, int8_t aPower)
{
    OT_UNUSED_VARIABLE(aInstance);

    rf_set_tx_power(aPower);
    sAutoTxPwrLevel = aPower;

    return OT_ERROR_NONE;
}

otError otPlatRadioGetCcaEnergyDetectThreshold(otInstance *aInstance, int8_t *aThreshold)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aThreshold);

    return OT_ERROR_NOT_IMPLEMENTED;
}

otError otPlatRadioSetCcaEnergyDetectThreshold(otInstance *aInstance, int8_t aThreshold)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aThreshold);

    return OT_ERROR_NOT_IMPLEMENTED;
}

int8_t otPlatRadioGetReceiveSensitivity(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return -100;
}

/*************************************************************************************************/

static void rf_abort(void)
{
    macToPlmeMessage_t *pMacToPlmeMsg;

    pMacToPlmeMsg = (macToPlmeMessage_t *)MSG_Alloc(sizeof(macToPlmeMessage_t));
    assert(pMacToPlmeMsg != NULL);

    pMacToPlmeMsg->macInstance                  = 0;
    pMacToPlmeMsg->msgType                      = gPlmeSetTRxStateReq_c;
    pMacToPlmeMsg->msgData.setTRxStateReq.state = gPhyForceTRxOff_c;

    /* Send request to Phy */
    (void)MAC_PLME_SapHandler(pMacToPlmeMsg, 0);

    MSG_Free(pMacToPlmeMsg);
}

static void rf_set_channel(uint8_t channel)
{
    macToPlmeMessage_t *pMacToPlmeMsg;

    if (channel != sChannel)
    {
        pMacToPlmeMsg = (macToPlmeMessage_t *)MSG_Alloc(sizeof(macToPlmeMessage_t));
        assert(pMacToPlmeMsg != NULL);

        pMacToPlmeMsg->macInstance                      = 0;
        pMacToPlmeMsg->msgType                          = gPlmeSetReq_c;
        pMacToPlmeMsg->msgData.setReq.PibAttribute      = gPhyPibCurrentChannel_c;
        pMacToPlmeMsg->msgData.setReq.PibAttributeValue = (uint64_t)channel;

        /* Send request to Phy */
        (void)MAC_PLME_SapHandler(pMacToPlmeMsg, 0);

        MSG_Free(pMacToPlmeMsg);

        sChannel = channel;
    }
}

int8_t PhyConvertLQIToRSSI(uint8_t lqi)
{
    int32_t rssi = (36 * lqi - 9836) / 109;

    return (int8_t)rssi;
}

static void rf_set_tx_power(int8_t tx_power)
{
    macToPlmeMessage_t *pMacToPlmeMsg;

    pMacToPlmeMsg = (macToPlmeMessage_t *)MSG_Alloc(sizeof(macToPlmeMessage_t));
    assert(pMacToPlmeMsg != NULL);

    pMacToPlmeMsg->macInstance                      = 0;
    pMacToPlmeMsg->msgType                          = gPlmeSetReq_c;
    pMacToPlmeMsg->msgData.setReq.PibAttribute      = gPhyPibTransmitPower_c;
    pMacToPlmeMsg->msgData.setReq.PibAttributeValue = (uint64_t)tx_power;

    /* Send request to Phy */
    MAC_PLME_SapHandler(pMacToPlmeMsg, 0);

    MSG_Free(pMacToPlmeMsg);
}

/* Phy Data Service Access Point handler
 * Called by Phy to notify when Tx has been done or Rx data is available */
phyStatus_t PD_OT_MAC_SapHandler(void *pMsg, instanceId_t instance)
{
    pdDataToMacMessage_t *pDataMsg = (pdDataToMacMessage_t *)pMsg;
    OSA_SR_ALLOC();

    assert(pMsg != NULL);

    switch (pDataMsg->msgType)
    {
    case gPdDataCnf_c:
        /* TX activity is done */
        sTxStatus = OT_ERROR_NONE;
        sState    = OT_RADIO_STATE_RECEIVE;
        sTxDone   = true;
        break;
    case gPdDataInd_c:
        /* RX activity is done */
        sRxDone = true;
        /* Retrieve frame information and data */
        sRxFrame.mInfo.mRxInfo.mLqi = pDataMsg->msgData.dataInd.ppduLinkQuality;
        /* TODO: implemented the function locally to get something working
         *       but could be better to compute RSSI in Phy and include it in the PD data indication message
         */
        sRxFrame.mInfo.mRxInfo.mRssi      = PhyConvertLQIToRSSI(pDataMsg->msgData.dataInd.ppduLinkQuality);
        sRxFrame.mInfo.mRxInfo.mTimestamp = pDataMsg->msgData.dataInd.timeStamp;
        sRxFrame.mLength                  = pDataMsg->msgData.dataInd.psduLength;
        OSA_ENTER_CRITICAL();
        FLib_MemCpy(sRxFrame.mPsdu, pDataMsg->msgData.dataInd.pPsdu, sRxFrame.mLength);
        OSA_EXIT_CRITICAL();
        sLastRssi = sRxFrame.mInfo.mRxInfo.mRssi;
        break;
    default:
        break;
    }
    /* The message has been allocated by the Phy, we have to free it */
    MSG_Free(pMsg);

    otTaskletsSignalPending(NULL);
    return gPhySuccess_c;
}

/* Phy Layer Management Entities Service Access Point handler
 * Called by Phy to notify PLME event */
phyStatus_t PLME_OT_MAC_SapHandler(void *pMsg, instanceId_t instance)
{
    plmeToMacMessage_t *pPlmeMsg = (plmeToMacMessage_t *)pMsg;

    assert(pMsg != NULL);

    switch (pPlmeMsg->msgType)
    {
    case gPlmeCcaCnf_c:
        if (pPlmeMsg->msgData.ccaCnf.status == gPhyChannelBusy_c)
        {
            /* Channel is busy */
            sTxStatus = OT_ERROR_CHANNEL_ACCESS_FAILURE;
            sState    = OT_RADIO_STATE_RECEIVE;
            sTxDone   = true;
        }
        break;
    case gPlmeEdCnf_c:
        /* Scan done */
        sEdScanDone = true;
        sMaxED      = pPlmeMsg->msgData.edCnf.energyLeveldB;
        break;
    case gPlmeTimeoutInd_c:
        /* Ack timeout */
        sState    = OT_RADIO_STATE_RECEIVE;
        sTxStatus = OT_ERROR_NO_ACK;
        sTxDone   = true;
        break;
    default:
        break;
    }
    /* The message has been allocated by the Phy, we have to free it */
    MSG_Free(pMsg);

    otTaskletsSignalPending(NULL);
    return gPhySuccess_c;
}

void otPlatRadioInit(void)
{
    /* Init Phy */
    Phy_Init();
    /* Register Phy Data Service Access Point (PD_SAP) and Phy Layer Management Entities Service Access Point (PLME_SAP)
     * handlers */
    Phy_RegisterSapHandlers((PD_MAC_SapHandler_t)PD_OT_MAC_SapHandler, (PLME_MAC_SapHandler_t)PLME_OT_MAC_SapHandler,
                            0);

    rf_set_channel(sChannel);
    // Phy set default power at init, see gPhyDefaultTxPowerLevel_d
    // rf_set_tx_power(0);

    sTxFrame.mLength = 0;
    sTxFrame.mPsdu   = sTxData;
    sRxFrame.mLength = 0;
#if DOUBLE_BUFFERING
    sRxFrame.mPsdu = sRxData;
#endif
}

void otPlatRadioProcess(const otInstance *aInstance)
{
    if (sTxDone)
    {
        if (sTxFrame.mPsdu[IEEE802154_FRM_CTL_LO_OFFSET] & IEEE802154_ACK_REQUEST)
        {
            otPlatRadioTxDone((otInstance *)aInstance, &sTxFrame, &sRxFrame, sTxStatus);
        }
        else
        {
            otPlatRadioTxDone((otInstance *)aInstance, &sTxFrame, NULL, sTxStatus);
        }
        sTxDone = false;
    }

    if (sRxDone)
    {
        // TODO Set this flag only when the packet is really acknowledged with frame pending set.
        // See https://github.com/openthread/openthread/pull/3785
        sRxFrame.mInfo.mRxInfo.mAckedWithFramePending = true;

#if OPENTHREAD_CONFIG_DIAG_ENABLE

        if (otPlatDiagModeGet())
        {
            otPlatDiagRadioReceiveDone((otInstance *)aInstance, &sRxFrame, OT_ERROR_NONE);
        }
        else
#endif
        {
            otPlatRadioReceiveDone((otInstance *)aInstance, &sRxFrame, OT_ERROR_NONE);
        }

        sRxDone = false;
    }

    if (sEdScanDone)
    {
        otPlatRadioEnergyScanDone((otInstance *)aInstance, sMaxED);
        sEdScanDone = false;
    }
}
