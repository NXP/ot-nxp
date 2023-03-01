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
 *   This file implements the OpenThread Border Router application.
 *
 */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include <assert.h>

#include "lwip/tcpip.h"
#include "lwip/nd6.h"
#include "lwip/mld6.h"
#include "lwip/prot/dns.h"
#include "lwip/prot/iana.h"
#include "lwip/udp.h"

#include "pin_mux.h"
#include "board.h"

#include "infra_if.h"

#ifdef OT_APP_BR_ETH_EN
#include "ethernetif.h"
#include "fsl_phy.h"
#include "fsl_phyksz8081.h"
#include "fsl_iomuxc.h"
#include "fsl_enet.h"
#include "fsl_silicon_id.h"
#endif

#include <openthread/srp_server.h>
#include <openthread/border_router.h>
#include <openthread/border_routing.h>
#include <openthread/thread.h>
#include <openthread/cli.h>
#include <openthread/udp.h>
#include <openthread/tasklet.h>
#include "openthread-system.h"
#include <openthread-core-config.h>

#include "wpl.h"
#include "common/code_utils.hpp"
#include "ot_lwip.h"

/* -------------------------------------------------------------------------- */
/*                                 Definitions                                */
/* -------------------------------------------------------------------------- */

#ifndef OT_MAIN_TASK_PRIORITY
#define OT_MAIN_TASK_PRIORITY 1
#endif

#ifndef OT_MAIN_TASK_SIZE
#define OT_MAIN_TASK_SIZE ((configSTACK_DEPTH_TYPE)8192 / sizeof(portSTACK_TYPE))
#endif

#ifndef OT_WIFI_CFG_TASK_PRIORITY
#define OT_WIFI_CFG_TASK_PRIORITY 3
#endif

#ifndef OT_WIFI_CFG_TASK_SIZE
#define OT_WIFI_CFG_TASK_SIZE ((configSTACK_DEPTH_TYPE)(4*1024) / sizeof(portSTACK_TYPE))
#endif

#if configAPPLICATION_ALLOCATED_HEAP
uint8_t __attribute__((section(".heap"))) ucHeap[configTOTAL_HEAP_SIZE];
#endif

#ifndef USE_OT_MDNS
#define USE_OT_MDNS 1
#endif

/* Ethernet configuration. */
#define EXAMPLE_PHY_ADDRESS  BOARD_ENET0_PHY_ADDRESS
#define EXAMPLE_PHY_OPS      &phyksz8081_ops
#define EXAMPLE_PHY_RESOURCE &g_phy_resource
#define EXAMPLE_CLOCK_FREQ   CLOCK_GetFreq(kCLOCK_IpgClk)

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */
static TaskHandle_t sMainTask = NULL;

#ifdef OT_APP_BR_ETH_EN
static phy_handle_t phyHandle;
static phy_ksz8081_resource_t g_phy_resource;
static struct netif sExtNetif;
#endif

static struct netif *sExtNetifPtr;

/* IPv6 multicast group FF02::FB */
static const ip_addr_t mdnsV6group = DNS_MQUERY_IPV6_GROUP_INIT;
static struct udp_pcb *mdns_pcb;

static otInstance * sInstance = NULL;

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

#ifdef OT_APP_BR_ETH_EN
static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data);
static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData);
static void appConfigEnetHw();
static void appConfigEnetIf();
#endif

#ifdef OT_APP_BR_WIFI_EN
static void wifiLinkCB(bool up);
static void appConfigWifiIf();
static void appConfigWifiHw();
#endif

