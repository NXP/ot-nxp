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
#include "br_rtos_manager.h"
#include "ot_lwip.h"

#include <openthread/nat64.h>
#include <openthread/platform/mdns_socket.h>
#include <openthread/platform/memory.h>
#include <openthread/platform/messagepool.h>
#include <openthread/platform/udp.h>
#include "lwip/dns.h"
#include "lwip/igmp.h"
#include "lwip/ip_addr.h"
#include "lwip/mld6.h"
#include "lwip/udp.h"

#include "openthread-core-config.h"

/* -------------------------------------------------------------------------- */
/*                                 Definitions                                */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */
static otInstance     *sInstance;
static struct udp_pcb *sMdnsPcb;
static uint32_t        sInfraIfIndex;
static const uint16_t  sMulticastPort = 5353;

static bool sMdnsIsEnabled;

static const otIp6Address sMulticastGroupv4MappedTov6 = {
    .mFields.m8 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xE0, 0x00, 0x00, 0xFB}};
static const otIp6Address sMulticastGroupv6 = {
    .mFields.m8 = {0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFB}};

typedef struct MdnsAddressInfo
{
    otPlatMdnsAddressInfo mAddrInfov6;
    otPlatMdnsAddressInfo mAddrInfov4;
    bool                  mTransmitIp6;
    bool                  mTransmitIp4;
    bool                  mMcastLoop;
} MdnsAddressInfo;

struct udpSendContext
{
    struct udp_pcb *pcb;
    otMessage      *message;
    MdnsAddressInfo addressInfo;
};

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

static void    MdnsSocketReceive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
static void    SendMulticast(otMessage *aMessage, uint32_t aInfraIfIndex);
static void    SendUnicast(otMessage *aMessage, const otPlatMdnsAddressInfo *aAddress);
static otError SetListeningEnabled(otInstance *aInstance, bool aEnable, uint32_t aInfraIfIndex);
static otError SocketInit(uint32_t aInfraIfIndex);
static otError SocketDeInit(uint32_t aInfraIfIndex);
static void    LwipTaskCb(void *aContext);
static void    MdnsProcessOtReceive(brMsgContext *aMsgContextPtr);

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void MdnsSocketInit(otInstance *aInstance, uint32_t aInfraIfIndex)
{
    sInstance     = aInstance;
    sInfraIfIndex = aInfraIfIndex;
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

static void MdnsSocketReceive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    (void)pcb;
    brMsgContext *contextMsgPtr = NULL;

    VerifyOrExit(sMdnsIsEnabled);

    contextMsgPtr = (brMsgContext *)otPlatCAlloc(1, sizeof(brMsgContext));
    VerifyOrExit(contextMsgPtr != NULL);

    contextMsgPtr->socket = (otUdpSocket *)arg;
    contextMsgPtr->pbuf   = p;

    contextMsgPtr->addrInfo.mAddress      = otPlatLwipConvertToOtAddress(addr);
    contextMsgPtr->addrInfo.mPort         = port;
    contextMsgPtr->addrInfo.mInfraIfIndex = sInfraIfIndex;
    contextMsgPtr->brMsgCallback          = MdnsProcessOtReceive;

    BrPostOtMessage(contextMsgPtr);

exit:
    if (contextMsgPtr == NULL)
    {
        pbuf_free(p);
    }
}

static void MdnsProcessOtReceive(brMsgContext *aContextMsgPtr)
{
    otMessage   *message = NULL;
    uint16_t     requiredBuffNo;
    otBufferInfo bufferInfo;

    VerifyOrExit(sMdnsIsEnabled);

    /* In large networks with high traffic, we have observed that mDNS module might jump to assert when trying to
       allocate OT message buffers for a new query/response that has to be sent. Here, we calculate the approximate
       number of OT message buffers that will be required to hold the incoming mDNS packet. If the number of free OT
       message buffers will drop below the imposed limit after the conversion has been perfomed, the incoming packet
       will be silently dropped. A possible scenario would be when multipackets (TC bit set) are received from multiple
       hosts, as mDNS module stores the incoming messages for a period of time. This mechanism tries to make sure that
       there are enough free buffers for mDNS module to perform it's execution.
    */
    requiredBuffNo =
        (aContextMsgPtr->pbuf->tot_len / (OPENTHREAD_CONFIG_MESSAGE_BUFFER_SIZE - sizeof(otMessageBuffer))) + 1;

    otMessageGetBufferInfo(sInstance, &bufferInfo);
    if ((bufferInfo.mFreeBuffers - requiredBuffNo) >= ((40 * OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS) / 100))
    {
        message = otPlatLwipConvertToOtMsg(aContextMsgPtr->pbuf);
        VerifyOrExit(message != NULL);
    }
    else
    {
        ExitNow();
    }

    // message is owned by OT, no need to free it explicitly
    otPlatMdnsHandleReceive(sInstance, message, /* aInUnicast */ false, &aContextMsgPtr->addrInfo);
exit:
    // Free the pbuf on lwip context to prevent any possible corruption
    (void)pbuf_free_callback(aContextMsgPtr->pbuf);
}

