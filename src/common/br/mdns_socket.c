/*
 *  Copyright (c) 2024, The OpenThread Authors.
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

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */
#include "mdns_socket.h"
#include <openthread/platform/mdns_socket.h>
#include <openthread/platform/udp.h>

/* -------------------------------------------------------------------------- */
/*                                 Definitions                                */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */
static otInstance        *sInstance;
static otUdpSocket        sMdnsSocket;
static uint32_t           sInfraIfIndex;
static const uint16_t     sMulticastPort = 5353;
static bool               sIsEnabled;
static const otIp6Address sMulticastGroup = {
    .mFields.m8 = {0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFB}};
static const otIp6Address kAnyAddress = {
    .mFields.m8 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

void    MdnsSocketReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);
void    SendMulticast(otMessage *aMessage, uint32_t aInfraIfIndex);
void    SendUnicast(otMessage *aMessage, const otPlatMdnsAddressInfo *aAddress);
otError SetListeningEnabled(otInstance *aInstance, bool aEnable, uint32_t aInfraIfIndex);

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void MdnsSocketInit(otInstance *aInstance, uint32_t aInfraIfIndex)
{
    sInstance     = aInstance;
    sInfraIfIndex = aInfraIfIndex;
    memset(&sMdnsSocket, 0, sizeof(sMdnsSocket));
    sMdnsSocket.mSockName.mPort = sMulticastPort;
    sMdnsSocket.mHandler        = MdnsSocketReceive;
}

otError otPlatMdnsSetListeningEnabled(otInstance *aInstance, bool aEnable, uint32_t aInfraIfIndex)
{
    return SetListeningEnabled(aInstance, aEnable, aInfraIfIndex);
}

void otPlatMdnsSendMulticast(otInstance *aInstance, otMessage *aMessage, uint32_t aInfraIfIndex)
{
    OT_UNUSED_VARIABLE(aInstance);
    return SendMulticast(aMessage, aInfraIfIndex);
}

void otPlatMdnsSendUnicast(otInstance *aInstance, otMessage *aMessage, const otPlatMdnsAddressInfo *aAddress)
{
    OT_UNUSED_VARIABLE(aInstance);
    return SendUnicast(aMessage, aAddress);
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

void MdnsSocketReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    otPlatMdnsAddressInfo addrInfo;
    memset(&addrInfo, 0, sizeof(addrInfo));

    addrInfo.mAddress      = aMessageInfo->mPeerAddr;
    addrInfo.mPort         = aMessageInfo->mPeerPort;
    addrInfo.mInfraIfIndex = sInfraIfIndex;

    otPlatMdnsHandleReceive(sInstance, aMessage, /* aInUnicast */ false, &addrInfo);
}

otError SetListeningEnabled(otInstance *aInstance, bool aEnable, uint32_t aInfraIfIndex)
{
    otError error = OT_ERROR_NONE;
    if (aEnable)
    {
        VerifyOrExit(otPlatUdpSocket(&sMdnsSocket) == OT_ERROR_NONE, error = OT_ERROR_FAILED);
        VerifyOrExit(otPlatUdpBindToNetif(&sMdnsSocket, OT_NETIF_BACKBONE) == OT_ERROR_NONE, error = OT_ERROR_FAILED);
        VerifyOrExit(otPlatUdpBind(&sMdnsSocket) == OT_ERROR_NONE, error = OT_ERROR_FAILED);
        VerifyOrExit(otPlatUdpJoinMulticastGroup(&sMdnsSocket, OT_NETIF_BACKBONE, &sMulticastGroup) == OT_ERROR_NONE,
                     error = OT_ERROR_FAILED);
        sIsEnabled = true;
    }
    else
    {
        sIsEnabled = false;
        VerifyOrExit(otPlatUdpLeaveMulticastGroup(&sMdnsSocket, OT_NETIF_BACKBONE, &sMulticastGroup) == OT_ERROR_NONE,
                     error = OT_ERROR_FAILED);
        VerifyOrExit(otPlatUdpClose(&sMdnsSocket) == OT_ERROR_NONE, error = OT_ERROR_FAILED);
    }

exit:
    return error;
}

void SendMulticast(otMessage *aMessage, uint32_t aInfraIfIndex)
{
    if (sIsEnabled && aInfraIfIndex == sInfraIfIndex)
    {
        otMessageInfo msgInfo;
        memset(&msgInfo, 0, sizeof(msgInfo));
        msgInfo.mPeerAddr = sMulticastGroup;
        msgInfo.mPeerPort = sMulticastPort;
        msgInfo.mSockAddr = kAnyAddress;
        msgInfo.mSockPort = sMulticastPort;
        // Enable multicast loop to cover the case where a discovery proxy
        // query must be also be checked against BR's own services.
        msgInfo.mMulticastLoop = true;

        otPlatUdpSend(&sMdnsSocket, aMessage, &msgInfo);
    }
    else
    {
        if (aMessage != NULL)
        {
            otMessageFree(aMessage);
        }
    }
}

void SendUnicast(otMessage *aMessage, const otPlatMdnsAddressInfo *aAddress)
{
    if (sIsEnabled && aAddress->mInfraIfIndex == sInfraIfIndex)
    {
        otMessageInfo msgInfo;
        memset(&msgInfo, 0, sizeof(msgInfo));
        msgInfo.mPeerAddr = aAddress->mAddress;
        msgInfo.mPeerPort = aAddress->mPort;
        msgInfo.mSockAddr = kAnyAddress;
        msgInfo.mSockPort = sMulticastPort;

        otPlatUdpSend(&sMdnsSocket, aMessage, &msgInfo);
    }
    else
    {
        if (aMessage != NULL)
        {
            otMessageFree(aMessage);
        }
    }
}
