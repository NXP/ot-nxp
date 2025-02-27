/*
 *  Copyright (c) 2017, The OpenThread Authors.
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

/* Openthread configuration */
#include OPENTHREAD_PROJECT_CORE_CONFIG_FILE

/* memcpy */
#include "string.h"

/* uMac, MMAC, Radio */
#include "MMAC.h"
#include "MicroSpecific_arm_sdk2.h"
#include "radio.h"

/* Openthread general */
#include "openthread-system.h"

#include <utils/code_utils.h>
#include <utils/encoding.h>
#include <utils/mac_frame.h>
#include "utils/link_metrics.h"

#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/diag.h>
#include <openthread/platform/radio.h>
#include <openthread/platform/time.h>

#if defined RADIO_LOG_ENABLED
#include "dbg_logging.h"
#define RADIO_LOG(fmt, ...)                                                    \
    do                                                                         \
    {                                                                          \
        DbgLogAdd(__FUNCTION__, fmt, VA_NUM_ARGS(__VA_ARGS__), ##__VA_ARGS__); \
    } while (0);
#else
#define RADIO_LOG(...)
#endif

#if OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE
#include "MWS.h"
#include "MacSched.h"
#include "dbg.h"
#endif
extern void BOARD_LedDongleToggle(void);
extern void OSA_InterruptEnable(void);
extern void OSA_InterruptDisable(void);
#if OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE
extern void BOARD_GetCoexIoCfg(void **rfDeny, void **rfActive, void **rfStatus);
#endif

/* Defines */
#define BIT_SET(arg, posn) ((arg) |= (1ULL << (posn)))
#define BIT_CLR(arg, posn) ((arg) &= ~(1ULL << (posn)))
#define BIT_TST(arg, posn) (!!((arg) & (1ULL << (posn))))

#define ALL_FFs_BYTE (0xFF)

#define K32W_RADIO_MIN_TX_POWER_DBM (-30)
#define K32W_RADIO_MAX_TX_POWER_DBM (15)

#define K32W_RADIO_RX_SENSITIVITY_DBM (-100)
#define K32W_RADIO_RX_MAX_DBM (10)

#define K32W_RADIO_DEFAULT_CHANNEL (11)

#define US_PER_SYMBOL (16) /* Duration of a single symbol in [us] */
#define SYMBOLS_TO_US(symbols) ((symbols)*US_PER_SYMBOL)
#define US_TO_MILI_DIVIDER (1000)

/* max number of SED children <= the size of address mask variable(s) */
#define MAX_FP_ADDRS MIN(OPENTHREAD_CONFIG_MLE_MAX_CHILDREN, 64)

#ifndef K32W0_RADIO_NUM_OF_RX_BUFS
#define K32W0_RADIO_NUM_OF_RX_BUFS (8) /* max number of RX buffers */
#endif

#if (K32W0_RADIO_NUM_OF_RX_BUFS) & (K32W0_RADIO_NUM_OF_RX_BUFS - 1)
#error "K32W0_RADIO_NUM_OF_RX_BUFS must be power of 2"
#endif

/* check IEEE Std. 802.15.4 - 2015: Table 8-81 - MAC sublayer constants */
#ifndef MAC_TX_RETRIES
#define MAC_TX_RETRIES (3)
#endif

#ifndef MAC_TX_CSMA_MIN_BE
#define MAC_TX_CSMA_MIN_BE (3)
#endif

#ifndef MAC_TX_CSMA_MAX_BE
#define MAC_TX_CSMA_MAX_BE (5)
#endif

#ifndef MAC_TX_CSMA_MAX_BACKOFFS
#define MAC_TX_CSMA_MAX_BACKOFFS (4)
#endif

/* Ensure MAC sublayer constants adhere to the spec ranges. */
#if (MAC_TX_RETRIES < 0) || (MAC_TX_RETRIES > 7)
#error "MAC_TX_RETRIES must be in range [0, 7]"
#endif

#if (MAC_TX_CSMA_MIN_BE < 0) || (MAC_TX_CSMA_MIN_BE > MAC_TX_CSMA_MAX_BE)
#error "MAC_TX_CSMA_MIN_BE must be in range [0, MAC_TX_CSMA_MAX_BE]"
#endif

#if (MAC_TX_CSMA_MAX_BE < 3) || (MAC_TX_CSMA_MAX_BE > 8)
#error "MAC_TX_CSMA_MAX_BE must be in range [3, 8]"
#endif

#if (MAC_TX_CSMA_MAX_BACKOFFS < 0) || (MAC_TX_CSMA_MAX_BACKOFFS > 5)
#error "MAC_TX_CSMA_MAX_BACKOFFS must be in range [0, 5]"
#endif

#define TX_TO 544 /* symbols. 2 max length frames + AIFS */

#define CSL_UNCERT 255 ///< The Uncertainty of the scheduling CSL of transmission by the parent, in ±10 us units.

/* RX was disabled due to no RX bufs */
#define OT_RADIO_STATE_RX_DISABLED ((otRadioState)(OT_RADIO_STATE_INVALID - 1))

#ifdef OT_RCP_TARGET
/* How long to delay RX done in certain cases */
#define DELAY_RX_DELTA 50000 /* usec */

#define DELAY_RX_CH_REVERT 0 /* usec */
#endif

#ifdef ANTENNA_DIVERSITY_ENABLE
/* Antenna Diversity feature requires enabling the ADO/ADE functions on the pins connected to the RF switch.
 * The K32W0x1 provides an output (ADO) on one of DIO7, DIO9 or DIO19 and optionally its complement (ADE) on
 * DIO6 that can be used to control an antenna switch; this enables antenna diversity to be implemented easily.
 */
#ifdef MAC_PROTO_TAG
#undef vMMAC_EnableAntennaDiversity
#undef vMMAC_DisableAntennaDiversity
#define vMMAC_EnableAntennaDiversity vMMAC_EnableAntennaDiversity
#define vMMAC_DisableAntennaDiversity vMMAC_DisableAntennaDiversity
void vMMAC_EnableAntennaDiversity(void);
void vMMAC_DisableAntennaDiversity(void);
#endif
#endif /* ANTENNA_DIVERSITY_ENABLE */

/* Use the au8Padding[] fields of the tsPhyFrame to keep some meta data about the received frame */
#define FMI_FP 0 /* ACKed with FP */
#define FMI_LM 1 /* Enh-ACKed with link metrics info */

/* Structures */
typedef struct
{
    uint16_t macAddress;
    uint16_t panId;
} fpNeighShortAddr;

typedef struct
{
    otExtAddress extAddr;
    uint16_t     panId;
} fpNeighExtAddr;

typedef struct
{
    tsPhyFrame   f;
    otRadioFrame of;
} rxRingBufferEntry;

typedef struct
{
    rxRingBufferEntry buffer[K32W0_RADIO_NUM_OF_RX_BUFS];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint16_t next;
    volatile uint16_t last;
    volatile uint16_t ovf_cnt;
} rxRingBuffer;

typedef enum
{
    kFcsSize             = sizeof(uint16_t),
    kDsnSize             = sizeof(uint8_t),
    kSecurityControlSize = sizeof(uint8_t),
    kFrameCounterSize    = sizeof(uint32_t),
    kKeyIndexSize        = sizeof(uint8_t),
    kMicSize             = sizeof(uint32_t),

    kMacFcfLowOffset = 0, /* Offset of FCF first byte inside Mac Hdr */
    kMacFrameDataReq = 4,

    kFcfTypeBeacon       = 0,
    kFcfTypeMacData      = 1,
    kFcfTypeAck          = 2,
    kFcfTypeMacCommand   = 3,
    kFcfFramePending     = 4,
    kFcfMacFrameTypeMask = 7 << 0,

    kFcfAckRequest        = 1 << 5,
    kFcfPanidCompression  = 1 << 6,
    kFcfSeqNbSuppresssion = 1 << 8,
    kFcfHasIe             = 1 << 9,
    kFcfDstAddrNone       = 0 << 10,
    kFcfDstAddrShort      = 2 << 10,
    kFcfDstAddrExt        = 3 << 10,
    kFcfDstAddrMask       = 3 << 10,
    kFcfSrcAddrNone       = 0 << 14,
    kFcfSrcAddrShort      = 2 << 14,
    kFcfSrcAddrExt        = 3 << 14,
    kFcfSrcAddrMask       = 3 << 14,

    kSecLevelMask            = 7 << 0,
    kFrameCounterSuppression = 1 << 5,

    kKeyIdMode0    = 0 << 3,
    kKeyIdMode1    = 1 << 3,
    kKeyIdMode2    = 2 << 3,
    kKeyIdMode3    = 3 << 3,
    kKeyIdModeMask = 3 << 3,

    kKeySourceSizeMode0 = 0,
    kKeySourceSizeMode1 = 0,
    kKeySourceSizeMode2 = 4,
    kKeySourceSizeMode3 = 8,
} macHdr;

typedef enum
{
    macToOtFrame, /* RX */
    otToMacFrame, /* TX */
} frameConversionType;

/* Private functions declaration */
static void K32WGetRxFrameInfo(rxRingBufferEntry *rbe);
static void K32WISR(uint32_t u32IntBitmap);
static void K32WProcessMacHeader(tsPhyFrame *aRxFrame);

static void K32WProcessRxFrames(otInstance *aInstance);
static void K32WProcessTxFrame(otInstance *aInstance);

static bool K32WCheckIfFpRequired(tsPhyFrame *aRxFrame);

static void K32WFrameConversion(tsPhyFrame *aPhyFrame, otRadioFrame *aOtFrame);

static void               K32WResetRxRingBuffer();
static void               K32WPushRxRingBuffer();
static void               K32WPopRxRingBuffer();
static rxRingBufferEntry *K32WGetRxRingBuffer();

static void K32WEnableReceive();

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
static void K32WEncFrame(void *t, const void *key);

#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
static uint8_t K32WGetVsIeLen(void *t);
static void    K32WGetVsIeGen(void *t, uint8_t *b);
#endif
#endif

/* Private variables declaration */
static volatile otRadioState sState = OT_RADIO_STATE_DISABLED;
static otInstance           *sInstance;    /* Saved OT Instance */
static int8_t                sTxPwrLevel;  /* Default power is 0 dBm */
static uint8_t               sChannel = 0; /* Default channel - must be invalid so it
                                              updates the first time it is set */
#ifdef OT_RCP_TARGET
static uint8_t sPrevRxChannel = 0; /* Previous channel - must be
                                      invalid so it is updated
                                      the first time it is set */
#endif                             /* OT_RCP_TARGET */
static bool_t sIsFpEnabled = TRUE; /* Enable address match so FP=0 for SED.
                                      Address match is disabled only when address table is full.
                                      See SourceMatchController::AddEntry() */
static uint16_t  sPanId;           /* PAN ID currently in use */
static uint16_t  sShortAddress;
static tsExtAddr sExtAddress;
static uint64_t  sCustomExtAddr = 0;

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
/* big endian version of extAddr for otMacFrameProcessTransmitAesCcm() */
static otExtAddress sRevExtAddr;
#endif

static fpNeighShortAddr sFpShortAddr[MAX_FP_ADDRS]; /* Frame Pending short addresses array */
static uint64_t         sFpShortAddrMask;           /* Mask - sFpShortAddr valid entries */

static fpNeighExtAddr sFpExtAddr[MAX_FP_ADDRS]; /* Frame Pending extended addresses array */
static uint64_t       sFpExtAddrMask;           /* Mask - sFpExtAddr is valid */

static rxRingBuffer sRxRing;                       /* Receive Ring Buffer */
static teRxOption   sRxOpt = E_MMAC_RX_START_NOW | /* RX Options */
                           E_MMAC_RX_ALIGN_NORMAL | E_MMAC_RX_USE_AUTO_ACK | E_MMAC_RX_NO_MALFORMED |
                           E_MMAC_RX_NO_FCS_ERROR | E_MMAC_RX_ADDRESS_MATCH;

static tsPhyFrame   sTxMacFrame; /* TX Frame */
static tsPhyFrame   sRxAckFrame; /* Frame used for keeping the ACK */
static otRadioFrame sAckOtFrame; /* Used for ACK frame conversion */

static bool_t       sRadioInitForLp    = FALSE;
static bool_t       sPromiscuousEnable = FALSE;
static bool_t       sTxDone;    /* TRUE if a TX frame was sent into the air */
static otError      sTxStatus;  /* Status of the latest TX operation */
static otRadioFrame sTxOtFrame; /* OT TX Frame to be send */

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
static uint32_t sMacFrameCounter;
static uint8_t  sKeyId;
#endif

#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
static uint32_t sCslPeriod;
#endif

/* tx/rx prevent low power mode, sleep allows it */
static bool_t sAllowDeviceToSleep = TRUE;

#if OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE
static bool isCoexInitialized;
#endif

#ifdef OT_RCP_TARGET
static bool     bDelayRx     = FALSE;
static uint32_t delayRxStart = 0;

static bool     bRevertRxChannel     = FALSE;
static uint32_t revertRxChannelStart = 0;
#endif /* OT_RCP_TARGET */

/* Stub functions for controlling low power mode */
WEAK void App_AllowDeviceToSleep();
WEAK void App_DisallowDeviceToSleep();
/* Stub functions for controlling LEDs on OT RCP USB dongle */
WEAK void BOARD_LedDongleToggle();

/**
 * Stub function used for controlling low power mode
 *
 */
WEAK void App_AllowDeviceToSleep()
{
}

/**
 * Stub function used for controlling low power mode
 *
 */
WEAK void App_DisallowDeviceToSleep()
{
}

/**
 * Stub functions for controlling LEDs on OT RCP USB dongle
 *
 */
WEAK void BOARD_LedDongleToggle()
{
}

#ifdef OT_RCP_TARGET
static inline bool bIsDelayExpired(uint32_t start, uint32_t delay, uint32_t curTime)
{
    return !(curTime - start < delay);
}
#endif /* OT_RCP_TARGET */

void App_SetCustomEui64(uint8_t *aIeeeEui64)
{
    memcpy((uint8_t *)&sCustomExtAddr, aIeeeEui64, sizeof(sCustomExtAddr));
}

void K32WRadioInit(void)
{
    /* RX initialization */
    for (int i = 0; i < K32W0_RADIO_NUM_OF_RX_BUFS; i++)
    {
        /* Both frames have the same payload */
        sRxRing.buffer[i].of.mPsdu = sRxRing.buffer[i].f.uPayload.au8Byte;
    }

    /* ACK frame initialization.
       Both frames have the same payload */
    sAckOtFrame.mPsdu = sRxAckFrame.uPayload.au8Byte;

    /* TX initialization.
       Both frames have the same payload */
    sTxOtFrame.mPsdu = sTxMacFrame.uPayload.au8Byte;
}

void K32WRadioProcess(otInstance *aInstance)
{
#ifdef OT_RCP_TARGET
    if (bRevertRxChannel &&
        bIsDelayExpired(revertRxChannelStart, DELAY_RX_CH_REVERT / US_PER_SYMBOL, u32MMAC_GetTime()))
    {
        otPlatRadioReceive(aInstance, sPrevRxChannel);
    }
#endif /* OT_RCP_TARGET */
    /* Try to send first the TX frames to the host for RCP scenario. */
    K32WProcessTxFrame(aInstance);

    K32WProcessRxFrames(aInstance);
}

otRadioState otPlatRadioGetState(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return sState;
}

void otPlatRadioGetIeeeEui64(otInstance *aInstance, uint8_t *aIeeeEui64)
{
    OT_UNUSED_VARIABLE(aInstance);

    if (0 == sCustomExtAddr)
    {
        tsExtAddr euiAddr;
        vMMAC_GetMacAddress(&euiAddr);

        memcpy(aIeeeEui64, &euiAddr.u32L, sizeof(uint32_t));
        memcpy(aIeeeEui64 + sizeof(uint32_t), &euiAddr.u32H, sizeof(uint32_t));
    }
    else
    {
        memcpy(aIeeeEui64, (uint8_t *)&sCustomExtAddr, sizeof(sCustomExtAddr));
    }
}

void otPlatRadioSetPanId(otInstance *aInstance, uint16_t aPanId)
{
    OT_UNUSED_VARIABLE(aInstance);

    sPanId = aPanId;
    vMMAC_SetRxPanId(aPanId);
}

void otPlatRadioSetExtendedAddress(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    OT_UNUSED_VARIABLE(aInstance);

    if (aExtAddress)
    {
        memcpy(&sExtAddress.u32L, aExtAddress->m8, sizeof(uint32_t));
        memcpy(&sExtAddress.u32H, aExtAddress->m8 + sizeof(uint32_t), sizeof(uint32_t));
        vMMAC_SetRxExtendedAddr(&sExtAddress);

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
        for (size_t i = 0; i < sizeof(*aExtAddress); i++)
        {
            sRevExtAddr.m8[i] = aExtAddress->m8[sizeof(*aExtAddress) - 1 - i];
        }
#endif
    }
}

void otPlatRadioSetShortAddress(otInstance *aInstance, uint16_t aShortAddress)
{
    OT_UNUSED_VARIABLE(aInstance);

    sShortAddress = aShortAddress;
    vMMAC_SetRxShortAddr(aShortAddress);
}

otError otPlatRadioEnable(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    sAllowDeviceToSleep = TRUE; /* synced with radio state */

    K32WResetRxRingBuffer();

    V2MMAC_Enable();
    V2MMAC_RegisterIntHandler(K32WISR);
    vMMAC_ConfigureRadio();

    /* Exit from low power and / or already initialized.
     * Cover the case when otPlatRadioSleep() / otPlatRadioEnable() is not used to enter / exit low power
     * but otPlatRadioDisable() / otPlatRadioEnable() because it's done from outside the OT stack.
     */
    if (sRadioInitForLp || sExtAddress.u32L || sExtAddress.u32H)
    {
        sRadioInitForLp = FALSE;

        /* Re-set modem settings after low power exit */
        vMMAC_SetChannelAndPower(sChannel, sTxPwrLevel);
        vMMAC_SetRxExtendedAddr(&sExtAddress);
        vMMAC_SetRxPanId(sPanId);
        vMMAC_SetRxShortAddr(sShortAddress);
    }

#ifdef ANTENNA_DIVERSITY_ENABLE
    /* Enabling this feature requires setting correct pin config to ADE/ADO pins */
    vMMAC_EnableAntennaDiversity();
#endif

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
    /* Frame encryption is done in radio.c/OT stack, for now.
       Since there is no encryption support in MAC. */
    V2MMAC_RegisterEncFn(K32WEncFrame);

#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
    V2MMAC_RegisterEnhAckVsIeFn(K32WGetVsIeLen, K32WGetVsIeGen);
#endif
#endif

    sInstance = aInstance;
    sState    = OT_RADIO_STATE_SLEEP;

    return OT_ERROR_NONE;
}

otError otPlatRadioDisable(otInstance *aInstance)
{
    otError error = OT_ERROR_INVALID_STATE;

    otEXPECT(otPlatRadioIsEnabled(aInstance));

#ifdef ANTENNA_DIVERSITY_ENABLE
    vMMAC_DisableAntennaDiversity();
#endif

    /* stop the radio so there are no pending interrupts */
    vMMAC_RadioToOffAndWait();

    sState = OT_RADIO_STATE_DISABLED;

    K32WResetRxRingBuffer();

    vMMAC_Disable();
    error = OT_ERROR_NONE;

exit:
    return error;
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

    /* The radio has been init and configuration should be restored in otPlatRadioEnable when
       exiting low power */
    sRadioInitForLp = TRUE;

    /* stop the radio so there are no pending interrupts */
    vMMAC_RadioToOffAndWait();

    sState = OT_RADIO_STATE_SLEEP;

    /* prevent multiple calls to the allow to sleep callback */
    if (FALSE == sAllowDeviceToSleep)
    {
        App_AllowDeviceToSleep();
        sAllowDeviceToSleep = TRUE;
        RADIO_LOG("App_AllowDeviceToSleep");
    }

exit:
    return status;
}

otError otPlatRadioReceive(otInstance *aInstance, uint8_t aChannel)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError  error  = OT_ERROR_NONE;
    uint32_t txTime = 0;

    otEXPECT_ACTION(((sState != OT_RADIO_STATE_TRANSMIT) && (sState != OT_RADIO_STATE_DISABLED)),
                    error = OT_ERROR_INVALID_STATE);

    /* Already in Rx on the same channel */
    otEXPECT((sChannel != aChannel) || ((OT_RADIO_STATE_RECEIVE != sState) && (OT_RADIO_STATE_RX_DISABLED != sState)));

    /* wait for Rx to finish */
    txTime = u32MMAC_GetTime();

    while (v2MAC_is_rx_ongoing() && ((u32MMAC_GetTime() - txTime) < TX_TO))
    {
    }
#ifdef OT_RCP_TARGET
    /* disarm channel change mechanism */
    if (bRevertRxChannel)
    {
        bRevertRxChannel     = FALSE;
        revertRxChannelStart = 0;
    }
#endif
    /* stop the radio so there are no pending interrupts */
    vMMAC_RadioToOffAndWait();

    sState = OT_RADIO_STATE_RECEIVE;

    /* prevent multiple calls to the allow to sleep callback */
    if (TRUE == sAllowDeviceToSleep)
    {
        App_DisallowDeviceToSleep();
        sAllowDeviceToSleep = FALSE;
        RADIO_LOG("App_DisallowDeviceToSleep");
    }
#ifdef OT_RCP_TARGET
    sPrevRxChannel =
#endif
        sChannel = aChannel;
    vMMAC_SetChannelAndPower(sChannel, sTxPwrLevel);
    K32WEnableReceive();

exit:
    return error;
}

