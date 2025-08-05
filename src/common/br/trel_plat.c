/*
 *  Copyright (c) 2024-2025, The OpenThread Authors.
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
 *   This file implements the OpenThread platform abstraction for Thread Radio Encapsulation Link (TREL) using DNS-SD
 * and UDP/IPv6.
 *
 */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include "trel_plat.h"
#include "br_rtos_manager.h"
#include "udp_plat.h"
#include "utils.h"
#include <string.h>
#include <openthread/ip6.h>
#include <openthread/link.h>

#include <openthread/platform/memory.h>
#include <openthread/platform/trel.h>
#include <openthread/platform/udp.h>
#include "common/code_utils.hpp"

#include "config/mle.h"
#include "lwip/api.h"
#include "lwip/sockets.h"
#include "lwip/tcpip.h"
#include "lwip/udp.h"

// #include "fsl_component_generic_list.h"
#include "fsl_os_abstraction.h"

/* -------------------------------------------------------------------------- */
/*                                 Definitions                                */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

static otUdpSocket        sTrelSocket;
static otInstance        *sInstance;
static struct netif      *sBackboneNetifPtr;
static bool               sTrelEnabled;
static otPlatTrelCounters sCounters;

static const otIp6Address kAnyAddress = {
    .mFields.m8 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */
static void TrelSocketReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void TrelPlatInit(otInstance *aInstance, struct netif *backboneNetif)
{
    sInstance         = aInstance;
    sBackboneNetifPtr = backboneNetif;

    (void)otPlatUdpBindToNetif(&sTrelSocket, OT_NETIF_BACKBONE);
}

void otPlatTrelEnable(otInstance *aInstance, uint16_t *aUdpPort)
{
    OT_UNUSED_VARIABLE(aInstance);

    sTrelSocket.mHandler = TrelSocketReceive;
    struct udp_pcb *pcb  = NULL;

    VerifyOrExit(!sTrelEnabled);

    VerifyOrExit(otPlatUdpSocket(&sTrelSocket) == OT_ERROR_NONE);
    VerifyOrExit(otPlatUdpBind(&sTrelSocket) == OT_ERROR_NONE);

    pcb = (struct udp_pcb *)sTrelSocket.mHandle;

    *aUdpPort                   = pcb->local_port;
    sTrelSocket.mSockName.mPort = pcb->local_port;
    sTrelEnabled                = true;

    otPlatTrelResetCounters(aInstance);
exit:
    return;
}

void otPlatTrelDisable(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    VerifyOrExit(sTrelEnabled);

    // close UDP socket
    otPlatUdpClose(&sTrelSocket);
    sTrelEnabled = false;

exit:
    return;
}

void otPlatTrelSend(otInstance       *aInstance,
                    const uint8_t    *aUdpPayload,
                    uint16_t          aUdpPayloadLen,
                    const otSockAddr *aDestSockAddr)
{
    OT_UNUSED_VARIABLE(aInstance);

    otMessage    *message;
    otMessageInfo messageInfo;
    memset(&messageInfo, 0, sizeof(otMessageInfo));
    otMessageSettings msgSettings = {.mLinkSecurityEnabled = false, .mPriority = OT_MESSAGE_PRIORITY_NORMAL};

    VerifyOrExit(sTrelEnabled);

    message = otUdpNewMessage(sInstance, &msgSettings);
    VerifyOrExit(message != NULL);
    if (otMessageAppend(message, aUdpPayload, aUdpPayloadLen) != OT_ERROR_NONE)
    {
        otMessageFree(message);
        ++sCounters.mTxFailure;
        return;
    }

    messageInfo.mPeerAddr        = aDestSockAddr->mAddress;
    messageInfo.mPeerPort        = aDestSockAddr->mPort;
    messageInfo.mSockPort        = sTrelSocket.mSockName.mPort;
    messageInfo.mSockAddr        = kAnyAddress;
    messageInfo.mIsHostInterface = true;

    if (otPlatUdpSend(&sTrelSocket, message, &messageInfo) == OT_ERROR_NONE)
    {
        ++sCounters.mTxPackets;
        sCounters.mTxBytes += aUdpPayloadLen;
    }
    else
    {
        ++sCounters.mTxFailure;
    }

exit:
    return;
}

const otPlatTrelCounters *otPlatTrelGetCounters(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return &sCounters;
}

void otPlatTrelResetCounters(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    memset(&sCounters, 0, sizeof(sCounters));
}

// This function is needed as a stub even if peer discovery is handled in the core
// stack until the stack version is at least 08efcda
void otPlatTrelNotifyPeerSocketAddressDifference(otInstance       *aInstance,
                                                 const otSockAddr *aPeerSockAddr,
                                                 const otSockAddr *aRxSockAddr)
{
}
/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */
static void TrelSocketReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    OT_UNUSED_VARIABLE(aContext);

    otSockAddr senderAddr;
    uint16_t   messageLen     = otMessageGetLength(aMessage);
    uint8_t   *rxPacketBuffer = (uint8_t *)otPlatCAlloc(1, messageLen);
    otMessageRead(aMessage, 0, rxPacketBuffer, messageLen);
    ++sCounters.mRxPackets;
    sCounters.mRxBytes += messageLen;
    senderAddr.mAddress = aMessageInfo->mPeerAddr;
    senderAddr.mPort    = aMessageInfo->mPeerPort;
    otPlatTrelHandleReceived(sInstance, rxPacketBuffer, messageLen, &senderAddr);
    otPlatFree(rxPacketBuffer);
}
