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

#ifndef __BR_RTOS_MANAGER_H__
#define __BR_RTOS_MANAGER_H__

#include "fsl_component_generic_list.h"
#include "fsl_os_abstraction.h"
#include "ot_lwip.h"
#include "stdarg.h"
#include <openthread/backbone_router_ftd.h>
#include <openthread/border_routing.h>
#include <openthread/ip6.h>
#include <openthread/nat64.h>
#include <openthread/udp.h>
#include <openthread/platform/mdns_socket.h>
#include "lwip/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

extern otPlatLockTaskCb   gLockTaskCb;
extern otPlatUnlockTaskCb gUnlockTaskCb;

/*
This macro is intended to be used whenever OT app calls lwip API.
A result might be needed from lwip, so this macro will let synchronous execution.
There is no need to verify that sMainStackLock exists, as it's the first thing executed
in appOtStart. Failure to create this mutex will lead to assert.
*/
#define CALL_LWIP_API_FROM_OT_CONTEXT(...) \
    do                                     \
    {                                      \
        if (gLockTaskCb)                   \
            /* release OT mutex */         \
            gUnlockTaskCb();               \
        LOCK_TCPIP_CORE();                 \
        if (gLockTaskCb)                   \
            /* acquire OT mutex */         \
            gLockTaskCb();                 \
        __VA_ARGS__;                       \
        UNLOCK_TCPIP_CORE();               \
    } while (0)

/*
This macro is intended to be used whenever OT posts a callback to lwip.
The callback will be asynchronous executed in lwip's thread context. No result in waited by caller task.
There is no need to verify that sMainStackLock exists, as it's the first thing executed
in appOtStart. Failure to create this mutex will lead to assert.
*/
#define POST_LWIP_CALLBACK_FROM_OT_CONTEXT(...) \
    do                                          \
    {                                           \
        if (gLockTaskCb)                        \
            /* release OT mutex */              \
            gUnlockTaskCb();                    \
        __VA_ARGS__;                            \
        if (gLockTaskCb)                        \
            /* acquire OT mutex */              \
            gLockTaskCb();                      \
    } while (0)

typedef struct brMsgContext_tag brMsgContext;
typedef void (*brMsgCallback)(brMsgContext *aMsgContextPtr);
typedef struct brPbuffAndLen_tag
{
    uint8_t *buffer;
    uint16_t bufferLen;
} brPbuffAndLen;
struct brMsgContext_tag
{
    list_element_t link;
    brMsgCallback  brMsgCallback;
    otUdpSocket   *socket;
    union
    {
        brPbuffAndLen buffAndLen;
        struct pbuf  *pbuf;
    };
    union
    {
        otMessageInfo         messageInfo;
        otPlatMdnsAddressInfo addrInfo;
        otIp6Address          ipAddress;
    };
};

typedef enum otEventType_tag
{
    eLinkChangedEvent = 0,
    eAddrSetOrChanged,
    eDhcp6PrefixChanged,
    eBrInitPlatform
} otEventType;

typedef struct otEvent brEvtContext;
struct otEvent
{
    list_element_t link;
    otEventType    type;
    union
    {
        struct
        {
            uint8_t netif_idx;
            bool    netif_state;
        } link_changed_event;

        struct
        {
            bool isIp6;
#if OPENTHREAD_CONFIG_NAT64_TRANSLATOR_ENABLE || OPENTHREAD_CONFIG_NAT64_BORDER_ROUTING_ENABLE
            otIp4Cidr cidr;
            bool      nat64TranslatorState;
#endif
        } addr_set_or_changed_event;

        struct
        {
            otBorderRoutingPrefixTableEntry prefixEntry;
        } dhcp6_prefix_changed_event;
    };
};

/* Must be called before BrInitServices and BrUpdateLwipThrIf*/
void BrInitPlatform(otInstance *aInstance, struct netif *aExtNetif, struct netif *aThreadNetif);
/* Must be called after BrInitPlatform */
void BrInitServices();
/* Must be called after BrInitPlatform, used for Matter to switch using OTBR implementation of LWIP Thread IP interface
   TX/RX functions. This allows using OTBR features in Matter like NAT64 and rate limiting */
void BrUpdateLwipThrIf();

/* According to Thread spec a Border Router MUST provide a mechanism to manually disable NAT64 translation in cases
   where the user does not desire NAT64 translation. */
void BrSetNat64TranslatorState(bool aEnable);

void BrInitAppLock(otPlatLockTaskCb aLockTaskCb, otPlatUnlockTaskCb aUnlockTaskCb);
void BrInitMdnsHost(const char *aHostName);
void BrMdnsHostSetInitialized(bool aState);
bool BrMdnsHostIsInitialized();

void BrPostOtMessage(brMsgContext *aContextMsgPtr);
void BrPostOtEvent(brEvtContext *aContextEvtPtr);

void BrNetifExtCb(struct netif *netif, netif_nsc_reason_t reason, const netif_ext_callback_args_t *args);

#ifdef __cplusplus
}
#endif
#endif /* __BR_RTOS_MANAGER_H__ */