void otPlatRadioEnableSrcMatch(otInstance *aInstance, bool aEnable)
{
    OT_UNUSED_VARIABLE(aInstance);

    sIsFpEnabled = aEnable;
}

otError otPlatRadioAddSrcMatchShortEntry(otInstance *aInstance, const uint16_t aShortAddress)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError error = OT_ERROR_NO_BUFS;
    uint8_t idx   = 0;

    for (; idx < MAX_FP_ADDRS; idx++)
    {
        if (!BIT_TST(sFpShortAddrMask, idx))
        {
            sFpShortAddr[idx].panId      = sPanId;
            sFpShortAddr[idx].macAddress = aShortAddress;
            BIT_SET(sFpShortAddrMask, idx);
            error = OT_ERROR_NONE;
            break;
        }
    }

    return error;
}

otError otPlatRadioAddSrcMatchExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError      error = OT_ERROR_NO_BUFS;
    uint8_t      idx   = 0;
    otExtAddress tmp;

    /* K32WCheckIfFpRequired() uses reversed addresses (big endian) */
    for (size_t i = 0; i < sizeof(*aExtAddress); i++)
    {
        tmp.m8[i] = aExtAddress->m8[sizeof(*aExtAddress) - 1 - i];
    }

    for (; idx < MAX_FP_ADDRS; idx++)
    {
        if (!BIT_TST(sFpExtAddrMask, idx))
        {
            sFpExtAddr[idx].panId   = sPanId;
            sFpExtAddr[idx].extAddr = tmp;
            BIT_SET(sFpExtAddrMask, idx);
            error = OT_ERROR_NONE;
            break;
        }
    }

    return error;
}

