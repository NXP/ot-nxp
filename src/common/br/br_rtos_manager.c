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
 *   This file implements the Border Router's initialization helper functions.
 *
 */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include "br_rtos_manager.h"
#include "border_agent.h"
#include "dns_upstream_resolver.h"
#include "infra_if.h"
#include "mdns_socket.h"
#include "ot_lwip.h"
#include "trel_plat.h"
#include "udp_plat.h"
#include "utils.h"

#include "lwip_hooks.h"
#include "lwip_mcast.h"
#include "lwip/dhcp6.h"

#include <openthread/backbone_router_ftd.h>
#include <openthread/border_router.h>
#include <openthread/dnssd_server.h>
#include <openthread/mdns.h>
#include <openthread/nat64.h>
#include <openthread/srp_server.h>
#include <openthread/platform/border_routing.h>
#include <openthread/platform/infra_if.h>

#include <string.h>

/* -------------------------------------------------------------------------- */
/*                                 Definitions                                */
/* -------------------------------------------------------------------------- */

#define MAX_HOST_IPV6_ADDRESSES 4

/* -------------------------------------------------------------------------- */
/*                             Private memory                                 */
/* -------------------------------------------------------------------------- */

static otInstance          *sInstance    = NULL;
static struct netif        *sExtNetif    = NULL;
static struct netif        *sThreadNetif = NULL;
static netif_ext_callback_t sNetifCallback;

static bool sDnsHostInitialized    = false;
static bool sBrIsInitialized       = false;
static bool sExternalNetifState    = false;
static bool sNat64TranslatorEnable = true;

static otMdnsHost   sHost;
static otIp6Address sHostAddresses[MAX_HOST_IPV6_ADDRESSES];

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */
static void HandleMulticastListenerCallback(void                                  *aContext,
                                            otBackboneRouterMulticastListenerEvent aEvent,
                                            const otIp6Address                    *aAddress);

static void Dhcp6PrefixChangedCb(struct netif *netif, const struct dhcp6_delegated_prefix *prefix, u8_t valid);
static void otDhcpPdCb(otBorderRoutingDhcp6PdState aState, void *aContext);

static void HandleMdnsRegisterCallback(otInstance *aInstance, otMdnsRequestId aRequestId, otError aError);
static bool UpdateIp6AddressList();
/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void BrInitPlatform(otInstance *aInstance, struct netif *aExtNetif, struct netif *aThreadNetif)
{
    sInstance    = aInstance;
    sExtNetif    = aExtNetif;
    sThreadNetif = aThreadNetif;

#if OT_APP_BR_LWIP_HOOKS_EN
    lwipHooksInit(sInstance, sExtNetif, aThreadNetif);
#endif
    UdpPlatInit(sInstance, sExtNetif, aThreadNetif);
    InfraIfInit(sInstance, sExtNetif);
    MdnsSocketInit(sInstance, netif_get_index(sExtNetif));
    TrelPlatInit(sInstance, sExtNetif);

    netif_add_ext_callback(&sNetifCallback, &BrNetifExtCb);
}

void BrUpdateLwipThrIf(otPlatLockTaskCb lockTaskCb, otPlatUnlockTaskCb unlockCb)
{
    // Update the LWIP Thread interface to use the send/receive functions from otPLatLwip.
    // These functions supoprt NAT64 and rate limiting.
    otPlatLwipInit(lockTaskCb, unlockCb);
    otPlatLwipSetOtInstance(sInstance);
    otPlatLwipAddThreadInterface(sThreadNetif);
}

void BrSetNat64TranslatorState(bool aEnable)
{
    if (sNat64TranslatorEnable != aEnable)
    {
        sNat64TranslatorEnable = aEnable;
        otNat64SetEnabled(sInstance, sNat64TranslatorEnable);
    }
}

void BrInitServices()
{
    if ((sInstance != NULL) && (sExtNetif != NULL))
    {
        otBorderRoutingInit(sInstance, netif_get_index(sExtNetif), true);
        otBorderRoutingSetEnabled(sInstance, true);
        otBorderRoutingDhcp6PdSetEnabled(sInstance, true);
        otBackboneRouterSetEnabled(sInstance, true);
        otBackboneRouterSetMulticastListenerCallback(sInstance, HandleMulticastListenerCallback, sExtNetif);
        otSrpServerSetAutoEnableMode(sInstance, true);
        otBorderRoutingDhcp6PdSetRequestCallback(sInstance, otDhcpPdCb, NULL);

#if OPENTHREAD_CONFIG_NAT64_TRANSLATOR_ENABLE || OPENTHREAD_CONFIG_NAT64_BORDER_ROUTING_ENABLE
        otNat64SetEnabled(sInstance, true);
#endif
#if OPENTHREAD_CONFIG_DNS_UPSTREAM_QUERY_ENABLE
        otDnssdUpstreamQuerySetEnabled(sInstance, true);
        DnsResolverInit(sInstance, sExtNetif);
#endif
    }
}

