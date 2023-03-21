/*
 *  Copyright (c) 2023, The OpenThread Authors.
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

#include "ot_lwip.h"

#include <string.h>

#include <common/code_utils.hpp>
#include <openthread-core-config.h>
#include <openthread/icmp6.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>

#include "lwip/tcpip.h"

/* -------------------------------------------------------------------------- */
/*                                 Definitions                                */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

static struct netif sThreadNetIf;
static otInstance  *sInstance = NULL;
static bool         sAddrAssigned[LWIP_IPV6_NUM_ADDRESSES];

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

static err_t otPlatLwipThreadNetIfInitCallback(struct netif *netif);
static err_t otPlatLwipSendPacket(struct netif *netif, struct pbuf *pkt, const struct ip6_addr *ipaddr);
static void  otPlatLwipReceivePacket(otMessage *pkt, void *context);

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void otPlatLwipInit(otInstance *aInstance)
{
    VerifyOrExit(aInstance != NULL);

    sInstance = aInstance;

    /* Initialize LWIP stack */
    tcpip_init(NULL, NULL);

    memset(sAddrAssigned, 0, sizeof(sAddrAssigned));

exit:
    return;
}

void otPlatLwipAddThreadInterface(void)
{
    struct netif *pNetIf;

    /* otPlatLwipInit must be called first */
    VerifyOrExit(sInstance != NULL);

    /* Lock LwIP stack */
    LOCK_TCPIP_CORE();

    /* Initialize a LwIP netif structure for the OpenThread interface and add it to the list of interfaces known to
     * LwIP. */
    pNetIf = netif_add(&sThreadNetIf, NULL, NULL, NULL, NULL, otPlatLwipThreadNetIfInitCallback, tcpip_input);

    /* Start with the interface in the down state. */
    netif_set_link_down(&sThreadNetIf);

    /* Unkock LwIP stack */
    UNLOCK_TCPIP_CORE();

    VerifyOrExit(pNetIf != NULL);

    /* Arrange for OpenThread to call our otPlatLwipReceivePacket() method whenever an IPv6 packet is received. */
    otIp6SetReceiveCallback(sInstance, otPlatLwipReceivePacket, NULL);
    /* Disable automatic echo mode in OpenThread. */
    otIcmp6SetEchoMode(sInstance, OT_ICMP6_ECHO_HANDLER_DISABLED);
    /* Enable the receive filter for Thread control traffic. */
    otIp6SetReceiveFilterEnabled(sInstance, true);

exit:
    return;
}