otError otPlatRadioClearSrcMatchShortEntry(otInstance *aInstance, const uint16_t aShortAddress)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError error = OT_ERROR_NO_ADDRESS;
    uint8_t idx   = 0;

    for (; idx < MAX_FP_ADDRS; idx++)
    {
        if (BIT_TST(sFpShortAddrMask, idx) && (sFpShortAddr[idx].macAddress == aShortAddress))
        {
            BIT_CLR(sFpShortAddrMask, idx);
            error = OT_ERROR_NONE;
            break;
        }
    }

    return error;
}

otError otPlatRadioClearSrcMatchExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError      error = OT_ERROR_NO_ADDRESS;
    uint8_t      idx   = 0;
    otExtAddress tmp;

    /* K32WCheckIfFpRequired() uses reversed addresses (big endian) */
    for (size_t i = 0; i < sizeof(*aExtAddress); i++)
    {
        tmp.m8[i] = aExtAddress->m8[sizeof(*aExtAddress) - 1 - i];
    }

    uint64_t v = otEncodingReadUint64Le(tmp.m8);

    for (; idx < MAX_FP_ADDRS; idx++)
    {
        uint64_t t = otEncodingReadUint64Le(sFpExtAddr[idx].extAddr.m8);

        if (BIT_TST(sFpExtAddrMask, idx) && (t == v))
        {
            BIT_CLR(sFpExtAddrMask, idx);
            error = OT_ERROR_NONE;
            break;
        }
    }

    return error;
}