void BrInitMdnsHost(const char *aHostName)
{
    sHost.mHostName        = aHostName;
    sHost.mAddressesLength = 0;
    sHost.mTtl             = 120;
    sHost.mInfraIfIndex    = netif_get_index(sExtNetif);
    sHost.mAddresses       = sHostAddresses;

    if (netif_is_link_up(sExtNetif))
    {
        netif_ext_callback_args_t args = {0};
        args.link_changed.state        = true;
        netif_nsc_reason_t cbFlags     = LWIP_NSC_LINK_CHANGED | LWIP_NSC_IPV6_SET;
        if (!ip4_addr_isany(netif_ip4_addr(sExtNetif)))
        {
            cbFlags |= LWIP_NSC_IPV4_ADDRESS_CHANGED;
        }
        BrNetifExtCb(sExtNetif, cbFlags, &args);
    }
}

void BrMdnsHostSetInitialized(bool aState)
{
    sDnsHostInitialized = aState;
}

bool BrMdnsHostIsInitialized()
{
    return sDnsHostInitialized;
}

void BrNetifExtCb(struct netif *netif, netif_nsc_reason_t reason, const netif_ext_callback_args_t *args)
{
    if (netif == sExtNetif)
    {
        if ((reason & LWIP_NSC_LINK_CHANGED))
        {
            sExternalNetifState = args->link_changed.state;
            otPlatInfraIfStateChanged(sInstance, netif_get_index(sExtNetif), sExternalNetifState);
            if (sExternalNetifState)
            {
                if (!sBrIsInitialized)
                {
                    BrInitServices();
                    sBrIsInitialized = true;
                }
                else
                {
                    otMdnsRegisterHost(sInstance, &sHost, 0, HandleMdnsRegisterCallback);
                }
            }
            else
            {
                BorderAgentDeInit();
                TrelOnExternalNetifDown();
            }
        }
        if ((reason & (LWIP_NSC_IPV6_SET | LWIP_NSC_IPV6_ADDR_STATE_CHANGED)) && sExternalNetifState)
        {
            if (UpdateIp6AddressList())
            {
                otMdnsRegisterHost(sInstance, &sHost, 0, HandleMdnsRegisterCallback);
            }
        }
#if OPENTHREAD_CONFIG_NAT64_TRANSLATOR_ENABLE || OPENTHREAD_CONFIG_NAT64_BORDER_ROUTING_ENABLE
        if ((reason & LWIP_NSC_IPV4_ADDRESS_CHANGED))
        {
            otIp4Cidr         aCidr;
            const ip4_addr_t *ip4Addr               = netif_ip4_addr(sExtNetif);
            const ip4_addr_t *ip4DefRoute           = netif_ip4_gw(sExtNetif);
            bool              bNat64TranslatorState = false;

            // Evaluate conditions to enable / disable NAT64. According to specification we must have a valid IPv4
            // address and a default gateway in order to enable 6 to 4 translation.
            if (!ip4_addr_isany(ip4Addr) && !ip4_addr_isany(ip4DefRoute))
            {
                // A default gateway is available on this interface, for LWIP this is translated to a default
                // interface. If no default interface is set public, addresses will not be forwarded to the
                // default gateway.
                netif_set_default(sExtNetif);

                // Bind the device's IPv4 address to the RAW sockets used for receiving IPv4 traffic to filter
                // out any other packets that might be received by the NAT64 translator, like multicast traffic.
                InfraIfNat64Init();

                aCidr.mAddress.mFields.m32 = ip4Addr->addr;
                aCidr.mLength              = 32U;
                // Ignore error for the call, can only fail if the cidr len is 0 but we are always setting it to 32.
                otNat64SetIp4Cidr(sInstance, &aCidr);
                // Only enable if master flag is true. The spec requires that the user can decide to disable NAT64
                // translation but default value is true.
                bNat64TranslatorState = sNat64TranslatorEnable;
            }
            else
            {
                // Disable the default interface, with no default gateway or IPv4 address it cannot be used.
                netif_set_default(sExtNetif);
            }
            otNat64SetEnabled(sInstance, bNat64TranslatorState);
        }
#endif /* OPENTHREAD_CONFIG_NAT64_TRANSLATOR_ENABLE */
    }
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */
static void Dhcp6PrefixChangedCb(struct netif *netif, const struct dhcp6_delegated_prefix *prefix, u8_t valid)
{
    if ((netif != NULL) && (prefix != NULL) && (valid == true))
    {
        otBorderRoutingPrefixTableEntry prefixEntry = {0};

        prefixEntry.mIsOnLink          = true;
        prefixEntry.mValidLifetime     = prefix->prefix_valid;
        prefixEntry.mPreferredLifetime = prefix->prefix_valid;

        // Use only 64 long pref to allow SLAAC even if we got smaller prefix. The remainig bits until 64
        // legth will be 0s.
        prefixEntry.mPrefix.mLength = 64;
        memcpy(prefixEntry.mPrefix.mPrefix.mFields.m8, prefix->prefix.addr, sizeof(otIp6Address));

        otPlatBorderRoutingProcessDhcp6PdPrefix(sInstance, &prefixEntry);
    }
}

static void otDhcpPdCb(otBorderRoutingDhcp6PdState aState, void *aContext)
{
    const struct dhcp6_delegated_prefix *prefix;

    switch (aState)
    {
    case OT_BORDER_ROUTING_DHCP6_PD_STATE_DISABLED:
    case OT_BORDER_ROUTING_DHCP6_PD_STATE_STOPPED:
        dhcp6_disable(sExtNetif);
        break;

    case OT_BORDER_ROUTING_DHCP6_PD_STATE_RUNNING:
        dhcp6_enable(sExtNetif);
        dhcp6_register_pd_callback(sExtNetif, &Dhcp6PrefixChangedCb);

        prefix = dhcp6_get_delegated_prefix(sExtNetif);
        if (prefix->prefix_valid > 0)
        {
            Dhcp6PrefixChangedCb(sExtNetif, prefix, true);
        }
        else
        {
            dhcp6_nd6_ra_trigger(sExtNetif, 0, 1);
        }
        break;

    default:
        break;
    }
}

static void HandleMulticastListenerCallback(void                                  *aContext,
                                            otBackboneRouterMulticastListenerEvent aEvent,
                                            const otIp6Address                    *aAddress)
{
    if (aEvent == OT_BACKBONE_ROUTER_MULTICAST_LISTENER_ADDED)
    {
        lwipMcastSubscribe((otIp6Address *)aAddress, (struct netif *)aContext);
    }
    else
    {
        lwipMcastUnsubscribe((otIp6Address *)aAddress, (struct netif *)aContext);
    }
}

static void HandleMdnsRegisterCallback(otInstance *aInstance, otMdnsRequestId aRequestId, otError aError)
{
    (void)aRequestId;
    if (aError == OT_ERROR_NONE)
    {
        BrMdnsHostSetInitialized(true);
        BorderAgentInit(aInstance, sHost.mHostName);
        TrelOnAppReady(sHost.mHostName);
    }
    else
    {
        // rename
        sHost.mHostName = CreateAlternativeBaseName(aInstance, sHost.mHostName);
        otMdnsRegisterHost(aInstance, &sHost, 0, HandleMdnsRegisterCallback);
    }
}

static bool UpdateIp6AddressList()
{
    const ip6_addr_t *addr6         = NULL;
    bool              bAddrChange   = false;
    uint32_t          newIp6AddrNum = 0;
    uint32_t          lwipIterator, addrListIterator;

    for (lwipIterator = 0; lwipIterator < LWIP_IPV6_NUM_ADDRESSES && newIp6AddrNum < MAX_HOST_IPV6_ADDRESSES;
         lwipIterator++)
    {
        if (ip6_addr_ispreferred(netif_ip6_addr_state(sExtNetif, lwipIterator)))
        {
            addr6 = netif_ip6_addr(sExtNetif, lwipIterator);
            for (addrListIterator = 0; addrListIterator < MAX_HOST_IPV6_ADDRESSES; addrListIterator++)
            {
                if (0 == memcmp(&sHostAddresses[addrListIterator].mFields.m32, addr6->addr,
                                sizeof(sHostAddresses[addrListIterator].mFields.m32)))
                {
                    break;
                }
            }
            if (addrListIterator == MAX_HOST_IPV6_ADDRESSES)
            {
                bAddrChange |= true;
            }
            memcpy(&sHostAddresses[newIp6AddrNum++].mFields.m32, addr6->addr, sizeof(sHostAddresses[0].mFields.m32));
        }
    }

    bAddrChange |= (newIp6AddrNum != sHost.mAddressesLength) ? true : false;
    sHost.mAddressesLength = newIp6AddrNum;

    return bAddrChange;
}