static void appMdnsRcvHook(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
static void appMdnsTxHook(otMessage *aMessage, uint16_t aPeerPort, otIp6Address *aPeerAddr, uint16_t aSockPort, void * aContext);
static void appMdnsProxyInit();
static void appStartBrService();
static void appOtInit();
static void appBrInit();
static void appStartBrService();
static void add_static_ipv6(struct netif *nif, uint8_t addr_idx, const char *addr_str);
static void mainloop(void *aContext);

/* -------------------------------------------------------------------------- */
/*                              Public functions prototypes                             */
/* -------------------------------------------------------------------------- */

extern void otAppCliInit(otInstance *aInstance);
extern void otSysRunIdleTask(void);


/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

static void add_static_ipv6(struct netif *nif, uint8_t addr_idx, const char *addr_str)
{
    ip6_addr_t addr;
    ip6addr_aton(addr_str, &addr);

    LOCK_TCPIP_CORE();
    netif_ip6_addr_set(nif, addr_idx, &addr);
    netif_ip6_addr_set_valid_life(nif, addr_idx, IP6_ADDR_LIFE_STATIC);
    netif_ip6_addr_set_pref_life(nif, addr_idx, IP6_ADDR_LIFE_STATIC);
    netif_ip6_addr_set_state(nif, addr_idx, IP6_ADDR_VALID);
    UNLOCK_TCPIP_CORE();
}

#ifdef OT_APP_BR_ETH_EN
static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    const status_t s = ENET_MDIOWrite(ENET, phyAddr, regAddr, data);
    return s;
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    const status_t s = ENET_MDIORead(ENET, phyAddr, regAddr, pData);
    return s;
}

static void appConfigEnetHw()
{
    /* Enet pins */
    BOARD_InitEnetPins();
    
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    
    /* Enet clock */
    const clock_enet_pll_config_t config = {.enableClkOutput = true, .enableClkOutput25M = false, .loopDivider = 1};
    CLOCK_InitEnetPll(&config);
    
    IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true);

    GPIO_PinInit(GPIO1, 9, &gpio_config);
    GPIO_PinInit(GPIO1, 10, &gpio_config);
    /* Pull up the ENET_INT before RESET. */
    GPIO_WritePinOutput(GPIO1, 10, 1);
    GPIO_WritePinOutput(GPIO1, 9, 0);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO1, 9, 1);
    
    /* MDIO Init */
    (void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(ENET)]);
    ENET_SetSMI(ENET, EXAMPLE_CLOCK_FREQ, false);
    
    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;
}

static void appConfigEnetIf()
{
    ethernetif_config_t enet_config = {.phyHandle   = &phyHandle,
                                       .phyAddr     = EXAMPLE_PHY_ADDRESS,
                                       .phyOps      = EXAMPLE_PHY_OPS,
                                       .phyResource = EXAMPLE_PHY_RESOURCE,
                                       .srcClockHz  = EXAMPLE_CLOCK_FREQ,
    };

    sExtNetifPtr = &sExtNetif;

    /* Set MAC address. */
    SILICONID_ConvertToMacAddr(&enet_config.macAddress);

    netifapi_netif_add(sExtNetifPtr, NULL, NULL, NULL, &enet_config, ethernetif0_init, tcpip_input);
    netifapi_netif_set_up(sExtNetifPtr);

    LOCK_TCPIP_CORE();
    netif_create_ip6_linklocal_address(sExtNetifPtr, 1);
    UNLOCK_TCPIP_CORE();
}
#endif

#ifdef OT_APP_BR_WIFI_EN
static void wifiLinkCB(bool up)
{
    otCliOutputFormat("Wi-fi link is now %s\r\n", up ? "up" : "down");
}

static void appConfigwifiIfTask(void * args)
{
    OT_UNUSED_VARIABLE(args);
    wpl_ret_t ret;

    ret = WPL_Join("my_net");
    if(ret != WPLRET_SUCCESS)
    {
        otCliOutputFormat("WPL_Join() to '%s' / '%s' failed with code %d\r\n",
                          WIFI_SSID, WIFI_PASSWORD, ret);
        VerifyOrExit(ret != WPLRET_SUCCESS);
    }

#if INCLUDE_uxTaskGetStackHighWaterMark == 1
    otCliOutputFormat("\r\n\t%s's stack watter mark: %dw\r\n", pcTaskGetName(NULL), uxTaskGetStackHighWaterMark(NULL));
#endif

    appStartBrService();

    vTaskSuspend(NULL);

exit:
#if INCLUDE_uxTaskGetStackHighWaterMark == 1
    otCliOutputFormat("\r\n\t%s's stack watter mark: %dw\r\n", pcTaskGetName(NULL), uxTaskGetStackHighWaterMark(NULL));
#endif
    return;
}

void tcpip_init_wifi()
{
    //dummy function part of the wifi port hack...
}