void otPlatRadioClearSrcMatchShortEntries(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    sFpShortAddrMask = 0;
}

void otPlatRadioClearSrcMatchExtEntries(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    sFpExtAddrMask = 0;
}

otRadioFrame *otPlatRadioGetTransmitBuffer(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return &sTxOtFrame;
}

otError otPlatRadioTransmit(otInstance *aInstance, otRadioFrame *aFrame)
{
    otError    error    = OT_ERROR_NONE;
    teTxOption eOptions = E_MMAC_TX_USE_AUTO_ACK;
    uint32_t   txTime   = 0;

    otEXPECT_ACTION((OT_RADIO_STATE_SLEEP == sState) || (OT_RADIO_STATE_RECEIVE == sState) ||
                        (OT_RADIO_STATE_RX_DISABLED == sState),
                    error = OT_ERROR_INVALID_STATE);

    /* wait for Rx to finish */
    txTime = u32MMAC_GetTime();
    while (v2MAC_is_rx_ongoing() && ((u32MMAC_GetTime() - txTime) < TX_TO))
    {
    }
    txTime = 0;
#ifdef OT_RCP_TARGET
    /* disarm channel change mechanism */
    if (bRevertRxChannel)
    {
        bRevertRxChannel     = FALSE;
        revertRxChannelStart = 0;
    }
#endif
    /* stop the radio so there are no pending interrupts */
    vMMAC_RadioToOffAndWait();

    /* go to TX state */
    sState    = OT_RADIO_STATE_TRANSMIT;
    sTxStatus = OT_ERROR_NONE;

    /* prevent multiple calls to the allow to sleep callback */
    if (TRUE == sAllowDeviceToSleep)
    {
        App_DisallowDeviceToSleep();
        sAllowDeviceToSleep = FALSE;
        RADIO_LOG("App_DisallowDeviceToSleep");
    }

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
    if (otMacFrameIsSecurityEnabled(aFrame) && otMacFrameIsKeyIdMode1(aFrame) &&
        !aFrame->mInfo.mTxInfo.mIsSecurityProcessed)
    {
        /* Encryption is done by MAC */
        eOptions |= E_MMAC_TX_ENC;

        if (!aFrame->mInfo.mTxInfo.mIsHeaderUpdated)
        {
            /* the header can be updated with frame counter, keyId and CSL IE */
            eOptions |= E_MMAC_TX_HDR_UPD;
        }
    }

    if (aFrame->mInfo.mTxInfo.mTxDelay)
    {
        eOptions |= E_MMAC_TX_USE_CCA | E_MMAC_TX_DELAY_START;

        /* txTime is in the future. Convert it to symbol time */
        txTime = aFrame->mInfo.mTxInfo.mTxDelay + aFrame->mInfo.mTxInfo.mTxDelayBaseTime;
        txTime = u32MMAC_GetTime() + (txTime - (uint32_t)otPlatTimeGet()) / US_PER_SYMBOL;
    }
    else
#endif
    {
        eOptions |= E_MMAC_TX_START_NOW;
    }

    /* set tx channel */
    if (sChannel != aFrame->mChannel)
    {
#if OT_RCP_TARGET
        /* preserve current RX channel */
        sPrevRxChannel = sChannel;
#endif
        /* after tx ends, rx on the same channel */
        sChannel = aFrame->mChannel;

        vMMAC_SetChannelAndPower(aFrame->mChannel, sTxPwrLevel);
    }

    if (aFrame->mInfo.mTxInfo.mCsmaCaEnabled)
    {
        eOptions |= E_MMAC_TX_USE_CCA;
    }

    /* No retransmissions, just 1 CCA */
    vMMAC_SetTxParameters(1, 0, 0, 0);

    /* frame conversion. aOtFrame is sTxOtFrame */
    sTxMacFrame.u8PayloadLength = aFrame->mLength - kFcsSize;

    if (eOptions & E_MMAC_TX_ENC)
    {
        sTxMacFrame.u8PayloadLength -= kMicSize;
    }

    /* Set RX buffer pointer for ACK */
    vMMAC_SetRxFrame((tsRxFrameFormat *)&sRxAckFrame);

    /* Status notification is received via K32WISR()  */
    vMMAC_StartV2MacTransmit(&sTxMacFrame, eOptions, txTime);

    if (eOptions & E_MMAC_TX_ENC)
    {
        /* the frames is always encrypted by MAC */
        aFrame->mInfo.mTxInfo.mIsSecurityProcessed = true;

        if (eOptions & E_MMAC_TX_HDR_UPD)
        {
            /* the header is always updated by MAC */
            aFrame->mInfo.mTxInfo.mIsHeaderUpdated = true;

            sMacFrameCounter++; /* get it from MAC? */
        }
    }

    otPlatRadioTxStarted(aInstance, aFrame);

exit:
    return error;
}

int8_t otPlatRadioGetRssi(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    int8_t  rssidBm       = OT_RADIO_RSSI_INVALID;
    int16_t rssiValSigned = 0;
    bool_t  stateChanged  = FALSE;

    otEXPECT((sState == OT_RADIO_STATE_SLEEP) || (sState == OT_RADIO_STATE_RECEIVE) ||
             (sState == OT_RADIO_STATE_RX_DISABLED));

    /* in RCP designs, the RSSI function is called while the radio is in
     * OT_RADIO_STATE_RECEIVE. Turn off the radio before reading RSSI,
     * otherwise we may end up waiting until a packet is received
     * (in i16MMAC_GetRSSI, while loop)
     */

    OSA_InterruptDisable();

    if ((sState == OT_RADIO_STATE_RECEIVE) || (sState == OT_RADIO_STATE_RX_DISABLED))
    {
        /* stop the radio so there are no pending interrupts */
        vMMAC_RadioToOffAndWait();

        sState       = OT_RADIO_STATE_SLEEP;
        stateChanged = TRUE;
    }

    rssiValSigned = i16MMAC_GetRSSI();

    if (stateChanged)
    {
        sState = OT_RADIO_STATE_RECEIVE;
        K32WEnableReceive();
    }

    OSA_InterruptEnable();

    otEXPECT((rssiValSigned != MMAC_INVALID_RSSI));

    rssiValSigned = i16Radio_BoundRssiValue(rssiValSigned);

    /* RSSI reported by radio is in 1/4 dBm step,
     * meaning values are 4 times larger than real dBm value.
     */
    rssidBm = (int8_t)(rssiValSigned >> 2);

exit:
    return rssidBm;
}

