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

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */
#include "mdns_socket.h"
#include <openthread/nat64.h>
#include <openthread/platform/mdns_socket.h>
#include <openthread/platform/memory.h>
#include <openthread/platform/udp.h>
#include "lwip/ip_addr.h"
#include "lwip/udp.h"

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
static const otIp6Address sMulticastGroupv4MappedTov6 = {
    .mFields.m8 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xE0, 0x00, 0x00, 0xFB}};
static const otIp6Address sMulticastGroupv6 = {
    .mFields.m8 = {0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFB}};
static const otIp6Address kAnyAddressv4MappedTov6 = {
    .mFields.m8 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00}};
static const otIp6Address kAnyAddressv6 = {
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
    otPlatMdnsAddressInfo addrInfo = {0};

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
        // Lwip states that for listening to dual-stack packets, IPADDR_TYPE_ANY should be used for local_ip and
        // remote_ip members and binding should also be done to IP_ANY_TYPE
        struct udp_pcb *pcb = (struct udp_pcb *)sMdnsSocket.mHandle;
        pcb->local_ip.type  = IPADDR_TYPE_ANY;
        pcb->remote_ip.type = IPADDR_TYPE_ANY;

        VerifyOrExit(otPlatUdpBindToNetif(&sMdnsSocket, OT_NETIF_BACKBONE) == OT_ERROR_NONE, error = OT_ERROR_FAILED);
        VerifyOrExit(otPlatUdpBind(&sMdnsSocket) == OT_ERROR_NONE, error = OT_ERROR_FAILED);

        VerifyOrExit(otPlatUdpJoinMulticastGroup(&sMdnsSocket, OT_NETIF_BACKBONE, &sMulticastGroupv4MappedTov6) ==
                         OT_ERROR_NONE,
                     error = OT_ERROR_FAILED);

        VerifyOrExit(otPlatUdpJoinMulticastGroup(&sMdnsSocket, OT_NETIF_BACKBONE, &sMulticastGroupv6) == OT_ERROR_NONE,
                     error = OT_ERROR_FAILED);
        sIsEnabled = true;
    }
    else
    {
        sIsEnabled = false;

        VerifyOrExit(otPlatUdpLeaveMulticastGroup(&sMdnsSocket, OT_NETIF_BACKBONE, &sMulticastGroupv4MappedTov6) ==
                         OT_ERROR_NONE,
                     error = OT_ERROR_FAILED);

        VerifyOrExit(otPlatUdpLeaveMulticastGroup(&sMdnsSocket, OT_NETIF_BACKBONE, &sMulticastGroupv6) == OT_ERROR_NONE,
                     error = OT_ERROR_FAILED);
        VerifyOrExit(otPlatUdpClose(&sMdnsSocket) == OT_ERROR_NONE, error = OT_ERROR_FAILED);
    }

exit:
    return error;
}

void SendMulticast(otMessage *aMessage, uint32_t aInfraIfIndex)
{
    otError    error       = OT_ERROR_NONE;
    uint8_t   *msgCopyBuf  = NULL;
    otMessage *copyMessage = NULL;

    if (sIsEnabled && aInfraIfIndex == sInfraIfIndex)
    {
        otMessageInfo msgInfov6 = {0};
        otMessageInfo msgInfov4 = {0};
        uint16_t      msgLen    = otMessageGetLength(aMessage);

        msgCopyBuf = (uint8_t *)otPlatCAlloc(1, msgLen);
        VerifyOrExit(msgCopyBuf != NULL, error = OT_ERROR_FAILED);

        if (otMessageRead(aMessage, 0, msgCopyBuf, msgLen) != msgLen)
        {
            ExitNow(error = OT_ERROR_FAILED);
        }
        copyMessage = otIp6NewMessage(sInstance, NULL);
        VerifyOrExit(copyMessage != NULL, error = OT_ERROR_FAILED);

        VerifyOrExit(otMessageAppend(copyMessage, msgCopyBuf, msgLen) == OT_ERROR_NONE, error = OT_ERROR_FAILED);
        otPlatFree(msgCopyBuf);

        msgInfov6.mPeerAddr = sMulticastGroupv6;
        msgInfov6.mPeerPort = sMulticastPort;
        msgInfov6.mSockAddr = kAnyAddressv6;
        msgInfov6.mSockPort = sMulticastPort;
        // Enable multicast loop to cover the case where a discovery proxy
        // query must be also be checked against BR's own services.
        msgInfov6.mMulticastLoop = true;

        msgInfov4.mPeerAddr = sMulticastGroupv4MappedTov6;
        msgInfov4.mPeerPort = sMulticastPort;
        msgInfov4.mSockAddr = kAnyAddressv4MappedTov6;
        msgInfov4.mSockPort = sMulticastPort;

        otPlatUdpSend(&sMdnsSocket, aMessage, &msgInfov6);
        otPlatUdpSend(&sMdnsSocket, copyMessage, &msgInfov4);
    }
exit:
    if (error == OT_ERROR_FAILED)
    {
        if (aMessage != NULL)
        {
            otMessageFree(aMessage);
        }
        if (copyMessage != NULL)
        {
            otMessageFree(copyMessage);
        }
        if (msgCopyBuf != NULL)
        {
            otPlatFree(msgCopyBuf);
        }
    }
}

void SendUnicast(otMessage *aMessage, const otPlatMdnsAddressInfo *aAddress)
{
    if (sIsEnabled && aAddress->mInfraIfIndex == sInfraIfIndex)
    {
        otIp4Address  ip4Address = {0};
        otMessageInfo msgInfo    = {0};
        if (otIp4FromIp4MappedIp6Address(&aAddress->mAddress, &ip4Address))
        {
            msgInfo.mSockAddr = kAnyAddressv4MappedTov6;
        }
        else
        {
            msgInfo.mSockAddr = kAnyAddressv6;
        }
        msgInfo.mPeerAddr = aAddress->mAddress;
        msgInfo.mPeerPort = aAddress->mPort;
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