void otPlatLwipUpdateState(otChangedFlags flags, void *context)
{
    // If the Thread device role has changed, or if an IPv6 address has been added or
    // removed in the Thread stack, update the state and configuration of the LwIP Thread interface.
    if (flags & (OT_CHANGED_THREAD_ROLE | OT_CHANGED_IP6_ADDRESS_ADDED | OT_CHANGED_IP6_ADDRESS_REMOVED))
    {
        err_t lwipErr = ERR_OK;
        bool  isInterfaceUp;
        bool  addrChange = flags & (OT_CHANGED_IP6_ADDRESS_ADDED | OT_CHANGED_IP6_ADDRESS_REMOVED) ? true : false;
        bool  addrAssigned[LWIP_IPV6_NUM_ADDRESSES];

        memset(addrAssigned, 0, sizeof(addrAssigned));

        // Lock LwIP stack.
        LOCK_TCPIP_CORE();

        // Determine whether the device Thread interface is up..
        isInterfaceUp = otIp6IsEnabled(sInstance);

        // If needed, adjust the link state of the LwIP netif to reflect the state of the OpenThread stack.
        // Set ifConnectivity to indicate the change in the link state.
        if (isInterfaceUp != (bool)netif_is_link_up(&sThreadNetIf))
        {
            if (isInterfaceUp)
            {
                netif_set_link_up(&sThreadNetIf);
            }
            else
            {
                netif_set_link_down(&sThreadNetIf);
            }

            // Presume the interface addresses are also changing.
            addrChange = true;
        }

        // If needed, adjust the set of addresses associated with the LwIP netif to reflect those
        // known to the Thread stack.
        if (addrChange)
        {
            // If attached to a Thread network, add addresses to the LwIP netif to match those
            // configured in the Thread stack...
            if (isInterfaceUp)
            {
                // Enumerate the list of unicast IPv6 addresses known to OpenThread...
                const otNetifAddress *otAddrs = otIp6GetUnicastAddresses(sInstance);
                for (const otNetifAddress *otAddr = otAddrs; otAddr != NULL; otAddr = otAddr->mNext)
                {
                    // Assign the following OpenThread addresses to LwIP's address table:
                    //   - link-local addresses.
                    //   - mesh-local addresses that are NOT RLOC addresses.
                    //   - global unicast addresses.
                    if (otAddr->mValid && !otAddr->mRloc)
                    {
                        ip6_addr_t lwipAddr = {0};
                        memcpy(&lwipAddr.addr, otAddr->mAddress.mFields.m8, sizeof(lwipAddr.addr));
                        int8_t addrIdx;

                        // Add the address to the LwIP netif.  If the address is a link-local, and the primary
                        // link-local address* for the LwIP netif has not been set already, then use
                        // netif_ip6_addr_set() to set the primary address.  Otherwise use netif_add_ip6_address(). This
                        // special case is required because LwIP's netif_add_ip6_address() will never set the primary
                        // link-local address.
                        //
                        // * -- The primary link-local address always appears in the first slot in the netif address
                        // table.
                        //
                        if (ip6_addr_islinklocal(&lwipAddr) && !addrAssigned[0])
                        {
                            netif_ip6_addr_set(&sThreadNetIf, 0, &lwipAddr);
                            addrIdx = 0;
                        }
                        else
                        {
                            // Add the address to the LwIP netif.  If the address table fills (ERR_VAL), simply stop
                            // adding addresses.  If something else fails, log it and soldier on.
                            lwipErr = netif_add_ip6_address(&sThreadNetIf, &lwipAddr, &addrIdx);
                            if (lwipErr == ERR_VAL)
                            {
                                break;
                            }
                        }
                        // Set non-mesh-local address state to PREFERRED or ACTIVE depending on the state in OpenThread.
                        netif_ip6_addr_set_state(
                            &sThreadNetIf, addrIdx,
                            (otAddr->mPreferred && otAddr->mAddressOrigin != OT_ADDRESS_ORIGIN_THREAD)
                                ? IP6_ADDR_PREFERRED
                                : IP6_ADDR_VALID);

                        // Record that the netif address slot was assigned during this loop.
                        addrAssigned[addrIdx] = true;
                    }
                }
            }
            // For each address associated with the netif that was *not* assigned above, remove the address
            // from the netif if the address is one that was previously assigned by this method.
            // In the case where the device is no longer attached to a Thread network, remove all addresses
            // from the netif.
            for (u8_t addrIdx = 0; addrIdx < LWIP_IPV6_NUM_ADDRESSES; addrIdx++)
            {
                if (!isInterfaceUp || (sAddrAssigned[addrIdx] && !addrAssigned[addrIdx]))
                {
                    // Remove the address from the netif by setting its state to INVALID
                    netif_ip6_addr_set_state(&sThreadNetIf, addrIdx, IP6_ADDR_INVALID);
                }
            }

            // Remember the set of assigned addresses.
            memcpy(sAddrAssigned, addrAssigned, sizeof(sAddrAssigned));
        }

        UNLOCK_TCPIP_CORE();
    }
}

struct pbuf *otPlatLwipConvertToLwipMsg(otMessage *otIpPkt, bool bTransport)
{
    struct pbuf *lwipIpPkt    = NULL;
    bool         bFreeLwipPkt = false;
    uint16_t     lwipIpPktLen = otMessageGetLength(otIpPkt);

    // Allocate an LwIP pbuf to hold the inbound packet.
    if (bTransport)
    {
        lwipIpPkt = pbuf_alloc(PBUF_TRANSPORT, lwipIpPktLen, PBUF_RAM);
    }
    else
    {
        lwipIpPkt = pbuf_alloc(PBUF_LINK, lwipIpPktLen, PBUF_POOL);
    }