otRadioCaps otPlatRadioGetCaps(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return OT_RADIO_CAPS_ACK_TIMEOUT |
           OT_RADIO_CAPS_SLEEP_TO_TX
#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
           /* MAC doesn't support enc/dec. It uses K32WEncFrame() callback */
           | OT_RADIO_CAPS_TRANSMIT_SEC | OT_RADIO_CAPS_TRANSMIT_TIMING
#endif
        ;
}

bool otPlatRadioGetPromiscuous(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return sPromiscuousEnable;
}

void otPlatRadioSetPromiscuous(otInstance *aInstance, bool aEnable)
{
    OT_UNUSED_VARIABLE(aInstance);

    if (sPromiscuousEnable != aEnable)
    {
        sPromiscuousEnable = aEnable;

        if (aEnable)
        {
            sRxOpt &= ~E_MMAC_RX_ADDRESS_MATCH;
        }
        else
        {
            sRxOpt |= E_MMAC_RX_ADDRESS_MATCH;
        }
    }
}

otError otPlatRadioEnergyScan(otInstance *aInstance, uint8_t aScanChannel, uint16_t aScanDuration)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError status = OT_ERROR_NOT_IMPLEMENTED;

    return status;
}

otError otPlatRadioGetTransmitPower(otInstance *aInstance, int8_t *aPower)
{
    otError error = OT_ERROR_NONE;
    otEXPECT_ACTION(aPower != NULL, error = OT_ERROR_INVALID_ARGS);

    *aPower = i8Radio_GetTxPowerLevel_dBm();
    return OT_ERROR_NONE;

exit:
    return error;
}

otError otPlatRadioSetTransmitPower(otInstance *aInstance, int8_t aPower)
{
    OT_UNUSED_VARIABLE(aInstance);
    otError status = OT_ERROR_NONE;

    otEXPECT_ACTION(((sState != OT_RADIO_STATE_TRANSMIT) && (sState != OT_RADIO_STATE_DISABLED)),
                    status = OT_ERROR_INVALID_STATE);

    /* stop the radio so there are no pending interrupts */
    vMMAC_RadioToOffAndWait();

    /* trim the values to the radio capabilities */
    if (aPower < K32W_RADIO_MIN_TX_POWER_DBM)
    {
        aPower = K32W_RADIO_MIN_TX_POWER_DBM;
    }
    else if (aPower > K32W_RADIO_MAX_TX_POWER_DBM)
    {
        aPower = K32W_RADIO_MAX_TX_POWER_DBM;
    }

    /* save for later use */
    sTxPwrLevel = aPower;

    /* The state is set to sleep to prevent a lockup caused by an RX interrup firing during
     * the radio off command called inside set channel and power */
    if (0 != sChannel)
    {
        vMMAC_SetChannelAndPower(sChannel, aPower);
    }
    else
    {
        /* if the channel has not yet been initialized use K32W_RADIO_DEFAULT_CHANNEL as default */
        vMMAC_SetChannelAndPower(K32W_RADIO_DEFAULT_CHANNEL, aPower);
    }

    if ((sState == OT_RADIO_STATE_RECEIVE) || (sState == OT_RADIO_STATE_RX_DISABLED))
    {
        sState = OT_RADIO_STATE_RECEIVE;
        K32WEnableReceive();
    }

exit:
    return status;
}

otError otPlatRadioGetCcaEnergyDetectThreshold(otInstance *aInstance, int8_t *aThreshold)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError error = OT_ERROR_NONE;
    uint8_t edThreshold;

    if (aThreshold == NULL)
    {
        error = OT_ERROR_INVALID_ARGS;
    }
    else
    {
        /* MMAC CCA API returns ED, need to convert to dBm
         * RSSI in K32W0 is 10 bit wide (8.2 format) with 0.25dBm steps.
         * Need to shift to right by 2 to get the whole part.
         */
        edThreshold = u8MMAC_ReadCcaThreshold();
        *aThreshold = (int8_t)(i16Radio_GetRSSIfromED(edThreshold) >> 2);
    }

    return error;
}

otError otPlatRadioSetCcaEnergyDetectThreshold(otInstance *aInstance, int8_t aThreshold)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError error = OT_ERROR_NONE;
    uint8_t edThreshold;

    /* K32W0 RSSI between -100 and 10 dBm */
    if ((aThreshold < K32W_RADIO_RX_SENSITIVITY_DBM) || (aThreshold > K32W_RADIO_RX_MAX_DBM))
    {
        error = OT_ERROR_INVALID_ARGS;
    }
    else
    {
        /* MMAC CCA API requires ED, need to convert from dBm
         * RSSI in K32W0 is 10 bit wide (8.2) with 0.25dBm steps.
         * Need to left shift aThreshold by 2.
         */
        edThreshold = u8Radio_GetEDfromRSSI(((int16_t)aThreshold) << 2);
        vMMAC_WriteCcaThreshold(edThreshold);
    }

    return error;
}

int8_t otPlatRadioGetReceiveSensitivity(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return K32W_RADIO_RX_SENSITIVITY_DBM;
}

/**
 * Interrupt service routine (e.g.: TX/RX/MAC HDR received)
 *
 * @param[in] u32IntBitmap  Bitmap telling which interrupt fired
 *
 */
static void K32WISR(uint32_t u32IntBitmap)
{
    rxRingBufferEntry *rbe = NULL;

    switch (sState)
    {
    case OT_RADIO_STATE_RECEIVE:

        if (u32IntBitmap & E_MMAC_INT_RX_HEADER)
        {
            /* This event doesn't mean end of reception */

            /* go back one index from current frame index */
            rbe = &sRxRing.buffer[sRxRing.next];

            /* FP processing first */
            K32WProcessMacHeader(&rbe->f);
        }

        if (u32IntBitmap & E_MMAC_INT_RX_COMPLETE)
        {
            /* no rx errors */
            if (0 == u32V2MAC_GetRxErrors())
            {
                /* go back one index from current frame index */
                rbe = &sRxRing.buffer[sRxRing.next];

                /* Get rx info for frame */
                K32WGetRxFrameInfo(rbe);

                /* RX interrupt fired so it's safe to consume the frame */
                K32WPushRxRingBuffer();

#ifdef OT_RCP_TARGET
                if (bRevertRxChannel)
                {
                    bRevertRxChannel     = FALSE;
                    revertRxChannelStart = 0;
                }
#endif /* OT_RCP_TARGET */
            }

            /* restart RX */
            K32WEnableReceive();
        }

        BOARD_LedDongleToggle();
        break;
    case OT_RADIO_STATE_TRANSMIT:

        if (u32IntBitmap & E_MMAC_INT_TX_COMPLETE)
        {
            uint32_t txErrors = u32V2MAC_GetTxErrors();

            if (txErrors & E_MMAC_TXSTAT_CCA_BUSY)
            {
                sTxStatus = OT_ERROR_CHANNEL_ACCESS_FAILURE;
            }
            else if (txErrors & E_MMAC_TXSTAT_NO_ACK)
            {
                sTxStatus = OT_ERROR_NO_ACK;
            }
            else if (txErrors & E_MMAC_TXSTAT_ABORTED)
            {
                sTxStatus = OT_ERROR_ABORT;
            }
            else if ((txErrors & E_MMAC_TXSTAT_TXPCTO) || (txErrors & E_MMAC_TXSTAT_TXTO))
            {
                /* The JN518x/K32W0x1 has a TXTO timeout.
                   Describe failure as a CCA failure for onward processing */
                sTxStatus = OT_ERROR_CHANNEL_ACCESS_FAILURE;
            }
            else
            {
                /* No error, convert ACK */
                K32WFrameConversion(&sRxAckFrame, &sAckOtFrame);
                sAckOtFrame.mChannel = sTxOtFrame.mChannel; /* ACK channel */
            }

            /* Tx finished */
            sTxDone = TRUE;

            /* go to RX and restore channel */
            if (sChannel != sTxOtFrame.mChannel)
            {
                vMMAC_SetChannelAndPower(sChannel, sTxPwrLevel);
            }
#ifdef OT_RCP_TARGET
            if ((sTxStatus == OT_ERROR_NONE) && otMacFrameIsVersion2015(&sAckOtFrame) &&
                otMacFrameIsSecurityEnabled(&sAckOtFrame) &&
                ((*(uint16_t *)&sAckOtFrame.mPsdu[kMacFcfLowOffset]) & kFcfHasIe) && !bDelayRx)
            {
                bDelayRx     = TRUE;
                delayRxStart = 0;
            }

            /* If channels were just switched, then maybe I should prepare to go back */
            if (sPrevRxChannel != sTxOtFrame.mChannel)
            {
                /* TODO: From George: revert condition below to make it easier to understand */
                if ((sTxStatus != OT_ERROR_NONE) ||
                    !(otMacFrameIsDataRequest(&sTxOtFrame) && (sAckOtFrame.mPsdu[kMacFcfLowOffset] & kFcfFramePending)))
                {
                    bRevertRxChannel     = TRUE;
                    revertRxChannelStart = u32MMAC_GetTime();
                }
            }
#endif /* OT_RCP_TARGET */
            BOARD_LedDongleToggle();
            sState = OT_RADIO_STATE_RECEIVE;
            K32WEnableReceive();
        }
        break;

    default:
        break;
    }

    otSysEventSignalPending();
}
/**
 * Process the MAC Header of the latest received packet
 * We are in interrupt context - we need to compute the FP
 * while the hardware generates the ACK.
 *
 * @param[in] aRxFrame     Pointer to the latest received MAC packet
 *
 */