static otError SetListeningEnabled(otInstance *aInstance, bool aEnable, uint32_t aInfraIfIndex)
{
    otError error = OT_ERROR_NONE;

    if (aEnable)
    {
        VerifyOrExit(SocketInit(aInfraIfIndex) == OT_ERROR_NONE, error = OT_ERROR_FAILED);
    }
    else
    {
        error = SocketDeInit(aInfraIfIndex);
    }

exit:
    return error;
}

static otError SocketInit(uint32_t aInfraIfIndex)
{
    otError error     = OT_ERROR_FAILED;
    err_t   lwipError = ERR_OK;
    VerifyOrExit(aInfraIfIndex == sInfraIfIndex, error = OT_ERROR_INVALID_ARGS);

    CALL_LWIP_API_FROM_OT_CONTEXT(sMdnsPcb = udp_new_ip_type(IPADDR_TYPE_ANY));
    if (sMdnsPcb != NULL)
    {
        CALL_LWIP_API_FROM_OT_CONTEXT(lwipError = udp_bind(sMdnsPcb, IP_ANY_TYPE, sMulticastPort));
        VerifyOrExit(lwipError == ERR_OK, error = OT_ERROR_FAILED);

        CALL_LWIP_API_FROM_OT_CONTEXT(udp_bind_netif(sMdnsPcb, netif_get_by_index(aInfraIfIndex)));
        ip_addr_t groupAddr = {0};
        groupAddr           = otPlatLwipConvertToLwipAddress(&sMulticastGroupv6);

        CALL_LWIP_API_FROM_OT_CONTEXT(
            lwipError = mld6_joingroup_netif(netif_get_by_index(aInfraIfIndex), ip_2_ip6(&groupAddr)));
        VerifyOrExit(lwipError == ERR_OK, error = OT_ERROR_FAILED);

        groupAddr = otPlatLwipConvertToLwipAddress(&sMulticastGroupv4MappedTov6);
        CALL_LWIP_API_FROM_OT_CONTEXT(
            lwipError = igmp_joingroup_netif(netif_get_by_index(aInfraIfIndex), ip_2_ip4(&groupAddr)));
        VerifyOrExit(lwipError == ERR_OK, error = OT_ERROR_FAILED);

        CALL_LWIP_API_FROM_OT_CONTEXT(udp_recv(sMdnsPcb, MdnsSocketReceive, NULL));
        sMdnsIsEnabled = true;
        error          = OT_ERROR_NONE;
    }

exit:
    if (error != OT_ERROR_NONE && sMdnsPcb != NULL)
    {
        CALL_LWIP_API_FROM_OT_CONTEXT(udp_remove(sMdnsPcb));
    }
    return error;
}

static otError SocketDeInit(uint32_t aInfraIfIndex)
{
    otError error = OT_ERROR_NONE;
    VerifyOrExit(aInfraIfIndex == sInfraIfIndex, error = OT_ERROR_INVALID_ARGS);
    VerifyOrExit(sMdnsIsEnabled, error = OT_ERROR_INVALID_STATE);
    ip_addr_t groupAddr = {0};
    groupAddr           = otPlatLwipConvertToLwipAddress(&sMulticastGroupv6);

    CALL_LWIP_API_FROM_OT_CONTEXT({
        (void)mld6_leavegroup_netif(netif_get_by_index(aInfraIfIndex), ip_2_ip6(&groupAddr));
        groupAddr = otPlatLwipConvertToLwipAddress(&sMulticastGroupv4MappedTov6);
        (void)igmp_leavegroup_netif(netif_get_by_index(aInfraIfIndex), ip_2_ip4(&groupAddr));
        udp_remove(sMdnsPcb);
    });

    sMdnsIsEnabled = false;
exit:
    return error;
}

static void SendMulticast(otMessage *aMessage, uint32_t aInfraIfIndex)
{
    otError error       = OT_ERROR_NONE;
    err_t   postCbError = ERR_OK;

    if (sMdnsIsEnabled && aInfraIfIndex == sInfraIfIndex)
    {
        struct udpSendContext *udpSendContexPtr =
            (struct udpSendContext *)otPlatCAlloc(1, sizeof(struct udpSendContext));
        VerifyOrExit(NULL != udpSendContexPtr, error = OT_ERROR_FAILED);

        MdnsAddressInfo addressInfo = {.mAddrInfov6.mAddress      = sMulticastGroupv6,
                                       .mAddrInfov6.mPort         = sMulticastPort,
                                       .mAddrInfov6.mInfraIfIndex = sInfraIfIndex,
                                       .mAddrInfov4.mAddress      = sMulticastGroupv4MappedTov6,
                                       .mAddrInfov4.mPort         = sMulticastPort,
                                       .mAddrInfov4.mInfraIfIndex = sInfraIfIndex,
                                       .mTransmitIp6              = true,
                                       .mTransmitIp4              = true,
                                       .mMcastLoop                = true};

        udpSendContexPtr->addressInfo = addressInfo;
        udpSendContexPtr->message     = aMessage;

        POST_LWIP_CALLBACK_FROM_OT_CONTEXT(postCbError = tcpip_callback(LwipTaskCb, (void *)udpSendContexPtr));

        if (postCbError != ERR_OK)
        {
            otPlatFree(udpSendContexPtr);
            otMessageFree(aMessage);
            aMessage = NULL;
            error    = OT_ERROR_FAILED;
        }
    }
    else
    {
        if (aMessage != NULL)
        {
            otMessageFree(aMessage);
            aMessage = NULL;
        }
    }
exit:
    if (error == OT_ERROR_FAILED)
    {
        if (aMessage != NULL)
        {
            otMessageFree(aMessage);
        }
    }
    return;
}