    VerifyOrExit(lwipIpPkt != NULL);

    // Copy the packet data from the OpenThread message object to the pbuf.
    if (otMessageRead(otIpPkt, 0, lwipIpPkt->payload, lwipIpPktLen) != lwipIpPktLen)
    {
        ExitNow(bFreeLwipPkt = true);
    }

exit:
    if (bFreeLwipPkt)
    {
        pbuf_free(lwipIpPkt);
        lwipIpPkt = NULL;
    }
    return lwipIpPkt;
}

otMessage *otPlatLwipConvertToOtMsg(struct pbuf *lwipIpPkt)
{
    otMessage              *otIpPkt     = NULL;
    const otMessageSettings msgSettings = {true, OT_MESSAGE_PRIORITY_NORMAL};
    uint16_t                remainingLen;
    bool                    bFreeOtPkt = false;

    // Allocate an OpenThread message
    otIpPkt = otIp6NewMessage(sInstance, &msgSettings);
    VerifyOrExit(otIpPkt != NULL);

    // Copy data from LwIP's packet buffer chain into the OpenThread message.
    remainingLen = lwipIpPkt->tot_len;
    for (struct pbuf *partialPkt = lwipIpPkt; partialPkt != NULL; partialPkt = partialPkt->next)
    {
        VerifyOrExit(partialPkt->len <= remainingLen, bFreeOtPkt = true);

        VerifyOrExit(otMessageAppend(otIpPkt, partialPkt->payload, partialPkt->len) == OT_ERROR_NONE,
                     bFreeOtPkt = true);
        remainingLen = (uint16_t)(remainingLen - partialPkt->len);
    }
    VerifyOrExit(remainingLen == 0, bFreeOtPkt = true);

exit:
    if (bFreeOtPkt)
    {
        otMessageFree(otIpPkt);
        otIpPkt = NULL;
    }
    return otIpPkt;
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

static err_t otPlatLwipThreadNetIfInitCallback(struct netif *netif)
{
    netif->name[0]    = 'o';
    netif->name[1]    = 't';
    netif->output_ip6 = otPlatLwipSendPacket;
    netif->output     = NULL;
    netif->linkoutput = NULL;
    netif->flags      = NETIF_FLAG_UP | NETIF_FLAG_LINK_UP | NETIF_FLAG_BROADCAST;
    netif->mtu        = OPENTHREAD_CONFIG_IP6_MAX_DATAGRAM_LENGTH;
    return ERR_OK;
}

static err_t otPlatLwipSendPacket(struct netif *netif, struct pbuf *pkt, const struct ip6_addr *ipaddr)
{
    err_t      lwipErr = ERR_IF;
    otMessage *otIpPkt = otPlatLwipConvertToOtMsg(pkt);

    if (otIpPkt != NULL)
    {
        /* Pass the packet to OpenThread to be sent.  Note that OpenThread takes care of releasing the otMessage object
         * regardless of whether otIp6Send() succeeds or fails. */
        if (otIp6Send(sInstance, otIpPkt) == OT_ERROR_NONE)
        {
            lwipErr = ERR_OK;
        }
    }

    /* pktPBuf is freed by LWIP stack */
    return lwipErr;
}

static void otPlatLwipReceivePacket(otMessage *pkt, void *context)
{
    struct pbuf *lwipIpPkt = otPlatLwipConvertToLwipMsg(pkt, false);

    if (lwipIpPkt != NULL)
    {
        /* Deliver the packet to the input function associated with the LwIP netif.
         * NOTE: The input function posts the inbound packet to LwIP's TCPIP thread.
         * Thus there's no need to acquire the LwIP TCPIP core lock here. */
        if (sThreadNetIf.input(lwipIpPkt, &sThreadNetIf) != ERR_OK)
        {
            pbuf_free(lwipIpPkt);
        }
    }

    /* Always free the OpenThread message. */
    otMessageFree(pkt);
}