static void K32WProcessMacHeader(tsPhyFrame *aRxFrame)
{
    /* Make sure this is set to 0 when we are not dealing with a data request */
    aRxFrame->au8Padding[FMI_FP] = 0;
    aRxFrame->au8Padding[FMI_LM] = 0;

    /* check if frame pending processing is required */
    if (!aRxFrame)
        return;

    bool isFpRequired = K32WCheckIfFpRequired(aRxFrame);

    vMMAC_SetTxPend(isFpRequired);

    /* use the unused filed to store if the frame was ack'ed with FP and report this back to OT stack */
    aRxFrame->au8Padding[FMI_FP] = isFpRequired;
}

/**
 * Check if Frame Pending was requested by aPsRxFrame
 * We are in interrupt context.
 *
 * @param[in] aRxFrame  Pointer to a Data Request Frame
 *
 * @return    TRUE 	    Frame Pending bit should be set in the reply
 * @return    FALSE	    Frame Pending bit shouldn't be set in the reply
 *
 */
static bool K32WCheckIfFpRequired(tsPhyFrame *aRxFrame)
{
    bool         isFpRequired = FALSE;
    uint8_t      idx          = 0;
    otRadioFrame f;
    otMacAddress srcAddr;

    f.mPsdu   = aRxFrame->uPayload.au8Byte;
    f.mLength = aRxFrame->u8PayloadLength;

    if (!(
#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
            /* Enhanced frame pending */
            (otMacFrameIsVersion2015(&f) && otMacFrameIsCommand(&f)) || otMacFrameIsData(&f) ||
#endif
            otMacFrameIsDataRequest(&f)))
    {
        return FALSE;
    }

    if (!sIsFpEnabled)
    {
        return TRUE;
    }

    if (otMacFrameGetSrcAddr(&f, &srcAddr))
    {
        return FALSE;
    }

    if (srcAddr.mType == OT_MAC_ADDRESS_TYPE_NONE)
    {
        isFpRequired = TRUE;
    }
    else if (srcAddr.mType == OT_MAC_ADDRESS_TYPE_SHORT)
    {
        for (idx = 0; idx < MAX_FP_ADDRS; idx++)
        {
            if (BIT_TST(sFpShortAddrMask, idx) && (sFpShortAddr[idx].macAddress == srcAddr.mAddress.mShortAddress))
            {
                isFpRequired = TRUE;
                break;
            }
        }
    }
    else if (srcAddr.mType == OT_MAC_ADDRESS_TYPE_EXTENDED)
    {
        /* srcAddr.mAddress.mExtAddress is returned in reverse order (big endian) */
        uint64_t v = otEncodingReadUint64Le(srcAddr.mAddress.mExtAddress.m8);

        for (idx = 0; idx < MAX_FP_ADDRS; idx++)
        {
            uint64_t t = otEncodingReadUint64Le(sFpExtAddr[idx].extAddr.m8);

            if (BIT_TST(sFpExtAddrMask, idx) && (t == v))
            {
                isFpRequired = TRUE;
                break;
            }
        }
    }

    return isFpRequired;
}

#ifdef OT_RCP_TARGET
static inline bool bForwardRxFrameToHost(rxRingBufferEntry *rbe)
{
    bool bFwdRxF = TRUE;
    /*
     * Signaled from TX that we've got a pending TX Done with Ack to
     * be reported to the host
     */
    if (bDelayRx == TRUE)
    {
        /* ... but we don't have a timestamp set yet */
        if (delayRxStart == 0)
        {
            /*
             * We received a frame that is delivered out of order (due to being
             * a notification and being processed before confirmations),
             * we will affect the security counter, leading to the failing of
             * the TX confirmation (coming in later).
             *
             * Note: this filtering is quite coarse and might have some negative effects.
             */
            if (otMacFrameIsVersion2015(&rbe->of) && otMacFrameIsAckRequested(&rbe->of) &&
                otMacFrameIsSecurityEnabled(&rbe->of) && (*(uint16_t *)&rbe->of.mPsdu[kMacFcfLowOffset] & kFcfHasIe))
            {
                /* Delay RX for some time to allow the host to "see" the previous TX */
                delayRxStart = u32MMAC_GetTime();

                /*
                 * u32MMAC_GetTime() can return 0 (f.i. on wrap-around). Increment it by one here
                 * so the condition above isn't triggered wrongly.
                 */
                if (delayRxStart == 0)
                {
                    delayRxStart++;
                }
                bFwdRxF = FALSE;
            }
            else
            {
                /* This is a corner case where before the corresponding response to the CSL TX there's another frame
                   coming in. We don't want to delay that, so we will deliver it. */
            }
        }
        else
        {
            if (bIsDelayExpired(delayRxStart, DELAY_RX_DELTA / US_PER_SYMBOL, u32MMAC_GetTime()))
            {
                /* Cool-off has elapsed, start delivering frames upstream */
                bDelayRx = FALSE;
            }
            else
            {
                /* return FALSE, so we'll get out of the loop, nothing to do here... */
                /*
                 * Beware that this equates with a head of line blocking of all
                 * the RX frames: after the first "culprit" is received, everything else
                 * will be delayed.
                 */
                bFwdRxF = FALSE;
            }
        }
    }

    return bFwdRxF;
}
#endif

/**
 * Process RX frames in process context and call the upper layer call-backs
 *
 * @param[in] aInstance  Pointer to OT instance
 */
static void K32WProcessRxFrames(otInstance *aInstance)
{
    rxRingBufferEntry *rbe = NULL;

    while ((rbe = K32WGetRxRingBuffer()) != NULL)
    {
#ifdef OT_RCP_TARGET
        if (bForwardRxFrameToHost(rbe) == FALSE)
        {
            break;
        }
#endif /* OT_RCP_TARGET */
        otPlatRadioReceiveDone(aInstance, &rbe->of, OT_ERROR_NONE);

        K32WPopRxRingBuffer();

        if (sState == OT_RADIO_STATE_RX_DISABLED)
        {
            sState = OT_RADIO_STATE_RECEIVE;
            K32WEnableReceive();
        }
    }

    if (sState == OT_RADIO_STATE_RX_DISABLED)
    {
        sState = OT_RADIO_STATE_RECEIVE;
        K32WEnableReceive();
    }
}

