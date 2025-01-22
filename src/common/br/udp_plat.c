/*
 *  Copyright (c) 2023-2025, The OpenThread Authors.
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
 *   This file implements the OpenThread platform abstraction for UDP.
 *
 */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include "udp_plat.h"
#include "ot_lwip.h"
#include <common/code_utils.hpp>
#include <openthread/ip6.h>
#include <openthread/tasklet.h>
#include <openthread/udp.h>
#include <openthread/platform/memory.h>
#include <openthread/platform/udp.h>
#include "lwip/api.h"
#include "lwip/icmp6.h"
#include "lwip/inet.h"
#include "lwip/mld6.h"
#include "lwip/prot/dns.h"
#include "lwip/prot/iana.h"
#include "lwip/raw.h"
#include "lwip/sockets.h"
#include "lwip/tcpip.h"
#include "lwip/udp.h"

#if LWIP_IPV4
#include "lwip/igmp.h"
#endif

#include "br_rtos_manager.h"
#include "fsl_component_generic_list.h"
#include "fsl_os_abstraction.h"

/* -------------------------------------------------------------------------- */
/*                                 Definitions                                */
/* -------------------------------------------------------------------------- */
struct udpSendContext
{
    struct udp_pcb *pcb;
    otMessage      *message;
    otMessageInfo  *messageInfo;
};

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */
static uint8_t sBackboneNetifIdx;
static uint8_t sOtNetifIdx;

static otInstance   *sInstance = NULL;
static struct netif *sBackboneNetifPtr;
static struct netif *sOtNetifPtr;

static bool sUdpPlatInit;
/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