static void appConfigWifiIf()
{
    wpl_ret_t ret;

    ret = WPL_Init();
    if(ret != WPLRET_SUCCESS)
    {
        otCliOutputFormat("WPL_Init() failed with code %d\r\n", ret);
        VerifyOrExit(ret != WPLRET_SUCCESS);
    }

    ret = WPL_Start(&wifiLinkCB);
    if(ret != WPLRET_SUCCESS)
    {
        otCliOutputFormat("WPL_Start() failed with code %d\r\n", ret);
        VerifyOrExit(ret != WPLRET_SUCCESS);
    }

    ret = WPL_AddNetwork(WIFI_SSID, WIFI_PASSWORD, "my_net");
    if(ret != WPLRET_SUCCESS)
    {
        otCliOutputFormat("WPL_AddNetwork() failed with code %d\r\n", ret);
        VerifyOrExit(ret != WPLRET_SUCCESS);
    }

    if(
        xTaskCreate(&appConfigwifiIfTask,
                    "wifi-cfg", OT_WIFI_CFG_TASK_SIZE,
                    NULL, OT_WIFI_CFG_TASK_PRIORITY,
                    NULL) != pdPASS
        )
    {
        otCliOutputFormat("appConfigwifiIfTask creation failed with code %d\r\n", ret);
    }

    sExtNetifPtr =  netif_get_by_index(netif_name_to_index("ml1"));

exit:
    return;
}

static void appConfigWifiHw()
{
    #if !defined(RW610)
    /* Configure SDHC slot pins used for Wi-Fi */
    BOARD_InitUSDHCPins();
    BOARD_InitMurataModulePins();
    #endif
}
#endif

static void appMdnsRcvHook(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    /* Inject the packet in the OT queue */

    const otMessageSettings msgSettings = { false, OT_MESSAGE_PRIORITY_NORMAL };
    otMessage * otUdpPkt = otIp6NewMessageFromBuffer(sInstance, p->payload, p->len, &msgSettings);

    otIp6Address  aPeerAddr;
    memcpy(aPeerAddr.mFields.m8, ip_2_ip6(addr), sizeof(ip6_addr_t));

    if (otUdpPkt != NULL)
    {
        // Pass the packet to OpenThread to be sent.  Note that OpenThread takes care of releasing
        // the otMessage object regardless of whether otIp6Send() succeeds or fails.
        otUdpForwardReceive(sInstance, otUdpPkt, port, &aPeerAddr, LWIP_IANA_PORT_MDNS);
    }

    pbuf_free(p);
}

static void appMdnsTxHook(otMessage *aMessage, uint16_t aPeerPort, otIp6Address *aPeerAddr, uint16_t aSockPort, void * aContext)
{
    ip_addr_t lwipAddr = IPADDR6_INIT(0,0,0,0);
    memcpy(ip_2_ip6(&lwipAddr), aPeerAddr->mFields.m8, sizeof(ip6_addr_t));

    //The LWIP address needs to be intilialized correctly with a zone
    if (ip_addr_ismulticast(&lwipAddr))
    {
        ip6_addr_assign_zone(ip_2_ip6(&lwipAddr), IP6_MULTICAST, sExtNetifPtr);
    }
    else
    {
        ip6_addr_assign_zone(ip_2_ip6(&lwipAddr), IP6_UNICAST, sExtNetifPtr);
    }

    struct pbuf *lwipIpPkt = otPlatLwipConvertToLwipMsg(aMessage, true);
    if (lwipIpPkt)
    {
        udp_sendto_if(mdns_pcb, lwipIpPkt, &lwipAddr, aPeerPort, sExtNetifPtr);
        pbuf_free(lwipIpPkt);
    }
    otMessageFree(aMessage);
}

static void appMdnsProxyInit()
{
    mld6_joingroup_netif(sExtNetifPtr, ip_2_ip6(&mdnsV6group));

//    LWIP_MEMPOOL_INIT(MDNS_PKTS);
    mdns_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);

    udp_bind(mdns_pcb, IP_ANY_TYPE, LWIP_IANA_PORT_MDNS);
    udp_recv(mdns_pcb, appMdnsRcvHook, NULL);

    otUdpForwardSetForwarder(sInstance, appMdnsTxHook, NULL);
}