/**
 * Process TX frame in process context and call the upper layer call-backs
 *
 * @param[in] aInstance  Pointer to OT instance
 */
static void K32WProcessTxFrame(otInstance *aInstance)
{
    if (sTxDone)
    {
        sTxDone = FALSE;
        if ((sTxOtFrame.mPsdu[kMacFcfLowOffset] & kFcfAckRequest) && (OT_ERROR_NONE == sTxStatus))
        {
            otPlatRadioTxDone(aInstance, &sTxOtFrame, &sAckOtFrame, sTxStatus);
        }
        else
        {
            otPlatRadioTxDone(aInstance, &sTxOtFrame, NULL, sTxStatus);
        }
    }
}

static void K32WGetRxFrameInfo(rxRingBufferEntry *rbe)
{
    memset(&rbe->of.mInfo.mRxInfo, 0, sizeof(rbe->of.mInfo.mRxInfo));

    /* must be called after Rx complete to get correct values from MAC */
    K32WFrameConversion(&rbe->f, &rbe->of);

    rbe->of.mChannel                             = sChannel; /* Rx channel */
    rbe->of.mInfo.mRxInfo.mAckedWithFramePending = (bool)rbe->f.au8Padding[FMI_FP];

#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE || OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
    bool_t security_check = FALSE;

#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    security_check = !!sCslPeriod || security_check;
#endif

#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
    security_check = !!rbe->f.au8Padding[FMI_LM] || security_check;
#endif

    if (security_check)
    {
        uint8_t keyId;

        if (otMacFrameIsVersion2015(&rbe->of) && otMacFrameIsAckRequested(&rbe->of) &&
            otMacFrameIsSecurityEnabled(&rbe->of) && otMacFrameIsKeyIdMode1(&rbe->of) &&
            ((keyId = otMacFrameGetKeyId(&rbe->of)) != 0) &&
            ((keyId == sKeyId) || (keyId == sKeyId - 1) || (keyId == sKeyId + 1)))
        {
            rbe->of.mInfo.mRxInfo.mAckedWithSecEnhAck = true;
            rbe->of.mInfo.mRxInfo.mAckKeyId           = keyId;
            rbe->of.mInfo.mRxInfo.mAckFrameCounter    = sMacFrameCounter;
            sMacFrameCounter++;
        }
    }
#endif
}

/**
 * aMacFormatFrame <-> aOtFrame bidirectional conversion
 *
 * @param[in] data             Pointer to frame or ring buffer entry
 * @param[in] aOtFrame         Pointer to OpenThread Frame
 */
static void K32WFrameConversion(tsPhyFrame *aPhyFrame, otRadioFrame *aOtFrame)
{
    aOtFrame->mLength = aPhyFrame->u8PayloadLength;
    aOtFrame->mInfo.mRxInfo.mTimestamp =
        otPlatTimeGet() - (uint64_t)(u32MMAC_GetTime() - u32V2MAC_GetRxTimestamp()) * US_PER_SYMBOL;
    aOtFrame->mInfo.mRxInfo.mLqi  = u8MMAC_GetRxLqi(NULL);
    aOtFrame->mInfo.mRxInfo.mRssi = i8Radio_GetLastPacketRSSI();
}

/**
 * Function used to init/reset an RX Ring Buffer
 *
 * @param[in] aRxRing         Pointer to an RX Ring Buffer
 */
static void K32WResetRxRingBuffer()
{
    sRxRing.head    = K32W0_RADIO_NUM_OF_RX_BUFS - 1;
    sRxRing.tail    = K32W0_RADIO_NUM_OF_RX_BUFS - 1;
    sRxRing.next    = 0;
    sRxRing.last    = 0;
    sRxRing.ovf_cnt = 0;
}

/**
 * Function used to push a received frame to the RX Ring buffer.
 */
static void K32WPushRxRingBuffer()
{
    sRxRing.head = sRxRing.next;
}

/**
 * Function used to pop a received frame to the RX Ring buffer.
 */
static void K32WPopRxRingBuffer()
{
    sRxRing.tail = sRxRing.last;
}

/**
 * Function used to get the first received frame from the RX Ring buffer.
 */
static rxRingBufferEntry *K32WGetRxRingBuffer()
{
    rxRingBufferEntry *rbe  = NULL;
    uint16_t           tail = sRxRing.tail;

    if (tail == sRxRing.head)
    {
        /* ring empty */
        return NULL;
    }

    tail         = (tail + 1) & (K32W0_RADIO_NUM_OF_RX_BUFS - 1);
    rbe          = &sRxRing.buffer[tail];
    sRxRing.last = tail;

    return rbe;
}

/**
 * Function used to enable the receiving of a frame.
 * Should be called when radio is idle.
 */
static void K32WEnableReceive()
{
    tsPhyFrame *pRxFrame = NULL;
    uint16_t    next     = (sRxRing.head + 1) & (K32W0_RADIO_NUM_OF_RX_BUFS - 1);

    if (next == sRxRing.tail)
    {
        /* ring full */
        sState = OT_RADIO_STATE_RX_DISABLED;
        sRxRing.ovf_cnt++;
        return;
    }

    pRxFrame     = &sRxRing.buffer[next].f;
    sRxRing.next = next;

    vMMAC_StartV2MacReceive(pRxFrame, sRxOpt);
}

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
void otPlatRadioSetMacKey(otInstance             *aInstance,
                          uint8_t                 aKeyIdMode,
                          uint8_t                 aKeyId,
                          const otMacKeyMaterial *aPrevKey,
                          const otMacKeyMaterial *aCurrKey,
                          const otMacKeyMaterial *aNextKey,
                          otRadioKeyType          aKeyType)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aKeyIdMode);

    assert(aPrevKey != NULL && aCurrKey != NULL && aNextKey != NULL);

    sKeyId = aKeyId;

    /* Assuming literal keys are used */
    V2MMAC_SetMacKey(aKeyId, aPrevKey->mKeyMaterial.mKey.m8, aCurrKey->mKeyMaterial.mKey.m8,
                     aNextKey->mKeyMaterial.mKey.m8);
}

void otPlatRadioSetMacFrameCounter(otInstance *aInstance, uint32_t aMacFrameCounter)
{
    OT_UNUSED_VARIABLE(aInstance);

    sMacFrameCounter = aMacFrameCounter;
    V2MMAC_SetMacFrameCounter(aMacFrameCounter);
}

void otPlatRadioSetMacFrameCounterIfLarger(otInstance *aInstance, uint32_t aMacFrameCounter)
{
    OT_UNUSED_VARIABLE(aInstance);

    if (aMacFrameCounter > sMacFrameCounter)
    {
        otPlatRadioSetMacFrameCounter(aInstance, aMacFrameCounter);
    }
}

#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
otError otPlatRadioEnableCsl(otInstance         *aInstance,
                             uint32_t            aCslPeriod,
                             otShortAddress      aShortAddr,
                             const otExtAddress *aExtAddr)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aShortAddr);
    OT_UNUSED_VARIABLE(aExtAddr);

    sCslPeriod = aCslPeriod;
    V2MMAC_EnableCsl(aCslPeriod);

    return OT_ERROR_NONE;
}

void otPlatRadioUpdateCslSampleTime(otInstance *aInstance, uint32_t aCslSampleTime)
{
    OT_UNUSED_VARIABLE(aInstance);

    /* aCslSampleTime is the next channel sample so in the future of the current Rx */
    aCslSampleTime = u32MMAC_GetTime() * US_PER_SYMBOL + (aCslSampleTime - (uint32_t)otPlatTimeGet());

    V2MMAC_SetCslSampleTime(aCslSampleTime);
}
#endif