static void UdpPlatLwipSockCb(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
static struct netif *UdpPlatGetIfPtr(otNetifIdentifier aNetifIdentifier);
static void          UdpPlatLwipTaskCb(void *context);
static void          UdpPlatProcessOtReceive(brMsgContext *aContextMsgPtr);
/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void UdpPlatInit(otInstance *aInstance, struct netif *backboneNetif, struct netif *otNetif)
{
    sInstance         = aInstance;
    sBackboneNetifPtr = backboneNetif;
    sOtNetifPtr       = otNetif;
    sBackboneNetifIdx = netif_get_index(backboneNetif);
    sOtNetifIdx       = netif_get_index(otNetif);

    sUdpPlatInit = true;
}

otError otPlatUdpSocket(otUdpSocket *aUdpSocket)
{
    otError         error = OT_ERROR_NONE;
    struct udp_pcb *pcb   = NULL;

    VerifyOrExit(aUdpSocket != NULL, error = OT_ERROR_INVALID_ARGS);
    CALL_LWIP_API_FROM_OT_CONTEXT({
        pcb = udp_new();
        if (pcb != NULL)
        {
            udp_recv(pcb, UdpPlatLwipSockCb, aUdpSocket);
        }
    });

    VerifyOrExit(pcb != NULL, error = OT_ERROR_FAILED);

    aUdpSocket->mHandle = pcb;

exit:
    return error;
}

otError otPlatUdpClose(otUdpSocket *aUdpSocket)
{
    otError error = OT_ERROR_NONE;

    struct udp_pcb *pcb = (struct udp_pcb *)aUdpSocket->mHandle;
    VerifyOrExit(pcb != NULL, error = OT_ERROR_INVALID_ARGS);

    CALL_LWIP_API_FROM_OT_CONTEXT(udp_remove(pcb));

    aUdpSocket->mHandle = NULL;

exit:
    return error;
}

otError otPlatUdpBind(otUdpSocket *aUdpSocket)
{
    otError error     = OT_ERROR_NONE;
    err_t   bindError = ERR_OK;

    struct udp_pcb *pcb = (struct udp_pcb *)aUdpSocket->mHandle;
    VerifyOrExit(pcb != NULL, error = OT_ERROR_INVALID_ARGS);

    uint16_t  port = aUdpSocket->mSockName.mPort;
    ip_addr_t addr = otPlatLwipConvertToLwipAddress(&aUdpSocket->mSockName.mAddress);

    CALL_LWIP_API_FROM_OT_CONTEXT(bindError = udp_bind(pcb, &addr, port));
    VerifyOrExit(bindError == ERR_OK, error = OT_ERROR_FAILED);

exit:
    return error;
}

otError otPlatUdpBindToNetif(otUdpSocket *aUdpSocket, otNetifIdentifier aNetifIdentifier)
{
    otError error = OT_ERROR_NONE;

    VerifyOrExit(sUdpPlatInit, error = OT_ERROR_INVALID_STATE);

    struct udp_pcb *pcb          = (struct udp_pcb *)aUdpSocket->mHandle;
    struct netif   *currentNetif = UdpPlatGetIfPtr(aNetifIdentifier);
    VerifyOrExit(pcb != NULL, error = OT_ERROR_INVALID_ARGS);

    CALL_LWIP_API_FROM_OT_CONTEXT(udp_bind_netif(pcb, currentNetif));

exit:
    return error;
}

otError otPlatUdpConnect(otUdpSocket *aUdpSocket)
{
    otError error        = OT_ERROR_NONE;
    err_t   connectError = ERR_OK;

    struct udp_pcb *pcb = (struct udp_pcb *)aUdpSocket->mHandle;
    VerifyOrExit(pcb != NULL, error = OT_ERROR_INVALID_ARGS);

    uint16_t  port = aUdpSocket->mPeerName.mPort;
    ip_addr_t addr = {0};

    // LWIP doesn't threat the case were port or address are 0. In this case, the connect should act more like a
    // disconnect and clear the connect information stored in PCB. If we let LWIP connect with 0, it will drop
    // valid UDP packets because the source port/address doesn't match 0.
    if ((port != 0) && !otIp6IsAddressUnspecified(&aUdpSocket->mPeerName.mAddress))
    {
        addr                 = otPlatLwipConvertToLwipAddress(&aUdpSocket->mPeerName.mAddress);
        addr.u_addr.ip6.zone = IP6_NO_ZONE;

        if (pcb->netif_idx == NETIF_NO_INDEX)
        {
            if (ip6_addr_islinklocal(ip_2_ip6(&addr)))
            {
                ip6_addr_assign_zone(ip_2_ip6(&addr), IP6_UNICAST, sBackboneNetifPtr);
            }
        }
        else
        {
            ip6_addr_assign_zone(ip_2_ip6(&addr), IP6_UNICAST, netif_get_by_index(pcb->netif_idx));
        }

        CALL_LWIP_API_FROM_OT_CONTEXT(connectError = udp_connect(pcb, &addr, port));
        VerifyOrExit(connectError == ERR_OK, error = OT_ERROR_FAILED);
    }
    else
    {
        uint8_t oldIfIndex = pcb->netif_idx;

        CALL_LWIP_API_FROM_OT_CONTEXT(udp_disconnect(pcb));

        if (oldIfIndex != NETIF_NO_INDEX)
        {
            CALL_LWIP_API_FROM_OT_CONTEXT(udp_bind_netif(pcb, netif_get_by_index(oldIfIndex)));
        }
    }
exit:
    return error;
}

otError otPlatUdpSend(otUdpSocket *aUdpSocket, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    otError error       = OT_ERROR_NONE;
    err_t   postCbError = ERR_OK;

    struct udpSendContext *udpSendContexPtr = (struct udpSendContext *)otPlatCAlloc(1, sizeof(struct udpSendContext));
    VerifyOrExit(NULL != udpSendContexPtr, error = OT_ERROR_FAILED);

    udpSendContexPtr->pcb = (struct udp_pcb *)aUdpSocket->mHandle;
    VerifyOrExit(udpSendContexPtr->pcb != NULL, error = OT_ERROR_INVALID_ARGS);

    udpSendContexPtr->message = aMessage;
    memcpy(udpSendContexPtr->messageInfo, aMessageInfo, sizeof(otMessageInfo));

    POST_LWIP_CALLBACK_FROM_OT_CONTEXT(postCbError = tcpip_callback(UdpPlatLwipTaskCb, (void *)udpSendContexPtr));
    if (postCbError != ERR_OK)
    {
        otPlatFree(udpSendContexPtr);
        otMessageFree(aMessage);
        aMessage = NULL;
        error    = OT_ERROR_FAILED;
    }

exit:
    if (error != OT_ERROR_NONE)
    {
        if (aMessage != NULL)
        {
            otMessageFree(aMessage);
        }
    }
    return error;
}

otError otPlatUdpJoinMulticastGroup(otUdpSocket        *aUdpSocket,
                                    otNetifIdentifier   aNetifIdentifier,
                                    const otIp6Address *aAddress)
{
    otError error     = OT_ERROR_NONE;
    err_t   joinError = ERR_OK;

    VerifyOrExit(sUdpPlatInit, error = OT_ERROR_INVALID_STATE);
    VerifyOrExit(aUdpSocket->mHandle != NULL, error = OT_ERROR_INVALID_STATE);

    ip_addr_t addr = otPlatLwipConvertToLwipAddress(aAddress);
    if (IP_IS_V4_VAL(addr))
    {
#if LWIP_IPV4

        CALL_LWIP_API_FROM_OT_CONTEXT(joinError =
                                          igmp_joingroup_netif(UdpPlatGetIfPtr(aNetifIdentifier), ip_2_ip4(&addr)));
        VerifyOrExit(joinError == ERR_OK, error = OT_ERROR_FAILED);

#else
        ExitNow(error = OT_ERROR_FAILED);
#endif
    }
    else
    {
        CALL_LWIP_API_FROM_OT_CONTEXT(joinError =
                                          mld6_joingroup_netif(UdpPlatGetIfPtr(aNetifIdentifier), ip_2_ip6(&addr)));
        VerifyOrExit(joinError == ERR_OK, error = OT_ERROR_FAILED);
    }

exit:
    return error;
}

otError otPlatUdpLeaveMulticastGroup(otUdpSocket        *aUdpSocket,
                                     otNetifIdentifier   aNetifIdentifier,
                                     const otIp6Address *aAddress)
{
    otError error      = OT_ERROR_NONE;
    err_t   leaveError = ERR_OK;

    VerifyOrExit(sUdpPlatInit, error = OT_ERROR_INVALID_STATE);
    VerifyOrExit(aUdpSocket->mHandle != NULL, error = OT_ERROR_INVALID_STATE);

    ip_addr_t addr = otPlatLwipConvertToLwipAddress(aAddress);
    if (IP_IS_V4_VAL(addr))
    {
#if LWIP_IPV4

        CALL_LWIP_API_FROM_OT_CONTEXT(leaveError =
                                          igmp_leavegroup_netif(UdpPlatGetIfPtr(aNetifIdentifier), ip_2_ip4(&addr)));
        VerifyOrExit(leaveError == ERR_OK, error = OT_ERROR_FAILED);
#else
        ExitNow(error = OT_ERROR_FAILED);
#endif
    }
    else
    {
        CALL_LWIP_API_FROM_OT_CONTEXT(leaveError =
                                          mld6_leavegroup_netif(UdpPlatGetIfPtr(aNetifIdentifier), ip_2_ip6(&addr)));
        VerifyOrExit(leaveError == ERR_OK, error = OT_ERROR_FAILED);
    }

exit:
    return error;
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

static void UdpPlatLwipSockCb(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    (void)pcb;
    otError error = OT_ERROR_NONE;

    brMsgContext *contextMsgPtr = (brMsgContext *)otPlatCAlloc(1, sizeof(brMsgContext));
    VerifyOrExit(contextMsgPtr != NULL);

    contextMsgPtr->socket = (otUdpSocket *)arg;
    contextMsgPtr->pbuf   = p;

    // messageInfo.mPeerAddr is populated with the remote IPv6 address from which the packet was received.
    contextMsgPtr->messageInfo.mPeerAddr = otPlatLwipConvertToOtAddress(addr);
    // messageInfo.mSockAddr is populated with the destination IPv6 address to which the packet is sent.
    contextMsgPtr->messageInfo.mSockAddr = otPlatLwipConvertToOtAddress((const ip_addr_t *)ip_current_dest_addr());
    // messageInfo.mPeerPort is populated with the remote port from which the packet was received.
    contextMsgPtr->messageInfo.mPeerPort = port;
    contextMsgPtr->messageInfo.mSockPort = contextMsgPtr->socket->mSockName.mPort;

#if LWIP_IPV4
    if (IP_IS_V4_VAL(*addr))
    {
        contextMsgPtr->messageInfo.mHopLimit = IPH_TTL(ip4_current_header());
    }
    else
    {
        contextMsgPtr->messageInfo.mHopLimit = IP6H_HOPLIM(ip6_current_header());
    }
#else
    contextMsgPtr->messageInfo.mHopLimit = IP6H_HOPLIM(ip6_current_header());
#endif

    contextMsgPtr->messageInfo.mIsHostInterface = (netif_get_index(ip_current_netif()) == sBackboneNetifIdx);
    contextMsgPtr->brMsgCallback                = UdpPlatProcessOtReceive;

    BrPostOtMessage(contextMsgPtr);

exit:
    if (contextMsgPtr == NULL)
    {
        pbuf_free(p);
    }
}

static void UdpPlatProcessOtReceive(brMsgContext *aContextMsgPtr)
{
    if (sInstance)
    {
        otMessage *message = otPlatLwipConvertToOtMsg(aContextMsgPtr->pbuf);
        VerifyOrExit(message != NULL);

        aContextMsgPtr->socket->mHandler(aContextMsgPtr->socket->mContext, message, &aContextMsgPtr->messageInfo);
        otMessageFree(message);
    }
exit:
    // Free the pbuf on lwip context to prevent any possible corruption
    (void)pbuf_free_callback(aContextMsgPtr->pbuf);
}

static struct netif *UdpPlatGetIfPtr(otNetifIdentifier aNetifIdentifier)
{
    struct netif *netifPtr = NULL;

    switch (aNetifIdentifier)
    {
    case OT_NETIF_THREAD_HOST:
        netifPtr = sOtNetifPtr;
        break;
    case OT_NETIF_BACKBONE:
        netifPtr = sBackboneNetifPtr;
        break;
    case OT_NETIF_UNSPECIFIED:
    case OT_NETIF_THREAD_INTERNAL:
    default:
        break;
    }

    return netifPtr;
}

static void UdpPlatLwipTaskCb(void *context)
{
    uint8_t netif_idx = NETIF_NO_INDEX;

    struct udpSendContext *udpSendContexPtr = (struct udpSendContext *)context;

    if (udpSendContexPtr->pcb->netif_idx == NETIF_NO_INDEX)
    {
        if (udpSendContexPtr->messageInfo->mIsHostInterface)
        {
            netif_idx = netif_get_index(sBackboneNetifPtr);
        }
        else
        {
            netif_idx = netif_get_index(sOtNetifPtr);
        }
    }
    else
    {
        netif_idx = udpSendContexPtr->pcb->netif_idx;
    }

    ip_addr_t peerAddr = otPlatLwipConvertToLwipAddress(&udpSendContexPtr->messageInfo->mPeerAddr);
    uint16_t  peerPort = udpSendContexPtr->messageInfo->mPeerPort;

    udpSendContexPtr->pcb->local_ip   = otPlatLwipConvertToLwipAddress(&udpSendContexPtr->messageInfo->mSockAddr);
    udpSendContexPtr->pcb->local_port = udpSendContexPtr->messageInfo->mSockPort;

    udpSendContexPtr->pcb->ttl =
        udpSendContexPtr->messageInfo->mHopLimit ? udpSendContexPtr->messageInfo->mHopLimit : UDP_TTL;

    udpSendContexPtr->pcb->flags &= ~(UDP_FLAGS_MULTICAST_LOOP);
    if (udpSendContexPtr->messageInfo->mMulticastLoop)
    {
        udpSendContexPtr->pcb->flags |= (UDP_FLAGS_MULTICAST_LOOP);
    }

    if (!ip_addr_isany(&udpSendContexPtr->pcb->local_ip))
    {
        // Assign zone if the source address has been specified by the application
        ip6_addr_assign_zone(ip_2_ip6(&udpSendContexPtr->pcb->local_ip), IP6_UNICAST, netif_get_by_index(netif_idx));
    }
    else
    {
        udpSendContexPtr->pcb->local_ip.type = IPADDR_TYPE_ANY;
    }

    // The LWIP address needs to be intilialized correctly with a zone
    if (IP_IS_V6_VAL(peerAddr))
    {
        if (ip_addr_ismulticast(&peerAddr))
        {
            ip6_addr_assign_zone(ip_2_ip6(&peerAddr), IP6_MULTICAST, netif_get_by_index(netif_idx));
        }
        else
        {
            ip6_addr_assign_zone(ip_2_ip6(&peerAddr), IP6_UNICAST, netif_get_by_index(netif_idx));
        }
    }

    struct pbuf *buffer = otPlatLwipConvertToLwipMsg(udpSendContexPtr->message, true);
    VerifyOrExit(buffer != NULL);
    (void)udp_sendto(udpSendContexPtr->pcb, buffer, &peerAddr, peerPort);
    pbuf_free(buffer);
    buffer = NULL;

exit:
    gLockTaskCb();
    otMessageFree(udpSendContexPtr->message);
    gUnlockTaskCb();
    otPlatFree(context);
    context = NULL;
    return;
}