static void appStartBrService()
{
    const uint8_t staticIpidx = 1;
    otIp6Prefix onLinkPrefix;
    ip_addr_t lwipAddr = IPADDR6_INIT(0,0,0,0);

    otBorderRoutingInit(sInstance, netif_get_index(sExtNetifPtr), true);

    otBorderRoutingSetEnabled(sInstance, true);

    otSrpServerSetEnabled(sInstance, true);

    InfraIfInit(sInstance, sExtNetifPtr);     

   while (OT_ERROR_INVALID_STATE == otBorderRoutingGetOnLinkPrefix(sInstance, &onLinkPrefix))
   {
    ;
   }

    memcpy(ip_2_ip6(&lwipAddr), &onLinkPrefix.mPrefix.mFields.m8, sizeof(onLinkPrefix.mPrefix));
    add_static_ipv6(sExtNetifPtr, staticIpidx, ip6addr_ntoa((const ip6_addr_t*)&lwipAddr));
    
    /* Subscribe to mdns-sd multicast address FF02::FB */
    appMdnsProxyInit();
}

static void appOtInit()
{
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    size_t   otInstanceBufferLength = 0;
    uint8_t *otInstanceBuffer       = NULL;
#endif

    otSysInit(0, NULL);

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    // Call to query the buffer size
    (void)otInstanceInit(NULL, &otInstanceBufferLength);

    // Call to allocate the buffer
    otInstanceBuffer = (uint8_t *)pvPortMalloc(otInstanceBufferLength);
    assert(otInstanceBuffer);

    // Initialize OpenThread with the buffer
    sInstance = otInstanceInit(otInstanceBuffer, &otInstanceBufferLength);
#else
    sInstance = otInstanceInitSingle();
#endif

#if OPENTHREAD_ENABLE_DIAG
    otDiagInit(sInstance);
#endif
    /* Init the CLI */
    otAppCliInit(sInstance);
}

static void appBrInit()
{
#ifdef OT_APP_BR_ETH_EN
    appConfigEnetHw();
#endif

#ifdef OT_APP_BR_WIFI_EN
    appConfigWifiHw();
#endif

    otPlatLwipInit(sInstance);

#ifdef OT_APP_BR_WIFI_EN
    appConfigWifiIf();
#endif

#ifdef OT_APP_BR_ETH_EN
    appConfigEnetIf();
#endif

    otPlatLwipAddThreadInterface();

    otSetStateChangedCallback(sInstance, otPlatLwipUpdateState, NULL);

#ifdef OT_APP_BR_ETH_EN
    appStartBrService();
#endif
}

static void mainloop(void *aContext)
{
    OT_UNUSED_VARIABLE(aContext);

    appOtInit();
    appBrInit();

    otSysProcessDrivers(sInstance);
    while (!otSysPseudoResetWasRequested())
    {
        otTaskletsProcess(sInstance);
        if (!otTaskletsArePending(sInstance))
        {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        }
        otSysProcessDrivers(sInstance);
    }

    otInstanceFinalize(sInstance);
    vTaskDelete(NULL);
}

void appOtStart(int argc, char *argv[])
{
    xTaskCreate(mainloop, "ot", OT_MAIN_TASK_SIZE, NULL, OT_MAIN_TASK_PRIORITY, &sMainTask);
    vTaskStartScheduler();
}

void otTaskletsSignalPending(otInstance *aInstance)
{
    (void)aInstance;
    xTaskNotifyGive(sMainTask);
}

void otSysEventSignalPending(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(sMainTask, &xHigherPriorityTaskWoken);
    /* Context switch needed? */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

#if (defined(configUSE_IDLE_HOOK) && (configUSE_IDLE_HOOK > 0))
void vApplicationIdleHook(void)
{
    otSysRunIdleTask();
}
#endif

#if (defined(configCHECK_FOR_STACK_OVERFLOW) && (configCHECK_FOR_STACK_OVERFLOW > 0))
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    assert(0);
}
#endif

#if (defined(configUSE_MALLOC_FAILED_HOOK) && (configUSE_MALLOC_FAILED_HOOK > 0))
void vApplicationMallocFailedHook(void)
{
    assert(0);
}
#endif

#if OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE
void *otPlatCAlloc(size_t aNum, size_t aSize)
{
    size_t total_size = aNum * aSize;
    void * ptr        = pvPortMalloc(total_size);
    if (ptr)
    {
        memset(ptr, 0, total_size);
    }

    return ptr;
}

void otPlatFree(void *aPtr)
{
    vPortFree(aPtr);
}
#endif

#if OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_APP
void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
    va_list ap;

    va_start(ap, aFormat);
    otCliPlatLogv(aLogLevel, aLogRegion, aFormat, ap);
    va_end(ap);
}
#endif
