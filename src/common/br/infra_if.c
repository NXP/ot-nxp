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

/**
 * @file
 *   This file implements the OpenThread platform abstraction for adjacent infrastructure interface.
 *
 */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include "infra_if.h"
#include "ot_platform_common.h"
#include <openthread/platform/infra_if.h>
#include <openthread/cli.h>
#include "common/code_utils.hpp"
#include "assert.h"
#include <openthread/ip6.h>
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/icmp6.h"
#include "lwip/api.h"
#include "lwip/raw.h"
#include "lwip/icmp6.h"
#include "lwip/tcpip.h"

/* -------------------------------------------------------------------------- */
/*                                 Definitions                                */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

static int sInfraIfIcmp6Socket = -1;
static uint8_t sInfraIfIndex;
static otInstance *sInstance = NULL;

struct netif *sNetifPtr;

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

static int CreateIcmp6Socket();
static uint8_t ReceiveIcmp6Message(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *addr);

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void InfraIfInit(otInstance *aInstance, struct netif *netif)
{
  struct ifreq ifr = {0};

  if(netif == NULL)
  {
    otCliOutputFormat("\r\nBorder Routing feature is disabled: infra interface is missing");
    return;
  }

  sInstance = aInstance;
  sNetifPtr = netif;

  sInfraIfIndex = netif_get_index(sNetifPtr);
  
  sInfraIfIcmp6Socket = CreateIcmp6Socket();
  if(sInfraIfIcmp6Socket != -1)
  {
    netif_index_to_name(sInfraIfIndex, (char*)&ifr.ifr_name);
    if(setsockopt(sInfraIfIcmp6Socket, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0)
    {
      otCliOutputFormat("\r\nFailed to bind icmp6 socket descriptor to the interface");
      return;
    }
  }

  LOCK_TCPIP_CORE();

  struct raw_pcb* icmp6_raw_pcb = raw_new_ip_type(IPADDR_TYPE_V6, IP6_NEXTH_ICMP6);
  raw_bind_netif(icmp6_raw_pcb, netif);

  raw_recv(icmp6_raw_pcb, ReceiveIcmp6Message, NULL);

  UNLOCK_TCPIP_CORE();

}

void InfraIfDeInit()
{
  if(sInfraIfIcmp6Socket != -1)
  {
    close(sInfraIfIcmp6Socket);
    sInfraIfIcmp6Socket = -1;
  }
}

otError otPlatInfraIfSendIcmp6Nd(uint32_t             aInfraIfIndex,
                                  const otIp6Address *aDestAddress,
                                  const uint8_t *     aBuffer,
                                  uint16_t            aBufferLength)
{
  struct sockaddr_in6 dst = {0};
  err_t sendErr;

  dst.sin6_family = AF_INET6;
  memcpy(&dst.sin6_addr.un.u32_addr, aDestAddress->mFields.m32, sizeof(dst.sin6_addr.un.u32_addr));

  dst.sin6_scope_id = sInfraIfIndex;

  sendErr = sendto(sInfraIfIcmp6Socket, aBuffer, aBufferLength, 0, (struct sockaddr*)&dst, sizeof(dst));

  return (sendErr != -1 ? OT_ERROR_NONE : OT_ERROR_FAILED);
}

bool otPlatInfraIfHasAddress(uint32_t aInfraIfIndex, const otIp6Address *aAddress)
{
    ip_addr_t searchedAddress = IPADDR6_INIT(0,0,0,0);
    memcpy(ip_2_ip6(&searchedAddress), &aAddress->mFields.m32, sizeof(aAddress->mFields.m32));

    return (netif_get_ip6_addr_match(sNetifPtr, (const ip6_addr_t*)&searchedAddress) > 0 ? true : false);
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

static int CreateIcmp6Socket()
{
  static struct sockaddr_in6 src = {0};
  int sockdesc = -1;

  if((sockdesc = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)) < 0)
  {
    otCliOutputFormat("\r\nFailed to get socket descriptor");
    return -1;
  }

  src.sin6_family = AF_INET6;
  
  inet_pton(AF_INET6, ip6addr_ntoa(netif_ip6_addr(sNetifPtr, 0)), &src.sin6_addr);

  if(bind(sockdesc, (struct sockaddr*)&src, sizeof(src)) != 0)
  {
    otCliOutputFormat("\r\nFailed to bind icmp6 socket descriptor to the source address");
    return -1;
  }
  return sockdesc;
}

static uint8_t ReceiveIcmp6Message(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *addr)
{
  (void)arg;
  (void)pcb;

  size_t icmpv6_type_pos = 40;
  otIp6Address  aPeerAddr;

  uint8_t icmpv6_type = *(uint8_t*)(p->payload + icmpv6_type_pos);

  switch(icmpv6_type)
  {
    case ICMP6_TYPE_RS: /* Router solicitation */
    case ICMP6_TYPE_RA: /* Router advertisement */
    case ICMP6_TYPE_NA: /* Neighbor advertisement */

      memcpy(aPeerAddr.mFields.m8, ip_2_ip6(addr), sizeof(ip6_addr_t));
      otPlatInfraIfRecvIcmp6Nd(sInstance, sInfraIfIndex, &aPeerAddr, (const uint8_t *)((uint8_t*)p->payload + icmpv6_type_pos),
                               p->len);
    break;

    default:
    break;
  }

  return 0; // packet eaten = 0 => packet was not consumed by application
}