static void SendUnicast(otMessage *aMessage, const otPlatMdnsAddressInfo *aAddress)
{
    otError error       = OT_ERROR_NONE;
    err_t   postCbError = ERR_OK;

    if (sMdnsIsEnabled && aAddress->mInfraIfIndex == sInfraIfIndex)
    {
        struct udpSendContext *udpSendContexPtr =
            (struct udpSendContext *)otPlatCAlloc(1, sizeof(struct udpSendContext));
        VerifyOrExit(NULL != udpSendContexPtr, error = OT_ERROR_FAILED);

        otIp4Address    tmp         = {0};
        MdnsAddressInfo addressInfo = {0};
        if (otIp4FromIp4MappedIp6Address(&aAddress->mAddress, &tmp) == OT_ERROR_NONE)
        {
            addressInfo.mAddrInfov4.mAddress = aAddress->mAddress;
            addressInfo.mAddrInfov4.mPort    = aAddress->mPort;
            addressInfo.mTransmitIp4         = true;
        }
        else
        {
            addressInfo.mAddrInfov6.mAddress = aAddress->mAddress;
            addressInfo.mAddrInfov6.mPort    = aAddress->mPort;
            addressInfo.mTransmitIp6         = true;
        }

        udpSendContexPtr->addressInfo = addressInfo;
        udpSendContexPtr->message     = aMessage;

        POST_LWIP_CALLBACK_FROM_OT_CONTEXT(postCbError = tcpip_callback(LwipTaskCb, (void *)udpSendContexPtr));
        if (postCbError != ERR_OK)
        {
            otPlatFree(udpSendContexPtr);
            otMessageFree(aMessage);
            aMessage = NULL;
            error    = OT_ERROR_FAILED;
        }
    }
    else
    {
        if (aMessage != NULL)
        {
            otMessageFree(aMessage);
            aMessage = NULL;
        }
    }
exit:
    if (error == OT_ERROR_FAILED)
    {
        if (aMessage != NULL)
        {
            otMessageFree(aMessage);
        }
    }
    return;
}

static void LwipTaskCb(void *aContext)
{
    struct udpSendContext *udpSendContexPtr = (struct udpSendContext *)aContext;
    struct pbuf           *buffer           = NULL;

    if (udpSendContexPtr->addressInfo.mMcastLoop)
    {
        // Enable multicast loop to cover the case where a discovery proxy
        // query must be also be checked against BR's own services.
        sMdnsPcb->flags |= (UDP_FLAGS_MULTICAST_LOOP);
    }

    if (udpSendContexPtr->addressInfo.mTransmitIp6)
    {
        buffer = otPlatLwipConvertToLwipMsg(udpSendContexPtr->message, true);
        if (buffer != NULL)
        {
            ip_addr_t peerAddress = otPlatLwipConvertToLwipAddress(
                (const otIp6Address *)&udpSendContexPtr->addressInfo.mAddrInfov6.mAddress);
            uint16_t port = udpSendContexPtr->addressInfo.mAddrInfov6.mPort;
            (void)udp_sendto(sMdnsPcb, buffer, &peerAddress, port);
            pbuf_free(buffer);
            buffer = NULL;

            // Disable multicast loop; there is no need to send over IPv4.
            sMdnsPcb->flags &= ~(UDP_FLAGS_MULTICAST_LOOP);
        }
    }

    if (udpSendContexPtr->addressInfo.mTransmitIp4)
    {
        buffer = otPlatLwipConvertToLwipMsg(udpSendContexPtr->message, true);
        if (buffer != NULL)
        {
            ip_addr_t peerAddress = otPlatLwipConvertToLwipAddress(
                (const otIp6Address *)&udpSendContexPtr->addressInfo.mAddrInfov4.mAddress);
            uint16_t port = udpSendContexPtr->addressInfo.mAddrInfov4.mPort;
            (void)udp_sendto(sMdnsPcb, buffer, &peerAddress, port);
            pbuf_free(buffer);
            buffer = NULL;
        }
    }

    gLockTaskCb();
    otMessageFree(udpSendContexPtr->message);
    gUnlockTaskCb();

    otPlatFree(aContext);
    aContext = NULL;

    return;
}
