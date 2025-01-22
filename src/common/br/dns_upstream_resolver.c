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
 *   This file implements the OpenThread platform abstraction for DNS resolver.
 *
 */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include "dns_upstream_resolver.h"
#include "br_rtos_manager.h"
#include "udp_plat.h"
#include <openthread/nat64.h>
#include <openthread/udp.h>
#include <openthread/platform/dns.h>
#include <openthread/platform/memory.h>
#include <openthread/platform/udp.h>
#include "common/code_utils.hpp"
#include "lwip/dns.h"
#include "lwip/udp.h"

/* -------------------------------------------------------------------------- */
/*                                 Definitions                                */
/* -------------------------------------------------------------------------- */

#define MAX_CONCURRENT_TRANSACTIONS 8
#define DNS_MSG_MAX_SIZE 512

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

static otInstance   *sInstance;
static struct netif *sBackboneNetif;

static ip_addr_t sUpstreamDnsServers[DNS_MAX_SERVERS];
static uint8_t   sUpstreamDnsServerCount;

static otUdpSocket sTransactions[MAX_CONCURRENT_TRANSACTIONS];

static const otIp6Address kAnyAddress = {
    .mFields.m8 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

static void            GetDnsServerList(void);
static void            SendQuery(otPlatDnsUpstreamQuery *aTxn, const otMessage *aMessage);
static void            SendResponse(otPlatDnsUpstreamQuery *aTxn, otMessage *aMessage);
static otUdpSocket    *CreateTransaction(otPlatDnsUpstreamQuery *aTxn);
static void            CloseTransaction(otPlatDnsUpstreamQuery *aTxn);
static void            CancelTransaction(otPlatDnsUpstreamQuery *aTxn);
static struct udp_pcb *CreateUdpSocket(otUdpSocket *aUdpSocket);
static otUdpSocket    *GetTransactionSocket(otPlatDnsUpstreamQuery *aTxn);
static void            OnUdpReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);
static void            DnsUpstreamResolverUdpReceive(void            *arg,
                                                     struct udp_pcb  *pcb,
                                                     struct pbuf     *p,
                                                     const ip_addr_t *addr,
                                                     u16_t            port);
static void            DnsUpstreamResolverProcessOtReceive(brMsgContext *aContextMsgPtr);
/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void DnsResolverInit(otInstance *aInstance, struct netif *aBackboneNetif)
{
    sInstance      = aInstance;
    sBackboneNetif = aBackboneNetif;
}

void otPlatDnsStartUpstreamQuery(otInstance *aInstance, otPlatDnsUpstreamQuery *aTxn, const otMessage *aQuery)
{
    OT_UNUSED_VARIABLE(aInstance);
    SendQuery(aTxn, aQuery);
}

void otPlatDnsCancelUpstreamQuery(otInstance *aInstance, otPlatDnsUpstreamQuery *aTxn)
{
    OT_UNUSED_VARIABLE(aInstance);
    CancelTransaction(aTxn);
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

void GetDnsServerList()
{
    sUpstreamDnsServerCount = 0;
    for (uint8_t i = 0; i < DNS_MAX_SERVERS; i++)
    {
        if (!ip_addr_isany(dns_getserver(i)))
        {
            sUpstreamDnsServers[i] = *dns_getserver(i);
            if (IP_IS_V4_VAL(sUpstreamDnsServers[i]))
            {
                ip4_2_ipv4_mapped_ipv6(ip_2_ip6(&sUpstreamDnsServers[i]), ip_2_ip4(&sUpstreamDnsServers[i]));
                sUpstreamDnsServers[i].type = IPADDR_TYPE_V6;
            }
            sUpstreamDnsServerCount++;
        }
    }
}

static void SendQuery(otPlatDnsUpstreamQuery *aTxn, const otMessage *aQuery)
{
    otError      error        = OT_ERROR_NONE;
    otUdpSocket *txn          = NULL;
    otMessage   *message      = NULL;
    uint8_t     *packetToSend = NULL;

    otMessageInfo     messageInfo = {0};
    otMessageSettings msgSettings = {.mLinkSecurityEnabled = false, .mPriority = OT_MESSAGE_PRIORITY_NORMAL};

    uint16_t length = otMessageGetLength(aQuery);
    packetToSend    = (uint8_t *)otPlatCAlloc(1, length);
    VerifyOrExit(packetToSend != NULL, error = OT_ERROR_NO_BUFS);

    VerifyOrExit(otMessageRead(aQuery, 0, packetToSend, length) == length, error = OT_ERROR_NO_BUFS);

    GetDnsServerList();
    txn = CreateTransaction(aTxn);
    VerifyOrExit(txn != NULL, error = OT_ERROR_NO_BUFS);

    messageInfo.mPeerPort = 53;
    messageInfo.mSockPort = txn->mSockName.mPort;
    messageInfo.mSockAddr = kAnyAddress;

    for (uint8_t i = 0; i < sUpstreamDnsServerCount; i++)
    {
        memcpy(&messageInfo.mPeerAddr, ip_2_ip6(&sUpstreamDnsServers[i])->addr, sizeof(messageInfo.mPeerAddr));

        message = otUdpNewMessage(sInstance, &msgSettings);
        VerifyOrExit(message != NULL, error = OT_ERROR_NO_BUFS);

        VerifyOrExit(otMessageAppend(message, packetToSend, length) == OT_ERROR_NONE, error = OT_ERROR_FAILED);

        error = otPlatUdpSend(txn, message, &messageInfo);
    }
exit:
    if (error != OT_ERROR_NONE)
    {
        if (message)
        {
            otMessageFree(message);
        }
    }
    if (packetToSend)
    {
        otPlatFree(packetToSend);
    }
    return;
}

static void SendResponse(otPlatDnsUpstreamQuery *aTxn, otMessage *aMessage)
{
    otPlatDnsUpstreamQueryDone(sInstance, aTxn, aMessage);
    CloseTransaction(aTxn);
}

static otUdpSocket *CreateTransaction(otPlatDnsUpstreamQuery *aTxn)
{
    otUdpSocket *txn = NULL;

    for (uint8_t i = 0; i < MAX_CONCURRENT_TRANSACTIONS; i++)
    {
        if (sTransactions[i].mContext == aTxn)
        {
            return NULL;
        }
        if (sTransactions[i].mContext == NULL)
        {
            struct udp_pcb *pcb = CreateUdpSocket(&sTransactions[i]);
            if (pcb == NULL)
            {
                break;
            }

            txn                       = &sTransactions[i];
            sTransactions[i].mContext = aTxn;
            break;
        }
    }

    return txn;
}

static void CloseTransaction(otPlatDnsUpstreamQuery *aTxn)
{
    otUdpSocket *sock = NULL;

    sock = GetTransactionSocket(aTxn);

    if (sock)
    {
        otPlatUdpClose(sock);
        sock->mContext        = NULL;
        sock->mSockName.mPort = 0;
    }
}

static void CancelTransaction(otPlatDnsUpstreamQuery *aTxn)
{
    CloseTransaction(aTxn);
}

static otUdpSocket *GetTransactionSocket(otPlatDnsUpstreamQuery *aTxn)
{
    for (uint8_t i = 0; i < MAX_CONCURRENT_TRANSACTIONS; i++)
    {
        if (sTransactions[i].mContext == aTxn)
        {
            return &sTransactions[i];
        }
    }

    return NULL;
}

static struct udp_pcb *CreateUdpSocket(otUdpSocket *aUdpSocket)
{
    struct udp_pcb *pcb = NULL;

    aUdpSocket->mHandler = OnUdpReceive;

    CALL_LWIP_API_FROM_OT_CONTEXT({
        pcb = udp_new();
        if (pcb != NULL)
        {
            udp_recv(pcb, DnsUpstreamResolverUdpReceive, aUdpSocket);
            aUdpSocket->mHandle = pcb;
        }
    });
    VerifyOrExit(pcb != NULL);
    VerifyOrExit(otPlatUdpBind(aUdpSocket) == OT_ERROR_NONE);
    VerifyOrExit(otPlatUdpBindToNetif(aUdpSocket, OT_NETIF_BACKBONE) == OT_ERROR_NONE);

    aUdpSocket->mSockName.mPort = pcb->local_port;

    return pcb;
exit:
    if (pcb != NULL)
    {
        CALL_LWIP_API_FROM_OT_CONTEXT(udp_remove(pcb));
    }
    return NULL;
}

static void OnUdpReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    OT_UNUSED_VARIABLE(aMessageInfo);
    SendResponse(aContext, aMessage);
}

static void DnsUpstreamResolverUdpReceive(void            *arg,
                                          struct udp_pcb  *pcb,
                                          struct pbuf     *p,
                                          const ip_addr_t *addr,
                                          u16_t            port)
{
    (void)pcb;
    otError error = OT_ERROR_NONE;

    brMsgContext *contextMsgPtr = (brMsgContext *)otPlatCAlloc(1, sizeof(brMsgContext));
    VerifyOrExit(contextMsgPtr != NULL);

    contextMsgPtr->socket = (otUdpSocket *)arg;
    contextMsgPtr->pbuf   = p;

    contextMsgPtr->brMsgCallback = DnsUpstreamResolverProcessOtReceive;

    BrPostOtMessage(contextMsgPtr);

exit:
    if (contextMsgPtr == NULL)
    {
        pbuf_free(p);
    }
}

static void DnsUpstreamResolverProcessOtReceive(brMsgContext *aContextMsgPtr)
{
    if (sInstance)
    {
        otMessage *message = otPlatLwipConvertToOtMsg(aContextMsgPtr->pbuf);
        VerifyOrExit(message != NULL);

        // message is owned by OT, no need to free it explicitly
        aContextMsgPtr->socket->mHandler(aContextMsgPtr->socket->mContext, message, &aContextMsgPtr->messageInfo);
    }
exit:
    // Free the pbuf on lwip context to prevent any possible corruption
    (void)pbuf_free_callback(aContextMsgPtr->pbuf);
}