uint8_t otPlatRadioGetCslUncertainty(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return CSL_UNCERT;
}

uint64_t otPlatRadioGetNow(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return otPlatTimeGet();
}

static void K32WEncFrame(void *t, const void *key)
{
    otRadioFrame f;

    f.mPsdu   = ((tsPhyFrame *)t)->uPayload.au8Byte;
    f.mLength = ((tsPhyFrame *)t)->u8PayloadLength;

    f.mInfo.mTxInfo.mAesKey              = key;
    f.mInfo.mTxInfo.mIsSecurityProcessed = false;

    otMacFrameProcessTransmitAesCcm(&f, &sRevExtAddr);
}

#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
otError otPlatRadioConfigureEnhAckProbing(otInstance         *aInstance,
                                          otLinkMetrics       aLinkMetrics,
                                          otShortAddress      aShortAddress,
                                          const otExtAddress *aExtAddress)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError error = OT_ERROR_NONE;

    error = otLinkMetricsConfigureEnhAckProbing(aShortAddress, aExtAddress, aLinkMetrics);
    otEXPECT(error == OT_ERROR_NONE);

exit:
    return error;
}

static uint8_t K32WGetVsIeLen(void *t)
{
    uint8_t      len;
    otRadioFrame f;
    otMacAddress dstAddr;

    f.mPsdu   = ((tsPhyFrame *)t)->uPayload.au8Byte;
    f.mLength = ((tsPhyFrame *)t)->u8PayloadLength;

    otMacFrameGetDstAddr(&f, &dstAddr);
    if ((len = otLinkMetricsEnhAckGetDataLen(&dstAddr)) > 0)
    {
        uint8_t tmp[OT_ACK_IE_MAX_SIZE];

        len = otMacFrameGenerateEnhAckProbingIe(tmp, NULL, len);
    }

    if (len)
    {
        ((tsPhyFrame *)t)->au8Padding[FMI_LM] = TRUE;
    }

    return len;
}

static void K32WGetVsIeGen(void *t, uint8_t *b)
{
    uint8_t      tmp[OT_ENH_PROBING_IE_DATA_MAX_SIZE];
    uint8_t      len;
    otRadioFrame f;
    otMacAddress dstAddr;

    /* the RSSI range for k32w0 is [-100, 10] dBm but the Thread spec requires it to be [-130, 0]
       Adjust RSSI to be [-110, 0] */

    uint8_t lqi  = u8MMAC_GetRxLqi(NULL);
    int8_t  rssi = i8Radio_GetLastPacketRSSI() - K32W_RADIO_RX_MAX_DBM;

    /* set the noise floor used by Link Metrics (-110 dBm) */
    otLinkMetricsInit(K32W_RADIO_RX_SENSITIVITY_DBM - K32W_RADIO_RX_MAX_DBM);

    f.mPsdu   = ((tsPhyFrame *)t)->uPayload.au8Byte;
    f.mLength = ((tsPhyFrame *)t)->u8PayloadLength;

    otMacFrameGetDstAddr(&f, &dstAddr);
    if ((len = otLinkMetricsEnhAckGenData(&dstAddr, lqi, rssi, tmp)) > 0)
    {
        otMacFrameGenerateEnhAckProbingIe(b, tmp, len);
    }
}
#endif

uint8_t otPlatRadioGetCslAccuracy(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return CONFIG_PLATFORM_CSL_ACCURACY;
}
#endif

#if OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE
otError otPlatRadioSetCoexEnabled(otInstance *aInstance, bool aEnabled)
{
    otError error    = OT_ERROR_NONE;
    void   *rfDeny   = NULL;
    void   *rfActive = NULL;
    void   *rfStatus = NULL;

    OT_UNUSED_VARIABLE(aInstance);

    if (isCoexInitialized == false)
    {
        BOARD_GetCoexIoCfg(&rfDeny, &rfActive, &rfStatus);

        /* This will enable coexistence by calling MWS_CoexistenceEnable() */
        if (MWS_CoexistenceInit(rfDeny, rfActive, rfStatus) != gMWS_Success_c)
        {
            error = OT_ERROR_INVALID_STATE;
        }
        else
        {
            vDynEnableCoex((void *)MWS_CoexistenceRegister, (void *)MWS_CoexistenceRequestAccess,
                           (void *)MWS_CoexistenceSetPriority, (void *)MWS_CoexistenceReleaseAccess,
                           (void *)MWS_CoexistenceChangeAccess);
            isCoexInitialized = true;
        }
    }

    if (aEnabled == true)
    {
        /* It's a superflous call if we just initialized... */
        MWS_CoexistenceEnable();
    }
    else
    {
        MWS_CoexistenceDisable();
    }

    return error;
}

bool otPlatRadioIsCoexEnabled(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return MWS_CoexistenceIsEnabled() == 0 ? false : true;
}

otError otPlatRadioGetCoexMetrics(otInstance *aInstance, otRadioCoexMetrics *aCoexMetrics)
{
    otError            error = OT_ERROR_NONE;
    struct sched_stats sched_stats;
#if defined(gMWS_EnableCoexistenceStats_d) && (gMWS_EnableCoexistenceStats_d == 1)
    mwsCoexStats_t coexStats;
#endif
    int ret = 0;

    OT_UNUSED_VARIABLE(aInstance);

    assert(aCoexMetrics != NULL);

    memset(aCoexMetrics, 0, sizeof(otRadioCoexMetrics));
#if defined(gMWS_EnableCoexistenceStats_d) && (gMWS_EnableCoexistenceStats_d == 1)
    /*
     * Retrieve the coex statistics. Since we don't have support for other types
     * of statistics, the 15.4 specific ones, as well as the ones specific for
     * scheduling will be printed here.
     */
    MWS_GetCoexStats(&coexStats);

    aCoexMetrics->mNumTxRequest          = coexStats.numTxRequests;
    aCoexMetrics->mNumTxGrantWait        = coexStats.numTxGrantWait;
    aCoexMetrics->mNumTxGrantWaitTimeout = coexStats.numTxGrantWaitTimeout;

    aCoexMetrics->mNumRxRequest        = coexStats.numRxRequests;
    aCoexMetrics->mNumRxGrantImmediate = coexStats.numRxGrantImmediate;

    aCoexMetrics->mNumRxGrantWait        = coexStats.numRxGrantWait;
    aCoexMetrics->mNumRxGrantWaitTimeout = coexStats.numRxGrantWaitTimeout;
#endif
    /* print remaining statistics */
    /* Get scheduler stats and dump them here */
    ret = sched_get_stats(&sched_stats);
    assert(ret == 0);

    PRINTF("\n\n802.15.4 scheduler metrics\n");
    PRINTF("--------------------------------------------------------\n");
    PRINTF("\t802.15.4 TX request DENY:  %d\n", sched_stats.tx_req_deny);
    PRINTF("\t802.15.4 TX request GRANT: %d\n", sched_stats.tx_req_grant);
    PRINTF("\t802.15.4 RX request DENY:  %d\n", sched_stats.rx_req_deny);
    PRINTF("\t802.15.4 RX request GRANT: %d\n", sched_stats.rx_req_grant);
    PRINTF("\t802.15.4 releases:         %d\n", sched_stats.rel);
    PRINTF("\t802.15.4 abort:            %d\n", sched_stats.abort);
    PRINTF("\t802.15.4 to BLE switch:    %d\n", sched_stats.l542ble);
    PRINTF("\tBLE to 802.15.4 switch:    %d\n", sched_stats.ble2l54);
    PRINTF("--------------------------------------------------------\n\n");

    return error;
}
#endif /* OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE */
