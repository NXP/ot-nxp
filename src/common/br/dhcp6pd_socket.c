/*
 *  Copyright (c) 2025, The OpenThread Authors.
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
#include "dhcp6pd_socket.h"
#include "br_rtos_manager.h"
#include "ot_lwip.h"

#include <openthread/platform/infra_if.h>
#include <openthread/platform/memory.h>
#include <openthread/platform/messagepool.h>
#include <openthread/platform/udp.h>
#include "lwip/ip_addr.h"
#include "lwip/udp.h"

#include "openthread-core-config.h"

/* -------------------------------------------------------------------------- */
/*                                 Definitions                                */
/* -------------------------------------------------------------------------- */
#define DHCPv6_SERVER_PORT 547
#define DHCPv6_CLIENT_PORT 546

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */
static otInstance     *sInstance;
static struct udp_pcb *sDhcpPdPcb;
static uint32_t        sInfraIfIndex;
static bool            sDhcpPdIsEnabled;

struct udpSendContext
{
    struct udp_pcb *pcb;
    otMessage      *message;
    otIp6Address    mIp6Address;
};
/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

static void Dhcp6PdSocketReceive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
static void SocketInit(uint32_t aInfraIfIndex);
static void SocketDeInit(uint32_t aInfraIfIndex);
static void LwipTaskCb(void *aContext);
static void Dhcp6PdProcessOtReceive(brMsgContext *aMsgContextPtr);

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void Dhcp6PdSocketInit(otInstance *aInstance, uint32_t aInfraIfIndex)
{
    sInstance     = aInstance;
    sInfraIfIndex = aInfraIfIndex;
}

void otPlatInfraIfDhcp6PdClientSetListeningEnabled(otInstance *aInstance, bool aEnable, uint32_t aInfraIfIndex)
{

    if (aEnable)
    {
        SocketInit(aInfraIfIndex);
    }
    else
    {
        SocketDeInit(aInfraIfIndex);
    }
}

void otPlatInfraIfDhcp6PdClientSend(otInstance   *aInstance,
                                    otMessage    *aMessage,
                                    otIp6Address *aDestAddress,
                                    uint32_t      aInfraIfIndex)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError error       = OT_ERROR_NONE;
    err_t   postCbError = ERR_OK;

    if (sDhcpPdIsEnabled && aInfraIfIndex == sInfraIfIndex)
    {
        struct udpSendContext *udpSendContexPtr =
            (struct udpSendContext *)otPlatCAlloc(1, sizeof(struct udpSendContext));
        VerifyOrExit(NULL != udpSendContexPtr, error = OT_ERROR_FAILED);

        udpSendContexPtr->mIp6Address = *aDestAddress;
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

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

static void Dhcp6PdSocketReceive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    (void)pcb;
    brMsgContext *contextMsgPtr = NULL;

    VerifyOrExit(sDhcpPdIsEnabled);

    contextMsgPtr = (brMsgContext *)otPlatCAlloc(1, sizeof(brMsgContext));
    VerifyOrExit(contextMsgPtr != NULL);

    contextMsgPtr->socket = (otUdpSocket *)arg;
    contextMsgPtr->pbuf   = p;

    contextMsgPtr->addrInfo.mAddress      = otPlatLwipConvertToOtAddress(addr);
    contextMsgPtr->addrInfo.mPort         = port;
    contextMsgPtr->addrInfo.mInfraIfIndex = sInfraIfIndex;
    contextMsgPtr->brMsgCallback          = Dhcp6PdProcessOtReceive;

    BrPostOtMessage(contextMsgPtr);

exit:
    if (contextMsgPtr == NULL)
    {
        pbuf_free(p);
    }
}

static void Dhcp6PdProcessOtReceive(brMsgContext *aContextMsgPtr)
{
    VerifyOrExit(sDhcpPdIsEnabled);

    otMessage *message = otPlatLwipConvertToOtMsg(aContextMsgPtr->pbuf);
    VerifyOrExit(message != NULL);

    // message is owned by OT, no need to free it explicitly
    otPlatInfraIfDhcp6PdClientHandleReceived(sInstance, message, sInfraIfIndex);

exit:
    // Free the pbuf on lwip context to prevent any possible corruption
    (void)pbuf_free_callback(aContextMsgPtr->pbuf);
}

static void SocketInit(uint32_t aInfraIfIndex)
{
    otError error     = OT_ERROR_FAILED;
    err_t   lwipError = ERR_OK;
    VerifyOrExit(aInfraIfIndex == sInfraIfIndex, error = OT_ERROR_INVALID_ARGS);

    CALL_LWIP_API_FROM_OT_CONTEXT(sDhcpPdPcb = udp_new_ip_type(IPADDR_TYPE_V6));
    if (sDhcpPdPcb != NULL)
    {
        CALL_LWIP_API_FROM_OT_CONTEXT(lwipError = udp_bind(sDhcpPdPcb, IP_ANY_TYPE, DHCPv6_CLIENT_PORT));
        VerifyOrExit(lwipError == ERR_OK, error = OT_ERROR_FAILED);

        CALL_LWIP_API_FROM_OT_CONTEXT(udp_bind_netif(sDhcpPdPcb, netif_get_by_index(aInfraIfIndex)));
        CALL_LWIP_API_FROM_OT_CONTEXT(udp_recv(sDhcpPdPcb, Dhcp6PdSocketReceive, NULL));

        sDhcpPdIsEnabled = true;
        error            = OT_ERROR_NONE;
    }

exit:
    assert(error != OT_ERROR_FAILED);
    // Do nothing when error is OT_ERROR_INVALID_ARGS
    return;
}

static void SocketDeInit(uint32_t aInfraIfIndex)
{
    VerifyOrExit(aInfraIfIndex == sInfraIfIndex);
    VerifyOrExit(sDhcpPdIsEnabled);

    CALL_LWIP_API_FROM_OT_CONTEXT(udp_remove(sDhcpPdPcb));

    sDhcpPdIsEnabled = false;
exit:
    return;
}

static void LwipTaskCb(void *aContext)
{
    struct udpSendContext *udpSendContexPtr = (struct udpSendContext *)aContext;
    struct pbuf           *buffer           = NULL;

    buffer = otPlatLwipConvertToLwipMsg(udpSendContexPtr->message, true);
    if (buffer != NULL)
    {
        ip_addr_t peerAddress = otPlatLwipConvertToLwipAddress((const otIp6Address *)&udpSendContexPtr->mIp6Address);
        (void)udp_sendto(sDhcpPdPcb, buffer, &peerAddress, DHCPv6_SERVER_PORT);
        pbuf_free(buffer);
        buffer = NULL;
    }

    gLockTaskCb();
    otMessageFree(udpSendContexPtr->message);
    gUnlockTaskCb();

    otPlatFree(aContext);
    aContext = NULL;

    return;
}
