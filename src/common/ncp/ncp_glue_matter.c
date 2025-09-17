/* @file ncp_glue_ot.c
 *
 *  @brief This file contains ot ncp command functions.
 *
 *  Copyright 2025 NXP
 *
 *  Licensed under the LA_OPT_NXP_Software_License.txt (the "Agreement")
 */
#include "ncp_glue_matter.h"
#include "ncp_cmd_ot.h"
#include "ncp_glue_ot.h"
#include "ncp_ot.h"
#include "ot_platform_common.h"

#include "openthread-system.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <openthread/border_agent.h>
#include <openthread/dataset.h>
#include <openthread/dns_client.h>
#include <openthread/icmp6.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/netdata.h>
#include <openthread/srp_client.h>
#include <openthread/srp_client_buffers.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include <openthread/thread_ftd.h>
#include <openthread/udp.h>

// data structure
struct ptr_to_eventid_mapping
{
    /** ID assigned to event*/
    int eventid;
    /** The function that should be invoked for this eventid. */
    int (*ptr_val)();
};

struct address_mapping_32_to_64
{
    uint64_t addr64; // address from HOST
    uint32_t addr32; // address used at ncp device
};

extern otInstance *gInstance;

static uint8_t                         ncp_cmd_buf[4096]                                   = {0};
static int                             ncp_cmd_size                                        = 0;
static uint8_t                         ot_ncp_tx_buf[4096]                                 = {0}; // for callbacks
static int                             registered_ptr_eventids                             = 0;
static struct ptr_to_eventid_mapping   ptr_eventid_array[NCP_CALLBACK_TO_EVENTID_ARRAY_SZ] = {0};
static struct address_mapping_32_to_64 addr_mapping_array[MAX_32_TO_64_ARRAY_LEN]          = {0};
static int                             total_32_to_64_values                               = 0;
otUdpSocket                            device_mSocket; // assuming one udp socket

static void processApi(int opCode, uint8_t *payloadIdx);
static void process_otIp6SetEnabled(int opCode, uint8_t *payloadIdx);
static void process_SetThreadEnabled(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetDeviceRole(int opCode, uint8_t *payloadIdx);
static void process_otDatasetSetActiveTlvs(int opCode, uint8_t *payloadIdx);
static void process_otIp6IsEnabled(int opCode, uint8_t *payloadIdx);
static void process_otIp6GetUnicastAddresses(int opCode, uint8_t *payloadIdx);
static void process_otDatasetGetActiveTlvs(int opCode, uint8_t *payloadIdx);
static void process_otDatasetIsCommissioned(int opCode, uint8_t *payloadIdx);
static void process_otDatasetGetActive(int opCode, uint8_t *payloadIdx);
static void process_otDatasetGetPendingTlvs(int opCode, uint8_t *payloadIdx);
static void process_otDatasetSetPendingTlvs(int opCode, uint8_t *payloadIdx);
static void process_otInstanceErasePersistentInfo(int opCode, uint8_t *payloadIdx);
static void process_otThreadIsRouterEligible(int opCode, uint8_t *payloadIdx);
static void process_otLinkGetCslPeriod(int opCode, uint8_t *payloadIdx);
static void process_otThreadSetRouterEligible(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetRloc16(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetLeaderRouterId(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetPartitionId(int opCode, uint8_t *payloadIdx);
static void process_otPlatRadioGetRssi(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetLeaderWeight(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetLocalLeaderWeight(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetVersion(int opCode, uint8_t *payloadIdx);
static void process_otLinkGetPollPeriod(int opCode, uint8_t *payloadIdx);
static void process_otLinkSetCslPeriod(int opCode, uint8_t *payloadIdx);
static void process_otLinkSetPollPeriod(int opCode, uint8_t *payloadIdx);
static void process_otLinkGetPanId(int opCode, uint8_t *payloadIdx);
static void process_otNetDataGetStableVersion(int opCode, uint8_t *payloadIdx);
static void process_otSrpClientSetLeaseInterval(int opCode, uint8_t *payloadIdx);
static void process_otSrpClientSetKeyLeaseInterval(int opCode, uint8_t *payloadIdx);
static void process_otSrpClientRemoveHostAndServices(int opCode, uint8_t *payloadIdx);
static void process_otSrpClientEnableAutoHostAddress(int opCode, uint8_t *payloadIdx);
static void process_otThreadSetLinkMode(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetLinkMode(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetParentAverageRssi(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetParentLastRssi(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetNetworkKey(int opCode, uint8_t *payloadIdx);
static void process_otThreadErrorToString(int opCode, uint8_t *payloadIdx);
static void process_otBorderAgentGetId(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetNetworkName(int opCode, uint8_t *payloadIdx);
static void process_otLinkGetExtendedAddress(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetExtendedPanId(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetMeshLocalPrefix(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetLeaderRloc(int opCode, uint8_t *payloadIdx);
static void process_otNetDataGet(int opCode, uint8_t *payloadIdx);
static void process_otIp6SubscribeMulticastAddress(int opCode, uint8_t *payloadIdx);
static void process_otIp6UnsubscribeMulticastAddress(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetNextNeighborInfo(int opCode, uint8_t *payloadIdx);
static void process_otNetDataGetNextRoute(int opCode, uint8_t *payloadIdx);
static void process_otLinkGetCounters(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetIp6Counters(int opCode, uint8_t *payloadIdx);
static void process_otSetStateChangedCallback(int opCode, uint8_t *payloadIdx);
static void process_otIp6AddressFromString(int opCode, uint8_t *payloadIdx);
static void process_otIp6AddressToString(int opCode, uint8_t *payloadIdx);
static void process_otNetDataGetVersion(int opCode, uint8_t *payloadIdx);
static void process_otThreadGetChildInfoById(int opCode, uint8_t *payloadIdx);
static void process_otIp6GetMulticastAddresses(int opCode, uint8_t *payloadIdx);
static void process_otThreadDiscover(int opCode, uint8_t *payloadIdx);
static void process_otUdpOpen(int opCode, uint8_t *payloadIdx);
static void process_otUdpBind(int opCode, uint8_t *payloadIdx);
static void process_otudpClose(int opCode, uint8_t *payloadIdx);
static void process_otUdpIsOpen(int opCode, uint8_t *payloadIdx);
static void process_otUdpSend(int opCode, uint8_t *payloadIdx);
static void process_otUdpNewMessage(int opCode, uint8_t *payloadIdx);
static void process_otMessageFree(int opCode, uint8_t *payloadIdx);
static void process_otMessageAppend(int opCode, uint8_t *payloadIdx);
static void process_otSrpClientSetHostName(int opCode, uint8_t *payloadIdx);
static void process_otSrpClientAddService(int opCode, uint8_t *payloadIdx);
static void process_otSrpClientRemoveService(int opCode, uint8_t *payloadIdx);
static void process_otSrpClientClearService(int opCode, uint8_t *payloadIdx);
static void process_otSrpClientEnableAutoStartMode(int opCode, uint8_t *payloadIdx);
static void process_otSrpClientSetCallback(int opCode, uint8_t *payloadIdx);
static void process_otDnsBrowseResponseGetServiceName(int opCode, uint8_t *payloadIdx);
static void process_otDnsClientBrowse(int opCode, uint8_t *payloadIdx);
static void process_otDnsClientGetDefaultConfig(int opCode, uint8_t *payloadIdx);
static void process_otDnsClientSetDefaultConfig(int opCode, uint8_t *payloadIdx);
static void process_otDnsInitTxtEntryIterator(int opCode, uint8_t *payloadIdx);
static void process_otDnsGetNextTxtEntry(int opCode, uint8_t *payloadIdx);
static void process_otDnsClientResolveService(int opCode, uint8_t *payloadIdx);
static void process_otIp6IsAddressUnspecified(int opCode, uint8_t *payloadIdx);
static void process_otDnsClientResolveAddress(int opCode, uint8_t *payloadIdx);
static void process_otIcmp6SetEchoMode(int opCode, uint8_t *payloadIdx);
static void process_otIp6SetReceiveFilterEnabled(int opCode, uint8_t *payloadIdx);
static void process_otIp6SetSlaacEnabled(int opCode, uint8_t *payloadIdx);
static void process_otIp6SetReceiveCallback(int opCode, uint8_t *payloadIdx);
static void process_otIp6NewMessage(int opCode, uint8_t *payloadIdx);
static void process_otIp6Send(int opCode, uint8_t *payloadIdx);
static void process_otLinkGetChannel(int opCode, uint8_t *payloadIdx);

// callback functions
static void processStateChange(otChangedFlags aFlags, void *aContext);
static void HandleActiveScanResult(otActiveScanResult *aResult, void *aContext);
static void HandleUdpReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);
static void ncp_OnSrpClientStateChange(const otSockAddr *aServerSockAddr, void *aContext);
static void ncp_SrpClientCallback(otError                    aError,
                                  const otSrpClientHostInfo *aHostInfo,
                                  const otSrpClientService  *aServices,
                                  const otSrpClientService  *aRemovedServices,
                                  void                      *aContext);
static void ncp_OnDnsBrowseResult(otError aError, const otDnsBrowseResponse *aResponse, void *aContext);
static void ncp_otDnsService_cb(otError aError, const otDnsServiceResponse *aResponse, void *aContext);
static void ncp_otDnsAddress_cb(otError aError, const otDnsAddressResponse *aResponse, void *aContext);
static void ncp_otIp6Receive_cb(otMessage *aMessage, void *aContext);

static void     ncp_memcpy(uint8_t *dest, uint8_t *src, int bytes_to_copy, int *totalsize);
static void     ncp_val_mem_copy(uint8_t *dest, uint8_t src, int *totalsize);
static bool     register_ptr_eventid(void *ptr_val, int eventid);
static void     map_32_to_64_addr(void *addr32bit, uint64_t addr64bit);
static uint64_t get_64_mapped_addr(void *addr32bit);
static void     remove_64_mapped_addr(void *addr32bit);
static void     remove_ptr_eventid(int eventid);
static void    *get_ptr_from_eventid(int eventid);

bool ncp_ot_fct_process(void)
{
    uint32_t           ret = 0;
    otmatter_payload_t payload_item;
    uint8_t           *payload_buf = NULL;
    uint32_t           payload_sz  = 0;

    ret = xQueueReceive(sOtmatterNcpCmdQueue, &payload_item, (TickType_t)0);
    if (ret == pdPASS)
    {
        payload_buf = payload_item.payload_buff;

        int      opCode     = *(((int *)payload_buf) + 0);                     // NCP API OPCODE
        uint8_t *payloadIdx = (((uint8_t *)payload_buf) + NCP_API_OPCODE_LEN); // GET Payloadindex

        processApi(opCode, payloadIdx);

        vPortFree(payload_buf);
        payload_buf = NULL;

        return true;
    }
    else
    {
        return false;
    }
}

int ncp_matter_ot_cmd_handle(void *cmd, int payloadsize)
{
    uint32_t           ret = 0;
    otmatter_payload_t payload_item;

    payload_item.payload_sz   = payloadsize;
    payload_item.payload_buff = (ncp_tlv_qelem_t *)pvPortMalloc(payloadsize);

    if (!payload_item.payload_buff)
    {
        OT_PLAT_ERR("failed to allocate memory for ncp matter ot queue element.\r\n");

        return NCP_STATUS_ERROR;
    }

    memcpy(payload_item.payload_buff, cmd, payloadsize);

    ret = xQueueSend(sOtmatterNcpCmdQueue, &payload_item, (TickType_t)0);
    if (ret != pdPASS)
    {
        OT_PLAT_ERR("send to ncp matter ot queue failed.\r\n");
        vPortFree(payload_item.payload_buff); // Free memory on failure

        return NCP_STATUS_ERROR;
    }

    otTaskletsSignalPending(gInstance);

    return 0;
}

static void ncp_memcpy(uint8_t *dest, uint8_t *src, int bytes_to_copy, int *totalsize)
{
    memcpy((uint8_t *)dest, (uint8_t *)src,
           bytes_to_copy); // casting to uint8_t just be sure func if the receiving pointer is of diff type
    *totalsize += bytes_to_copy;

    return;
}

static void ncp_val_mem_copy(uint8_t *dest, uint8_t src, int *totalsize)
{
    *(uint8_t *)dest = (uint8_t)src;
    *totalsize += 1;

    return;
}

/*This function will save the address of cb function gainst provided eventid*/
static bool register_ptr_eventid(void *ptr_val, int eventid)
{
    // check if eventid is not already registered
    bool eventid_registered = 0;

    int rot_eventid;
    for (rot_eventid = 0; rot_eventid < registered_ptr_eventids; rot_eventid++)
    {
        if (ptr_eventid_array[rot_eventid].eventid == eventid)
        {
            eventid_registered = 1;
            break;
        }
    }

    if (eventid_registered == 0)
    {
        assert(registered_ptr_eventids < NCP_CALLBACK_TO_EVENTID_ARRAY_SZ);
        ptr_eventid_array[registered_ptr_eventids].eventid = eventid;
        ptr_eventid_array[registered_ptr_eventids].ptr_val = ptr_val;
        registered_ptr_eventids++;
    }

    return eventid_registered;
}

static void map_32_to_64_addr(void *addr32bit, uint64_t addr64bit)
{
    // check if addr is not already registered
    bool addr_registered = 0;
    int  rot_addr;
    for (int rot_addr = 0; rot_addr < total_32_to_64_values; rot_addr++)
    {
        if (addr_mapping_array[rot_addr].addr32 == (uint32_t)addr32bit &&
            addr_mapping_array[rot_addr].addr64 == addr64bit)
        {
            addr_registered = 1;
            break;
        }
    }

    if (addr_registered == 0)
    {
        assert(total_32_to_64_values < MAX_32_TO_64_ARRAY_LEN);
        addr_mapping_array[total_32_to_64_values].addr64 = addr64bit;
        addr_mapping_array[total_32_to_64_values].addr32 = (uint32_t)addr32bit;
        total_32_to_64_values++;
    }
}

static uint64_t get_64_mapped_addr(void *addr32bit)
{
    uint64_t ret_64_mapped_addr = 0;
    for (int rot_addr_arr = 0; rot_addr_arr < total_32_to_64_values; rot_addr_arr++)
    {
        if (addr_mapping_array[rot_addr_arr].addr32 == (uint32_t)addr32bit)
        {
            ret_64_mapped_addr = addr_mapping_array[rot_addr_arr].addr64;
            break;
        }
    }

    return ret_64_mapped_addr;
}

void remove_64_mapped_addr(void *addr32bit)
{
    uint64_t temp_save_64_mapped_addr = 0;
    uint32_t temp_save_32_mapped_addr = 0;
    for (int rot_addr_arr = 0; rot_addr_arr < total_32_to_64_values; rot_addr_arr++)
    {
        if (addr_mapping_array[rot_addr_arr].addr32 == (uint32_t)addr32bit)
        {
            // need to remove here decrement total_32_to_64_values and replace last one at this position unless this is
            // the the only value or its at the last position

            if (total_32_to_64_values == 1 ||
                (total_32_to_64_values - rot_addr_arr) ==
                    1) // if there is only one value or value found is the last index value
            {
                addr_mapping_array[rot_addr_arr].addr64 = 0;
                addr_mapping_array[rot_addr_arr].addr32 = 0;
            }
            else
            {
                addr_mapping_array[rot_addr_arr].addr64 = 0;
                addr_mapping_array[rot_addr_arr].addr32 = 0;

                temp_save_64_mapped_addr = addr_mapping_array[total_32_to_64_values - 1].addr64;
                temp_save_32_mapped_addr = addr_mapping_array[total_32_to_64_values - 1].addr32;

                addr_mapping_array[total_32_to_64_values - 1].addr64 = 0;
                addr_mapping_array[total_32_to_64_values - 1].addr32 = 0;

                addr_mapping_array[rot_addr_arr].addr64 = temp_save_64_mapped_addr;
                addr_mapping_array[rot_addr_arr].addr32 = temp_save_32_mapped_addr;
            }
            total_32_to_64_values--;

            break;
        }
    }

    return;
}

static void remove_ptr_eventid(int eventid)
{
    void *temp_ptr     = NULL;
    int   temp_eventid = 0;
    for (int rot_eventid = 0; rot_eventid < registered_ptr_eventids; rot_eventid++)
    {
        if (ptr_eventid_array[rot_eventid].eventid == eventid)
        {
            // need to remove here decrement registered_ptr_eventids and replace last one at this position unless this
            // is the the only value or its at the last position

            if (registered_ptr_eventids == 1 ||
                (registered_ptr_eventids - rot_eventid) ==
                    1) // if there is only one value or value found is the last index value
            {
                ptr_eventid_array[rot_eventid].eventid = 0;
                ptr_eventid_array[rot_eventid].ptr_val = NULL;
            }
            else
            {
                ptr_eventid_array[rot_eventid].eventid = 0;
                ptr_eventid_array[rot_eventid].ptr_val = NULL;

                temp_eventid = ptr_eventid_array[registered_ptr_eventids - 1].eventid;
                temp_ptr     = ptr_eventid_array[registered_ptr_eventids - 1].ptr_val;

                ptr_eventid_array[registered_ptr_eventids - 1].eventid = 0;
                ptr_eventid_array[registered_ptr_eventids - 1].ptr_val = NULL;

                ptr_eventid_array[rot_eventid].eventid = temp_eventid;
                ptr_eventid_array[rot_eventid].ptr_val = temp_ptr;
            }
            registered_ptr_eventids--;

            break;
        }
    }

    return;
}

/*This function returens address of the cb function on host side provided
 * eventid*/
static void *get_ptr_from_eventid(int eventid)
{
    void *ret_ptr = NULL;
    for (int rot_eventid = 0; rot_eventid < registered_ptr_eventids; rot_eventid++)
    {
        if (ptr_eventid_array[rot_eventid].eventid == eventid)
        {
            ret_ptr = ptr_eventid_array[rot_eventid].ptr_val;
            break;
        }
    }

    return ret_ptr;
}

static void processApi(int opCode, uint8_t *payloadIdx)
{
    switch (opCode)
    {
    case NCP_CMD_OPCODE_SetThreadEnabled:
        process_SetThreadEnabled(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otIp6SetEnabled:
        process_otIp6SetEnabled(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_GET_ROLE:
        process_otThreadGetDeviceRole(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otDatasetSetActiveTlvs:
        process_otDatasetSetActiveTlvs(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otIp6IsEnabled:
        process_otIp6IsEnabled(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_GET_IPADDR:
        process_otIp6GetUnicastAddresses(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otDatasetGetActiveTlvs:
        process_otDatasetGetActiveTlvs(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otDatasetIsCommissioned:
        process_otDatasetIsCommissioned(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otDatasetGetActive:
        process_otDatasetGetActive(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otDatasetGetPendingTlvs:
        process_otDatasetGetPendingTlvs(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otDatasetSetPendingTlvs:
        process_otDatasetSetPendingTlvs(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otInstanceErasePersistentInfo:
        process_otInstanceErasePersistentInfo(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadIsRouterEligible:
        process_otThreadIsRouterEligible(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otLinkGetCslPeriod:
        process_otLinkGetCslPeriod(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadSetRouterEligible:
        process_otThreadSetRouterEligible(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetRloc16:
        process_otThreadGetRloc16(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetLeaderRouterId:
        process_otThreadGetLeaderRouterId(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetPartitionId:
        process_otThreadGetPartitionId(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otPlatRadioGetRssi:
        process_otPlatRadioGetRssi(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetLeaderWeight:
        process_otThreadGetLeaderWeight(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetLocalLeaderWeight:
        process_otThreadGetLocalLeaderWeight(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetVersion:
        process_otThreadGetVersion(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otLinkGetPollPeriod:
        process_otLinkGetPollPeriod(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otLinkSetCslPeriod:
        process_otLinkSetCslPeriod(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otLinkSetPollPeriod:
        process_otLinkSetPollPeriod(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otLinkGetPanId:
        process_otLinkGetPanId(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otNetDataGetStableVersion:
        process_otNetDataGetStableVersion(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otSrpClientSetLeaseInterval:
        process_otSrpClientSetLeaseInterval(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otSrpClientSetKeyLeaseInterval:
        process_otSrpClientSetKeyLeaseInterval(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otSrpClientRemoveHostAndServices:
        process_otSrpClientRemoveHostAndServices(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otSrpClientEnableAutoHostAddress:
        process_otSrpClientEnableAutoHostAddress(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadSetLinkMode:
        process_otThreadSetLinkMode(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetLinkMode:
        process_otThreadGetLinkMode(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetParentAverageRssi:
        process_otThreadGetParentAverageRssi(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetParentLastRssi:
        process_otThreadGetParentLastRssi(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetNetworkKey:
        process_otThreadGetNetworkKey(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadErrorToString:
        process_otThreadErrorToString(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otBorderAgentGetId:
        process_otBorderAgentGetId(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetNetworkName:
        process_otThreadGetNetworkName(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otLinkGetExtendedAddress:
        process_otLinkGetExtendedAddress(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetExtendedPanId:
        process_otThreadGetExtendedPanId(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetMeshLocalPrefix:
        process_otThreadGetMeshLocalPrefix(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetLeaderRloc:
        process_otThreadGetLeaderRloc(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otNetDataGet:
        process_otNetDataGet(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otIp6SubscribeMulticastAddress:
        process_otIp6SubscribeMulticastAddress(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otIp6UnsubscribeMulticastAddress:
        process_otIp6UnsubscribeMulticastAddress(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetNextNeighborInfo:
        process_otThreadGetNextNeighborInfo(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otNetDataGetNextRoute:
        process_otNetDataGetNextRoute(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otLinkGetCounters:
        process_otLinkGetCounters(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetIp6Counters:
        process_otThreadGetIp6Counters(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otSetStateChangedCallback:
        process_otSetStateChangedCallback(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otIp6AddressFromString:
        process_otIp6AddressFromString(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otIp6AddressToString:
        process_otIp6AddressToString(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otNetDataGetVersion:
        process_otNetDataGetVersion(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadGetChildInfoById:
        process_otThreadGetChildInfoById(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otIp6GetMulticastAddresses:
        process_otIp6GetMulticastAddresses(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otThreadDiscover:
        process_otThreadDiscover(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otUdpOpen:
        process_otUdpOpen(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otUdpBind:
        process_otUdpBind(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otUdpClose:
        process_otudpClose(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otUdpIsOpen:
        process_otUdpIsOpen(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otUdpSend:
        process_otUdpSend(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otUdpNewMessage:
        process_otUdpNewMessage(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otMessageFree:
        process_otMessageFree(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otMessageAppend:
        process_otMessageAppend(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otSrpClientSetHostName:
        process_otSrpClientSetHostName(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otSrpClientAddService:
        process_otSrpClientAddService(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otSrpClientRemoveService:
        process_otSrpClientRemoveService(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otSrpClientClearService:
        process_otSrpClientClearService(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otSrpClientEnableAutoStartMode:
        process_otSrpClientEnableAutoStartMode(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otSrpClientSetCallback:
        process_otSrpClientSetCallback(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otDnsBrowseResponseGetServiceName:
        process_otDnsBrowseResponseGetServiceName(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otDnsClientBrowse:
        process_otDnsClientBrowse(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otDnsClientGetDefaultConfig:
        process_otDnsClientGetDefaultConfig(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otDnsClientSetDefaultConfig:
        process_otDnsClientSetDefaultConfig(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otDnsInitTxtEntryIterator:
        process_otDnsInitTxtEntryIterator(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otDnsGetNextTxtEntry:
        process_otDnsGetNextTxtEntry(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otDnsClientResolveService:
        process_otDnsClientResolveService(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otIp6IsAddressUnspecified:
        process_otIp6IsAddressUnspecified(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otDnsClientResolveAddress:
        process_otDnsClientResolveAddress(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otIcmp6SetEchoMode:
        process_otIcmp6SetEchoMode(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otIp6SetReceiveFilterEnabled:
        process_otIp6SetReceiveFilterEnabled(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otIp6SetSlaacEnabled:
        process_otIp6SetSlaacEnabled(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otIp6SetReceiveCallback:
        process_otIp6SetReceiveCallback(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otIp6NewMessage:
        process_otIp6NewMessage(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otIp6Send:
        process_otIp6Send(opCode, payloadIdx);
        break;
    case NCP_CMD_OPCODE_otLinkGetChannel:
        process_otLinkGetChannel(opCode, payloadIdx);
        break;
    default:
        configASSERT(false);
    }
}

static void process_otDatasetSetActiveTlvs(int opCode, uint8_t *payloadIdx)
{
    uint8_t                  error        = 0;
    int                      payloadsaved = 0;
    otOperationalDatasetTlvs ncpDatasetTlvs;
    memset(&ncpDatasetTlvs, 0, sizeof(otOperationalDatasetTlvs));
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    int ret_val           = 0;
    ncp_cmd_size          = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncpDatasetTlvs.mLength = (*(uint8_t *)(p_payload_param + payloadsaved++));

    // mTlvs
    ncp_memcpy((uint8_t *)&(ncpDatasetTlvs.mTlvs), (p_payload_param + payloadsaved), ncpDatasetTlvs.mLength,
               &payloadsaved);

    if (gInstance != NULL)
    {
        error = otDatasetSetActiveTlvs(gInstance, &ncpDatasetTlvs);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadGetDeviceRole(int opCode, uint8_t *payloadIdx)
{
    uint8_t devicerole = 0;
    /*common part*/
    int ret_val           = 0;
    ncp_cmd_size          = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    otInstance *device_otInstance;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        devicerole = otThreadGetDeviceRole(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = devicerole;
    ncp_cmd_size += sizeof(devicerole);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_SetThreadEnabled(int opCode, uint8_t *payloadIdx)
{
    uint8_t error = 0;
    /*common part*/
    int ret_val           = 0;
    ncp_cmd_size          = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    bool     user_val        = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    otInstance *device_otInstance;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    user_val = (bool)(*(uint8_t *)(p_payload_param + payloadsaved++));

    if (gInstance != NULL)
    {
        error = otThreadSetEnabled(gInstance, user_val);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otIp6SetEnabled(int opCode, uint8_t *payloadIdx)
{
    uint8_t error = 0;
    /*common part*/
    int ret_val           = 0;
    ncp_cmd_size          = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    bool     user_val        = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    otInstance *device_otInstance;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    user_val = (bool)(*(uint8_t *)(p_payload_param + payloadsaved++));

    if (gInstance != NULL)
    {
        error = otIp6SetEnabled(gInstance, user_val);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otIp6IsEnabled(int opCode, uint8_t *payloadIdx)
{
    uint8_t error = 0;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int         payloadsaved    = 0;
    uint8_t    *p_payload_param = (uint8_t *)payloadIdx;
    otInstance *device_otInstance;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otIp6IsEnabled(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otIp6GetUnicastAddresses(int opCode, uint8_t *payloadIdx)
{
    ncp_cmd_size = 0;
    /*common part*/
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    uint8_t no_of_pointers = 0;
    /*common part*/
    int         payloadsaved    = 0;
    uint8_t    *p_payload_param = (uint8_t *)payloadIdx;
    otInstance *device_otInstance;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        const otNetifAddress *unicastAddrs = otIp6GetUnicastAddresses(gInstance);
        //  otIp6Address mAddress;                ///< The IPv6 unicast address.
        // uint8_t      mPrefixLength;           ///< The Prefix length (in bits).
        // uint8_t      mAddressOrigin;          ///< The IPv6 address origin.
        // bool         mPreferred;          ///< TRUE if the address is preferred, FALSE otherwise.
        // bool         mValid : 1;              ///< TRUE if the address is valid, FALSE otherwise.
        // bool         mScopeOverrideValid : 1; ///< TRUE if the mScopeOverride value is valid, FALSE otherwise.
        // unsigned int mScopeOverride : 4;      ///< The IPv6 scope of this address.
        // bool         mRloc : 1;               ///< TRUE if the address is an RLOC, FALSE otherwise.
        // bool         mMeshLocal : 1;          ///< TRUE if the address is mesh-local, FALSE otherwise.
        // bool         mSrpRegistered : 1;      ///< Used by OT core only (indicates whether registered by SRP Client).
        // const struct otNetifAddress *mNext;   ///< A pointer to the next network interface address.
        for (const otNetifAddress *addr = unicastAddrs; addr; addr = addr->mNext)
        {
            ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&(addr->mAddress), sizeof(otIp6Address), &ncp_cmd_size);

            ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&(addr->mPrefixLength), sizeof(uint8_t), &ncp_cmd_size);

            ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&(addr->mAddressOrigin), sizeof(uint8_t), &ncp_cmd_size);

            ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], addr->mPreferred, &ncp_cmd_size);

            ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], addr->mValid, &ncp_cmd_size);

            ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], addr->mScopeOverrideValid, &ncp_cmd_size);

            ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t)addr->mScopeOverride, &ncp_cmd_size);

            ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], addr->mRloc, &ncp_cmd_size);

            ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], addr->mMeshLocal, &ncp_cmd_size);

            ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], addr->mSrpRegistered, &ncp_cmd_size);

            ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&(addr->mNext), sizeof(uint32_t), &ncp_cmd_size);

            no_of_pointers++;
        }
    }
    else
    {
        ret_val = -1;
    }
    tlv_payload    = (uint8_t *)(&ncp_cmd_buf[ncp_cmd_size]);
    *tlv_payload++ = no_of_pointers; // last item in payload
    ncp_cmd_size += ((uint8_t *)tlv_payload - (uint8_t *)(&ncp_cmd_buf[ncp_cmd_size]));

    *(tlv_response + 1) = (int)ret_val; // return: ret_val
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otDatasetGetActiveTlvs(int opCode, uint8_t *payloadIdx)
{
    uint8_t                  error = 0;
    otOperationalDatasetTlvs get_ncpDatasetTlvs;
    int                      payloadsaved    = 0;
    uint8_t                 *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otDatasetGetActiveTlvs(gInstance, &get_ncpDatasetTlvs);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    // returning TLV data received
    if (error == OT_ERROR_NONE && ret_val != -1)
    {
        // mLength
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDatasetTlvs.mLength, &ncp_cmd_size);

        // mTlvs
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDatasetTlvs.mTlvs, get_ncpDatasetTlvs.mLength,
                   &ncp_cmd_size);
    }
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otDatasetIsCommissioned(int opCode, uint8_t *payloadIdx)
{
    uint8_t  error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otDatasetIsCommissioned(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otDatasetGetActive(int opCode, uint8_t *payloadIdx)
{
    uint8_t              error = 0;
    otOperationalDataset get_ncpDataset;
    int                  payloadsaved    = 0;
    uint8_t             *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otDatasetGetActive(gInstance, &get_ncpDataset);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    // returning TLV data received
    if (error == OT_ERROR_NONE && ret_val != -1)
    {
        // mActiveTimestamp.mSeconds
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mActiveTimestamp.mSeconds, sizeof(uint64_t),
                   &ncp_cmd_size);

        // mActiveTimestamp.mTicks
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mActiveTimestamp.mTicks, sizeof(uint16_t),
                   &ncp_cmd_size);

        // mAuthoritative
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mActiveTimestamp.mAuthoritative,
                   sizeof(uint8_t), &ncp_cmd_size);

        // mPendingTimestamp.mSeconds
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mPendingTimestamp.mSeconds, sizeof(uint64_t),
                   &ncp_cmd_size);

        // mPendingTimestamp. mActiveTimestamp.mTicks
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mPendingTimestamp.mTicks, sizeof(uint16_t),
                   &ncp_cmd_size);

        // mPendingTimestamp.mAuthoritative
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mPendingTimestamp.mAuthoritative,
                   sizeof(uint8_t), &ncp_cmd_size);

        // mNetworkKey.m8|
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mNetworkKey.m8, OT_NETWORK_KEY_SIZE,
                   &ncp_cmd_size);

        // mNetworkName.m8|
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mNetworkName.m8,
                   (OT_NETWORK_NAME_MAX_SIZE + 1), &ncp_cmd_size);

        // mExtendedPanId.m8
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mExtendedPanId.m8, (OT_EXT_PAN_ID_SIZE),
                   &ncp_cmd_size);

        // mMeshLocalPrefix.m8
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mMeshLocalPrefix.m8, (OT_IP6_PREFIX_SIZE),
                   &ncp_cmd_size);

        // mDelay
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mDelay, sizeof(uint32_t), &ncp_cmd_size);

        // mPanId
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mPanId, sizeof(uint16_t), &ncp_cmd_size);

        // mChannel
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mChannel, sizeof(uint16_t), &ncp_cmd_size);

        // mPskc.m8
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mPskc.m8, (OT_PSKC_MAX_SIZE), &ncp_cmd_size);

        // mSecurityPolicy.mRotationTime
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mSecurityPolicy.mRotationTime,
                   sizeof(uint16_t), &ncp_cmd_size);

        //.mSecurityPolicy.mObtainNetworkKeyEnabled
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mSecurityPolicy.mObtainNetworkKeyEnabled,
                         &ncp_cmd_size);

        //.mSecurityPolicy.mNativeCommissioningEnabled
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mSecurityPolicy.mNativeCommissioningEnabled,
                         &ncp_cmd_size);

        //.mSecurityPolicy.mRoutersEnabled
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mSecurityPolicy.mRoutersEnabled, &ncp_cmd_size);

        //.mSecurityPolicy.mExternalCommissioningEnabled
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mSecurityPolicy.mExternalCommissioningEnabled,
                         &ncp_cmd_size);

        //.mSecurityPolicy.mCommercialCommissioningEnabled
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mSecurityPolicy.mCommercialCommissioningEnabled,
                         &ncp_cmd_size);

        //.mSecurityPolicy.mAutonomousEnrollmentEnabled
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mSecurityPolicy.mAutonomousEnrollmentEnabled,
                         &ncp_cmd_size);

        //.mSecurityPolicy.mNetworkKeyProvisioningEnabled
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mSecurityPolicy.mNetworkKeyProvisioningEnabled,
                         &ncp_cmd_size);

        //.mSecurityPolicy.mTobleLinkEnabled
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mSecurityPolicy.mTobleLinkEnabled, &ncp_cmd_size);

        //.mSecurityPolicy.mNonCcmRoutersEnabled
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mSecurityPolicy.mNonCcmRoutersEnabled,
                         &ncp_cmd_size);

        //.mSecurityPolicy.mVersionThresholdForRouting (uint8_t)
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mSecurityPolicy.mVersionThresholdForRouting,
                         &ncp_cmd_size);

        // mChannelMask
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDataset.mChannelMask, sizeof(uint32_t),
                   &ncp_cmd_size);

        // mComponents.mIsActiveTimestampPresent
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mComponents.mIsActiveTimestampPresent,
                         &ncp_cmd_size);

        // mComponents.mIsPendingTimestampPresent
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mComponents.mIsPendingTimestampPresent,
                         &ncp_cmd_size);

        // mComponents.mIsNetworkKeyPresent
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mComponents.mIsNetworkKeyPresent, &ncp_cmd_size);

        // mComponents.mIsNetworkNamePresent
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mComponents.mIsNetworkNamePresent, &ncp_cmd_size);

        // mComponents.mIsExtendedPanIdPresent
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mComponents.mIsExtendedPanIdPresent, &ncp_cmd_size);

        // mComponents.mIsMeshLocalPrefixPresent
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mComponents.mIsMeshLocalPrefixPresent,
                         &ncp_cmd_size);

        // mComponents.mIsDelayPresent
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mComponents.mIsDelayPresent, &ncp_cmd_size);

        // mComponents.mIsPanIdPresent
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mComponents.mIsPanIdPresent, &ncp_cmd_size);

        // mComponents.mIsChannelPresent
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mComponents.mIsChannelPresent, &ncp_cmd_size);

        // mComponents.mIsPskcPresent
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mComponents.mIsPskcPresent, &ncp_cmd_size);

        // mComponents.mIsSecurityPolicyPresent
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mComponents.mIsSecurityPolicyPresent,
                         &ncp_cmd_size);

        // mComponents.mIsChannelMaskPresent
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDataset.mComponents.mIsChannelMaskPresent, &ncp_cmd_size);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otDatasetGetPendingTlvs(int opCode, uint8_t *payloadIdx)
{
    uint8_t                  error = 0;
    otOperationalDatasetTlvs get_ncpDatasetTlvs;
    int                      payloadsaved    = 0;
    uint8_t                 *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otDatasetGetPendingTlvs(gInstance, &get_ncpDatasetTlvs);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    // returning TLV data received
    if (error == OT_ERROR_NONE && ret_val != -1)
    {
        // mLength
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], get_ncpDatasetTlvs.mLength, &ncp_cmd_size);

        // mTlvs
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&get_ncpDatasetTlvs.mTlvs, get_ncpDatasetTlvs.mLength,
                   &ncp_cmd_size);
    }
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otDatasetSetPendingTlvs(int opCode, uint8_t *payloadIdx)
{
    uint8_t                  error           = 0;
    uint8_t                  datasetlen      = 0;
    int                      payloadsaved    = 0;
    uint8_t                 *p_payload_param = (uint8_t *)payloadIdx;
    otOperationalDatasetTlvs ncpDatasetTlvs;
    memset(&ncpDatasetTlvs, 0, sizeof(otOperationalDatasetTlvs));
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncpDatasetTlvs.mLength = (*(uint8_t *)(p_payload_param + payloadsaved++));

    // mTlvs
    ncp_memcpy((uint8_t *)&(ncpDatasetTlvs.mTlvs), (p_payload_param + payloadsaved), ncpDatasetTlvs.mLength,
               &payloadsaved);

    if (gInstance != NULL)
    {
        error = otDatasetSetPendingTlvs(gInstance, &ncpDatasetTlvs);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otInstanceErasePersistentInfo(int opCode, uint8_t *payloadIdx)
{
    uint8_t  error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otInstanceErasePersistentInfo(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadIsRouterEligible(int opCode, uint8_t *payloadIdx)
{
    uint8_t  error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otThreadIsRouterEligible(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otLinkGetCslPeriod(int opCode, uint8_t *payloadIdx)
{
    uint32_t error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
        error = otLinkGetCslPeriod(gInstance);
#endif
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&error, sizeof(uint32_t), &ncp_cmd_size); // copy csl period
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadSetRouterEligible(int opCode, uint8_t *payloadIdx)
{
    uint8_t error = 0;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    bool        user_val     = 0;
    int         payloadsaved = 0;
    otInstance *device_otInstance;
    uint8_t    *p_payload_param = (uint8_t *)payloadIdx;

    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // user value
    ncp_val_mem_copy((uint8_t *)(&user_val), *(uint8_t *)(p_payload_param + payloadsaved), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otThreadSetRouterEligible(gInstance, user_val);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadGetRloc16(int opCode, uint8_t *payloadIdx)
{
    uint16_t error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otThreadGetRloc16(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&error, sizeof(uint16_t), &ncp_cmd_size); // copy csl period
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadGetLeaderRouterId(int opCode, uint8_t *payloadIdx)
{
    uint8_t  error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otThreadGetLeaderRouterId(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&error, sizeof(uint8_t), &ncp_cmd_size); // copy leader id
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadGetPartitionId(int opCode, uint8_t *payloadIdx)
{
    uint32_t error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otThreadGetPartitionId(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&error, sizeof(uint32_t), &ncp_cmd_size); // copy leader id
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otPlatRadioGetRssi(int opCode, uint8_t *payloadIdx)
{
    int8_t   error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otPlatRadioGetRssi(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&error, sizeof(int8_t), &ncp_cmd_size); // copy leader id
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadGetLeaderWeight(int opCode, uint8_t *payloadIdx)
{
    uint8_t  error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otThreadGetLeaderWeight(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&error, sizeof(uint8_t), &ncp_cmd_size); // copy LeaderWeight
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadGetLocalLeaderWeight(int opCode, uint8_t *payloadIdx)
{
    uint8_t  error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otThreadGetLocalLeaderWeight(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&error, sizeof(uint8_t),
                   &ncp_cmd_size); // copy Local LeaderWeight
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadGetVersion(int opCode, uint8_t *payloadIdx)
{
    uint16_t error = 0;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    error = otThreadGetVersion();

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&error, sizeof(uint16_t),
                   &ncp_cmd_size); // copy Thread Version
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otLinkGetPollPeriod(int opCode, uint8_t *payloadIdx)
{
    uint32_t error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otLinkGetPollPeriod(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&error, sizeof(uint32_t), &ncp_cmd_size); // copy PollPeriod
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otLinkSetCslPeriod(int opCode, uint8_t *payloadIdx)
{
    uint8_t error = 0;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    uint32_t cslperiod       = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // cslperiod
    ncp_memcpy((uint8_t *)&cslperiod, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
        error = otLinkSetCslPeriod(gInstance, cslperiod);
#endif
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otLinkSetPollPeriod(int opCode, uint8_t *payloadIdx)
{
    uint8_t error = 0;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    uint32_t pollperiod      = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // pollperiod
    ncp_memcpy((uint8_t *)&pollperiod, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otLinkSetPollPeriod(gInstance, pollperiod);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otLinkGetPanId(int opCode, uint8_t *payloadIdx)
{
    otPanId  error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otLinkGetPanId(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&error, sizeof(uint16_t), &ncp_cmd_size); // copy csl period
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otNetDataGetStableVersion(int opCode, uint8_t *payloadIdx)
{
    uint8_t  error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otNetDataGetStableVersion(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&error, sizeof(uint8_t), &ncp_cmd_size);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otSrpClientSetLeaseInterval(int opCode, uint8_t *payloadIdx)
{
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    uint32_t aInterval       = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // aInterval
    ncp_memcpy((uint8_t *)&aInterval, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        otSrpClientSetLeaseInterval(gInstance, aInterval);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otSrpClientSetKeyLeaseInterval(int opCode, uint8_t *payloadIdx)
{
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    uint32_t aInterval       = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // aInterval
    ncp_memcpy((uint8_t *)&aInterval, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        otSrpClientSetKeyLeaseInterval(gInstance, aInterval);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

void process_otSrpClientRemoveHostAndServices(int opCode, uint8_t *payloadIdx)
{
    uint8_t error = 0;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    bool     aRemoveKeyLease    = 0;
    bool     aSendUnregToServer = 0;
    int      payloadsaved       = 0;
    uint8_t *p_payload_param    = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // aRemoveKeyLease
    ncp_val_mem_copy((uint8_t *)(&aRemoveKeyLease), *(uint8_t *)(p_payload_param + payloadsaved), &payloadsaved);

    // aSendUnregToServer
    ncp_val_mem_copy((uint8_t *)(&aSendUnregToServer), *(uint8_t *)(p_payload_param + payloadsaved), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otSrpClientRemoveHostAndServices(gInstance, aRemoveKeyLease, aSendUnregToServer);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

void process_otSrpClientEnableAutoHostAddress(int opCode, uint8_t *payloadIdx)
{
    uint8_t error = 0;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    bool     aRemoveKeyLease    = 0;
    bool     aSendUnregToServer = 0;
    int      payloadsaved       = 0;
    uint8_t *p_payload_param    = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otSrpClientEnableAutoHostAddress(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

void process_otThreadSetLinkMode(int opCode, uint8_t *payloadIdx)
{
    uint8_t          error = 0;
    otLinkModeConfig linkMode;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    bool     user_val        = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // linkMode.mRxOnWhenIdle
    linkMode.mRxOnWhenIdle = *(uint8_t *)(p_payload_param + payloadsaved++);

    // linkMode.mDeviceType
    linkMode.mDeviceType = *(uint8_t *)(p_payload_param + payloadsaved++);

    // linkMode.mNetworkData
    linkMode.mNetworkData = *(uint8_t *)(p_payload_param + payloadsaved++);

    if (gInstance != NULL)
    {
        error = otThreadSetLinkMode(gInstance, linkMode);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

void process_otThreadGetLinkMode(int opCode, uint8_t *payloadIdx)
{
    otLinkModeConfig linkMode;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        linkMode = otThreadGetLinkMode(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], linkMode.mRxOnWhenIdle, &ncp_cmd_size);

        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], linkMode.mDeviceType, &ncp_cmd_size);

        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], linkMode.mNetworkData, &ncp_cmd_size);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

void process_otThreadGetParentAverageRssi(int opCode, uint8_t *payloadIdx)
{
    int8_t averageRssi;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otThreadGetParentAverageRssi(gInstance, &averageRssi);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (error == OT_ERROR_NONE && ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&averageRssi, sizeof(int8_t),
                   &ncp_cmd_size); // copy rssi value
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

void process_otThreadGetParentLastRssi(int opCode, uint8_t *payloadIdx)
{
    int8_t lastRssi;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otThreadGetParentLastRssi(gInstance, &lastRssi);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (error == OT_ERROR_NONE && ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&lastRssi, sizeof(int8_t), &ncp_cmd_size); // copy rssi value
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

void process_otThreadGetNetworkKey(int opCode, uint8_t *payloadIdx)
{
    otNetworkKey networkKey;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        otThreadGetNetworkKey(gInstance, &networkKey);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&networkKey.m8, OT_NETWORK_KEY_SIZE,
                   &ncp_cmd_size); // copy network key
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

void process_otThreadErrorToString(int opCode, uint8_t *payloadIdx)
{
    const char *p_to_error_string;
    otError     aError;
    uint8_t     stringlen = 0;
    /*common part*/
    ncp_cmd_size          = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    ncp_memcpy((uint8_t *)&aError, (p_payload_param + payloadsaved), sizeof(uint8_t), &payloadsaved);

    p_to_error_string = otThreadErrorToString(aError);

    stringlen = strlen(p_to_error_string) + 1; // string plus null uint8_tacter

    ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], stringlen, &ncp_cmd_size); // copy string total size
    ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)p_to_error_string, (stringlen + 1),
               &ncp_cmd_size); // copy string plus null uint8_tacter

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otBorderAgentGetId(int opCode, uint8_t *payloadIdx)
{
#if OPENTHREAD_CONFIG_BORDER_AGENT_ENABLE
    otBorderAgentId br_agentId;
#endif
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
#if OPENTHREAD_CONFIG_BORDER_AGENT_ENABLE
        error = otBorderAgentGetId(gInstance, &br_agentId);
#endif
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (error == OT_ERROR_NONE && ret_val != -1)
    {
#if OPENTHREAD_CONFIG_BORDER_AGENT_ENABLE
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&br_agentId.mId, OT_BORDER_AGENT_ID_LENGTH,
                   &ncp_cmd_size); //
#endif
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadGetNetworkName(int opCode, uint8_t *payloadIdx)
{
    const char *p_to_string;
    uint8_t     stringlen = 0;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        p_to_string = otThreadGetNetworkName(gInstance);

        stringlen = strlen(p_to_string) + 1; // string plus null uint8_tacter
    }
    else
    {
        ret_val     = -1;
        stringlen   = 0;
        p_to_string = NULL;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], stringlen, &ncp_cmd_size); // copy string total size
    ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)p_to_string, (stringlen + 1),
               &ncp_cmd_size); // copy string plus null uint8_tacter

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otLinkGetExtendedAddress(int opCode, uint8_t *payloadIdx)
{
    const otExtAddress *p_to_string;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        p_to_string = otLinkGetExtendedAddress(gInstance);

        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)p_to_string->m8, OT_EXT_ADDRESS_SIZE, &ncp_cmd_size);
    }
    else
    {
        ret_val = -1;
    }
    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadGetExtendedPanId(int opCode, uint8_t *payloadIdx)
{
    const otExtendedPanId *p_to_string;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        p_to_string = otThreadGetExtendedPanId(gInstance);

        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)p_to_string->m8, OT_EXT_PAN_ID_SIZE, &ncp_cmd_size);
    }
    else
    {
        ret_val = -1;
    }
    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadGetMeshLocalPrefix(int opCode, uint8_t *payloadIdx)
{
    const otMeshLocalPrefix *p_to_string;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        p_to_string = otThreadGetMeshLocalPrefix(gInstance);

        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)p_to_string->m8, OT_IP6_PREFIX_SIZE, &ncp_cmd_size);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadGetLeaderRloc(int opCode, uint8_t *payloadIdx)
{
    otIp6Address address;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otThreadGetLeaderRloc(gInstance, &address);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (error == OT_ERROR_NONE && ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&address, sizeof(otIp6Address), &ncp_cmd_size);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otNetDataGet(int opCode, uint8_t *payloadIdx)
{
    uint8_t *data;
    uint8_t  datalen;
    bool     aStable;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&aStable, (p_payload_param + payloadsaved), sizeof(uint8_t), &payloadsaved);
    ncp_memcpy((uint8_t *)&datalen, (p_payload_param + payloadsaved), sizeof(uint8_t), &payloadsaved);

    data = (uint8_t *)pvPortMalloc(datalen);

    if (gInstance != NULL)
    {
        error = otNetDataGet(gInstance, aStable, data, &datalen);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&datalen, sizeof(int8_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)data, datalen, &ncp_cmd_size);
    }

    vPortFree(data);
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otIp6SubscribeMulticastAddress(int opCode, uint8_t *payloadIdx)
{
    otIp6Address address;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&address, (p_payload_param + payloadsaved), sizeof(otIp6Address), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otIp6SubscribeMulticastAddress(gInstance, &address);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otIp6UnsubscribeMulticastAddress(int opCode, uint8_t *payloadIdx)
{
    otIp6Address address;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&address, (p_payload_param + payloadsaved), sizeof(otIp6Address), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otIp6UnsubscribeMulticastAddress(gInstance, &address);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadGetNextNeighborInfo(int opCode, uint8_t *payloadIdx)
{
    otNeighborInfo         neighborInfo;
    otNeighborInfoIterator iterator;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&iterator, (p_payload_param + payloadsaved), sizeof(otNeighborInfoIterator), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otThreadGetNextNeighborInfo(gInstance, &iterator, &neighborInfo);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (ret_val != -1)
    {
        // otNeighborInfoIterator
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&iterator, sizeof(otNeighborInfoIterator), &ncp_cmd_size);
        // otNeighborInfo
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&neighborInfo.mExtAddress.m8, OT_EXT_ADDRESS_SIZE,
                   &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&neighborInfo.mAge, sizeof(uint32_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&neighborInfo.mConnectionTime, sizeof(uint32_t),
                   &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&neighborInfo.mRloc16, sizeof(uint16_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&neighborInfo.mLinkFrameCounter, sizeof(uint32_t),
                   &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&neighborInfo.mMleFrameCounter, sizeof(uint32_t),
                   &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&neighborInfo.mLinkQualityIn, sizeof(uint8_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&neighborInfo.mAverageRssi, sizeof(int8_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&neighborInfo.mLastRssi, sizeof(int8_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&neighborInfo.mLinkMargin, sizeof(uint8_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&neighborInfo.mFrameErrorRate, sizeof(uint16_t),
                   &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&neighborInfo.mMessageErrorRate, sizeof(uint16_t),
                   &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&neighborInfo.mVersion, sizeof(uint16_t), &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], neighborInfo.mRxOnWhenIdle, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], neighborInfo.mFullThreadDevice, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], neighborInfo.mFullNetworkData, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], neighborInfo.mIsChild, &ncp_cmd_size);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otNetDataGetNextRoute(int opCode, uint8_t *payloadIdx)
{
    otNetworkDataIterator iterator = OT_NETWORK_DATA_ITERATOR_INIT;
    otExternalRouteConfig routeConfig;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&iterator, (p_payload_param + payloadsaved), sizeof(otNeighborInfoIterator), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otNetDataGetNextRoute(gInstance, &iterator, &routeConfig);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (ret_val != -1)
    {
        // otNetworkDataIterator
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&iterator, sizeof(otNetworkDataIterator), &ncp_cmd_size);

        // otExternalRouteConfig
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&routeConfig.mPrefix.mPrefix.mFields, sizeof(otIp6Address),
                   &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&routeConfig.mPrefix.mLength, sizeof(uint8_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&routeConfig.mRloc16, sizeof(uint16_t), &ncp_cmd_size);

        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], routeConfig.mPreference, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], routeConfig.mNat64, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], routeConfig.mStable, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], routeConfig.mNextHopIsThisDevice, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], routeConfig.mAdvPio, &ncp_cmd_size);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otLinkGetCounters(int opCode, uint8_t *payloadIdx)
{
    const otMacCounters *macCounters;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        macCounters = otLinkGetCounters(gInstance);

        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)macCounters, sizeof(otMacCounters), &ncp_cmd_size);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadGetIp6Counters(int opCode, uint8_t *payloadIdx)
{
    const otIpCounters *ipCounters;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        ipCounters = otThreadGetIp6Counters(gInstance);

        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)ipCounters, sizeof(otIpCounters), &ncp_cmd_size);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otSetStateChangedCallback(int opCode, uint8_t *payloadIdx)
{
    otStateChangedCallback *aCallback = NULL;
    uint64_t                aContext;
    void                   *tempaContext = ((uint32_t)rand() << 16) | (uint32_t)rand();
    uint8_t                 error        = 0;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // context
    ncp_memcpy((uint8_t *)&aContext, (p_payload_param + payloadsaved), sizeof(uint64_t), &payloadsaved);

    /*call ot API for otSetStateChangedCallback, we are assuming only one ot instance
     *processStateChange is the callback function on ncp device
     */
    if (gInstance != NULL)
    {
        /*Need to check this if have buff issues -->OPENTHREAD_CONFIG_MAX_STATECHANGE_HANDLERS*/
        error = otSetStateChangedCallback(gInstance, processStateChange, tempaContext);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (error == OT_ERROR_NONE && ret_val != -1)
    {
        /*This mapping will be used during callback function,
         *64bit address will sent to host to call the same callback
         *function as requested on host side
         */
        register_ptr_eventid(tempaContext, NCP_EVENT_ID_OT_STATE_CHANGE);

        map_32_to_64_addr(tempaContext, aContext);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void processStateChange(otChangedFlags aFlags, void *aContext)
{
    int total_tx_len = 0;

    uint64_t hostaContext = get_64_mapped_addr(aContext);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&hostaContext, sizeof(uint64_t), &total_tx_len);

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aFlags, sizeof(otChangedFlags), &total_tx_len);

    /*send as an event to host with evntid value 2 as defined in NCP_EVENT_ID_OT_STATE_CHANGE*/
    ot_send_response(NCP_OT_CMD_EVENT_OT_STATE_CHANGE, NCP_CMD_RESULT_OK, ot_ncp_tx_buf, total_tx_len);
}

static void process_otIp6AddressFromString(int opCode, uint8_t *payloadIdx)
{
    otIp6Address address;
    const char   IPstring[OT_IP6_ADDRESS_STRING_SIZE];
    uint8_t      string_length;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    ncp_memcpy((uint8_t *)&string_length, (p_payload_param + payloadsaved), sizeof(uint8_t), &payloadsaved);
    ncp_memcpy((uint8_t *)IPstring, (p_payload_param + payloadsaved), string_length, &payloadsaved);

    error = otIp6AddressFromString(IPstring, &address);

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&address.mFields, sizeof(otIp6Address), &ncp_cmd_size);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otIp6AddressToString(int opCode, uint8_t *payloadIdx)
{
    otIp6Address address;
    char        *buff;
    uint8_t      bufflen;
    uint16_t     return_string_len = 0;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    ncp_memcpy((uint8_t *)&bufflen, (p_payload_param + payloadsaved), sizeof(uint16_t), &payloadsaved);
    ncp_memcpy((uint8_t *)&address.mFields, (p_payload_param + payloadsaved), sizeof(otIp6Address), &payloadsaved);

    buff = (char *)pvPortMalloc(bufflen);

    otIp6AddressToString(&address, buff, bufflen);

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    return_string_len = (strlen(buff) + 1);
    ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&return_string_len, sizeof(uint16_t), &ncp_cmd_size);
    ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)buff, return_string_len, &ncp_cmd_size);

    vPortFree(buff);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otNetDataGetVersion(int opCode, uint8_t *payloadIdx)
{
    uint8_t  error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otNetDataGetVersion(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&error, sizeof(uint8_t), &ncp_cmd_size);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otThreadGetChildInfoById(int opCode, uint8_t *payloadIdx)
{
    otChildInfo childInfo;
    uint16_t    childId;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&childId, (p_payload_param + payloadsaved), sizeof(uint16_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otThreadGetChildInfoById(gInstance, childId, &childInfo);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mExtAddress.m8, OT_EXT_ADDRESS_SIZE,
                   &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mTimeout, sizeof(uint32_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mAge, sizeof(uint32_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mConnectionTime, sizeof(uint64_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mRloc16, sizeof(uint16_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mChildId, sizeof(uint16_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mNetworkDataVersion, sizeof(uint8_t),
                   &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mLinkQualityIn, sizeof(uint8_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mAverageRssi, sizeof(int8_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mLastRssi, sizeof(int8_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mFrameErrorRate, sizeof(uint16_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mMessageErrorRate, sizeof(uint16_t),
                   &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mQueuedMessageCnt, sizeof(uint16_t),
                   &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mSupervisionInterval, sizeof(uint16_t),
                   &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&childInfo.mVersion, sizeof(uint8_t), &ncp_cmd_size);

        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], childInfo.mRxOnWhenIdle, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], childInfo.mFullThreadDevice, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], childInfo.mFullNetworkData, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], childInfo.mIsStateRestoring, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], childInfo.mIsCslSynced, &ncp_cmd_size);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otIp6GetMulticastAddresses(int opCode, uint8_t *payloadIdx)
{
    ncp_cmd_size = 0;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    uint8_t no_of_pointers = 0;
    /*common part*/
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        const otNetifMulticastAddress *multicastAddrs = otIp6GetMulticastAddresses(gInstance);

        for (const otNetifMulticastAddress *addr = multicastAddrs; addr; addr = addr->mNext)
        {
            ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&(addr->mAddress), sizeof(otIp6Address), &ncp_cmd_size);

            ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&(addr->mNext), sizeof(uint32_t), &ncp_cmd_size);

            no_of_pointers++;
        }
    }
    else
    {
        ret_val = -1;
    }

    ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&no_of_pointers, sizeof(uint8_t), &ncp_cmd_size);
    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void HandleActiveScanResult(otActiveScanResult *aResult, void *aContext)
{
    int     total_tx_len     = 0;
    uint8_t is_scan_finished = ((aResult == NULL) ? 1 : 0);

    uint64_t hostaContext = get_64_mapped_addr(aContext);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&hostaContext, sizeof(uint64_t), &total_tx_len);
    ncp_val_mem_copy(&ot_ncp_tx_buf[total_tx_len], is_scan_finished, &total_tx_len);

    if (!is_scan_finished)
    {
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aResult->mExtAddress.m8, OT_EXT_ADDRESS_SIZE,
                   &total_tx_len);
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aResult->mNetworkName.m8, (OT_NETWORK_NAME_MAX_SIZE + 1),
                   &total_tx_len);
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aResult->mExtendedPanId.m8, OT_EXT_PAN_ID_SIZE,
                   &total_tx_len);
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aResult->mSteeringData.mLength, sizeof(uint8_t),
                   &total_tx_len);
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aResult->mSteeringData.m8, OT_STEERING_DATA_MAX_LENGTH,
                   &total_tx_len);
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aResult->mPanId, sizeof(uint16_t), &total_tx_len);
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aResult->mJoinerUdpPort, sizeof(uint16_t), &total_tx_len);
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aResult->mChannel, sizeof(uint8_t), &total_tx_len);
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aResult->mRssi, sizeof(int8_t), &total_tx_len);
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aResult->mLqi, sizeof(uint8_t), &total_tx_len);
        ncp_val_mem_copy(&ot_ncp_tx_buf[total_tx_len], aResult->mVersion, &total_tx_len);
        ncp_val_mem_copy(&ot_ncp_tx_buf[total_tx_len], aResult->mIsNative, &total_tx_len);
        ncp_val_mem_copy(&ot_ncp_tx_buf[total_tx_len], aResult->mDiscover, &total_tx_len);
        ncp_val_mem_copy(&ot_ncp_tx_buf[total_tx_len], aResult->mIsJoinable, &total_tx_len);
    }

    /*send as an event to host with evntid value 3 as defined in NCP_OT_CMD_EVENT_OT_THREAD_DISCOVER*/
    ot_send_response(NCP_OT_CMD_EVENT_OT_THREAD_DISCOVER, NCP_CMD_RESULT_OK, ot_ncp_tx_buf, total_tx_len);
}

static void process_otThreadDiscover(int opCode, uint8_t *payloadIdx)
{
    otStateChangedCallback *aCallback = NULL;
    uint64_t                aContext;
    void                   *tempaContext = ((uint32_t)rand() << 16) | (uint32_t)rand();
    uint32_t                ScanChannels;
    uint16_t                PanId;
    bool                    aJoiner;
    bool                    aEnableEui64Filtering;
    uint8_t                 error = 0;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&ScanChannels, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);
    ncp_memcpy((uint8_t *)&PanId, (p_payload_param + payloadsaved), sizeof(uint16_t), &payloadsaved);
    aJoiner               = *(uint8_t *)(p_payload_param + payloadsaved++);
    aEnableEui64Filtering = *(uint8_t *)(p_payload_param + payloadsaved++);
    // context
    ncp_memcpy((uint8_t *)&aContext, (p_payload_param + payloadsaved), sizeof(uint64_t), &payloadsaved);

    /*HandleActiveScanResult is the callback function on ncp device*/
    if (gInstance != NULL)
    {
        error = otThreadDiscover(gInstance, ScanChannels, PanId, aJoiner, aEnableEui64Filtering, HandleActiveScanResult,
                                 tempaContext);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (error == OT_ERROR_NONE && ret_val != -1)
    {
        /*This mapping will be used during callback function,
         *64bit address will sent to host to call the same callback
         *function as requested on host side
         */
        register_ptr_eventid(tempaContext, NCP_EVENT_ID_OT_THREAD_DISCOVER);

        map_32_to_64_addr(tempaContext, aContext);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otUdpOpen(int opCode, uint8_t *payloadIdx)
{
    otUdpReceive *aCallback = NULL;
    uint64_t      aContext;
    void         *tempaContext = ((uint32_t)rand() << 16) | (uint32_t)rand();
    uint64_t      recv_device_socket;
    uint8_t       error = 0;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&recv_device_socket, (p_payload_param + payloadsaved), sizeof(uint64_t), &payloadsaved);

    // mSockName
    ncp_memcpy((uint8_t *)&(device_mSocket.mSockName), (p_payload_param + payloadsaved), sizeof(otSockAddr),
               &payloadsaved);

    // mPeerName
    ncp_memcpy((uint8_t *)&(device_mSocket.mPeerName), (p_payload_param + payloadsaved), sizeof(otSockAddr),
               &payloadsaved);

    // mHandler of udp socket
    ncp_memcpy((uint8_t *)&(device_mSocket.mHandler), (p_payload_param + payloadsaved), sizeof(uint32_t),
               &payloadsaved);

    //*mContext
    ncp_memcpy((uint8_t *)&(device_mSocket.mContext), (p_payload_param + payloadsaved), sizeof(uint32_t),
               &payloadsaved);

    //*mHandle
    ncp_memcpy((uint8_t *)&(device_mSocket.mHandle), (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    //*mNext
    ncp_memcpy((uint8_t *)&(device_mSocket.mNext), (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // context
    ncp_memcpy((uint8_t *)&(aContext), (p_payload_param + payloadsaved), sizeof(uint64_t), &payloadsaved);

    /*call ot API for udpopen, we are assuming only one ot instance
     *HandleUdpReceive is the callback function on ncp device
     */
    if (gInstance != NULL)
    {
        error = otUdpOpen(gInstance, &device_mSocket, HandleUdpReceive, tempaContext);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (error == OT_ERROR_NONE && ret_val != -1)
    {
        /*This mapping will be used during callback function,
         *64bit address will sent to host to call the same callback
         *function as requested on host side
         */
        register_ptr_eventid(tempaContext, NCP_EVENT_ID_UDP_RECEIVE);

        map_32_to_64_addr(tempaContext, aContext);

        map_32_to_64_addr(&device_mSocket, recv_device_socket); // map socket values

        // return 32 bit pointer value to host for mapping
        uint32_t device_socket_addr = &device_mSocket;
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&(device_socket_addr), sizeof(uint32_t), &ncp_cmd_size);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void HandleUdpReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    int      total_tx_len = 0;
    uint8_t  buf[1500]    = {0};
    uint16_t readlength;
    uint16_t getlength;
    uint8_t *tlv_var_payload = (uint8_t *)(&ot_ncp_tx_buf[total_tx_len]);

    getlength  = otMessageGetLength(aMessage);
    readlength = otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, sizeof(buf) - 1);
    // buf[length] = '\0';
    // length++; // to accommodate for null uint8_tacter

    uint64_t hostaContext = get_64_mapped_addr(aContext);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&hostaContext, sizeof(uint64_t), &total_tx_len);

    // no need to save aMessage pointer
    // save received msg length

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&getlength, sizeof(uint16_t), &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&readlength, sizeof(uint16_t), &total_tx_len);

    // save received message
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)buf, readlength, &total_tx_len);

    // struct otMessageInfo
    //  mSockAddr
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(aMessageInfo->mSockAddr), sizeof(otIp6Address),
               &total_tx_len);

    // mPeerAddr
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(aMessageInfo->mPeerAddr), sizeof(otIp6Address),
               &total_tx_len);

    // mSockPort
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(aMessageInfo->mSockPort), sizeof(uint16_t), &total_tx_len);

    // mPeerPort
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(aMessageInfo->mPeerPort), sizeof(uint16_t), &total_tx_len);

    // mHopLimit
    ncp_val_mem_copy(&ot_ncp_tx_buf[total_tx_len], aMessageInfo->mHopLimit, &total_tx_len);

    // mEcn
    ncp_val_mem_copy(&ot_ncp_tx_buf[total_tx_len], aMessageInfo->mEcn, &total_tx_len);

    // mIsHostInterface
    ncp_val_mem_copy(&ot_ncp_tx_buf[total_tx_len], aMessageInfo->mIsHostInterface, &total_tx_len);

    // mAllowZeroHopLimit
    ncp_val_mem_copy(&ot_ncp_tx_buf[total_tx_len], aMessageInfo->mAllowZeroHopLimit, &total_tx_len);

    // mMulticastLoop
    ncp_val_mem_copy(&ot_ncp_tx_buf[total_tx_len], aMessageInfo->mMulticastLoop, &total_tx_len);

    /*send as an event to host with evntid value 1 as define in NCP_OT_CMD_EVENT_UDPRECV*/
    ot_send_response(NCP_OT_CMD_EVENT_UDPRECV, NCP_CMD_RESULT_OK, ot_ncp_tx_buf, total_tx_len);
}

static void process_otUdpBind(int opCode, uint8_t *payloadIdx)
{
    uint8_t           error = 0;
    otSockAddr        sockaddr;
    otNetifIdentifier netif;
    otUdpSocket      *device_recv_mSocket;
    int               payloadsaved    = 0;
    uint8_t          *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&device_recv_mSocket, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // asockname.maddress
    ncp_memcpy((uint8_t *)&(sockaddr.mAddress), (p_payload_param + payloadsaved), sizeof(otIp6Address), &payloadsaved);

    // asockname.mPORT
    ncp_memcpy((uint8_t *)&(sockaddr.mPort), (p_payload_param + payloadsaved), sizeof(uint16_t), &payloadsaved);

    // aNetif
    netif = *(uint8_t *)(p_payload_param + payloadsaved++);

    if (gInstance != NULL)
    {
        error = otUdpBind(gInstance, device_recv_mSocket, &sockaddr, netif);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otUdpIsOpen(int opCode, uint8_t *payloadIdx)
{
    uint8_t      error               = 0;
    otUdpSocket *device_recv_mSocket = NULL;
    int          payloadsaved        = 0;
    uint8_t     *p_payload_param     = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&device_recv_mSocket, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otUdpIsOpen(gInstance, device_recv_mSocket);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otudpClose(int opCode, uint8_t *payloadIdx)
{
    uint8_t      error               = 0;
    int          payloadsaved        = 0;
    otUdpSocket *device_recv_mSocket = NULL;
    void        *device_udpcontext;
    uint8_t     *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // socket
    ncp_memcpy((uint8_t *)&device_recv_mSocket, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otUdpClose(gInstance, device_recv_mSocket);
        if (error == OT_ERROR_NONE)
        {
            remove_64_mapped_addr(device_recv_mSocket);
            device_udpcontext = get_ptr_from_eventid(NCP_EVENT_ID_UDP_RECEIVE);
            remove_64_mapped_addr(device_udpcontext);
            remove_ptr_eventid(NCP_EVENT_ID_UDP_RECEIVE);
        }
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otUdpSend(int opCode, uint8_t *payloadIdx)
{
    otMessage    *message = NULL;
    otMessageInfo aMessageInfo;
    otUdpSocket  *device_recv_mSocket = NULL;
    uint8_t       error               = 0;
    int           payloadsaved        = 0;
    uint8_t      *p_payload_param     = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // socket
    ncp_memcpy((uint8_t *)&device_recv_mSocket, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&message, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // otmessageinfo
    ncp_memcpy((uint8_t *)&(aMessageInfo.mSockAddr), (p_payload_param + payloadsaved), sizeof(otIp6Address),
               &payloadsaved);

    // mPeerAddr
    ncp_memcpy((uint8_t *)&(aMessageInfo.mPeerAddr), (p_payload_param + payloadsaved), sizeof(otIp6Address),
               &payloadsaved);

    // mSockPort
    ncp_memcpy((uint8_t *)&(aMessageInfo.mSockPort), (p_payload_param + payloadsaved), sizeof(uint16_t), &payloadsaved);

    // mPeerPort
    ncp_memcpy((uint8_t *)&(aMessageInfo.mPeerPort), (p_payload_param + payloadsaved), sizeof(uint16_t), &payloadsaved);

    //// mHopLimit
    aMessageInfo.mHopLimit = *(uint8_t *)(p_payload_param + payloadsaved++);

    // mEcn
    aMessageInfo.mEcn = *(uint8_t *)(p_payload_param + payloadsaved++);

    // mIsHostInterface
    aMessageInfo.mIsHostInterface = *(uint8_t *)(p_payload_param + payloadsaved++);

    // mAllowZeroHopLimit
    aMessageInfo.mAllowZeroHopLimit = *(uint8_t *)(p_payload_param + payloadsaved++);

    // mMulticastLoop
    aMessageInfo.mMulticastLoop = *(uint8_t *)(p_payload_param + payloadsaved++);

    // Call ot API's to send message
    if (gInstance != NULL)
    {
        error = otUdpSend(gInstance, device_recv_mSocket, message, &aMessageInfo);
        if (error == OT_ERROR_NONE)
        {
            remove_64_mapped_addr(message); // if not executed here, host need to call otMessageFree
        }
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otUdpNewMessage(int opCode, uint8_t *payloadIdx)
{
    uint8_t            error   = 0;
    otMessage         *message = NULL;
    otMessageSettings *p_messageSettings;
    otMessageSettings  messageSettings;

    uint8_t isNULL_aSettings;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // otMessageSettings
    isNULL_aSettings = *(uint8_t *)(p_payload_param + payloadsaved++);
    if (isNULL_aSettings)
    {
        p_messageSettings = NULL;
    }
    else
    {
        messageSettings.mLinkSecurityEnabled = *(uint8_t *)(p_payload_param + payloadsaved++);

        messageSettings.mPriority = *(uint8_t *)(p_payload_param + payloadsaved++);
        p_messageSettings         = &messageSettings;
    }

    if (gInstance != NULL)
    {
        message = otUdpNewMessage(gInstance, p_messageSettings);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (message != NULL) // save mapping to free later for otMessageFree
    {
        map_32_to_64_addr(message, 0); // Just keep track of device side address
    }

    // return 32 bit pointer value to host for mapping
    ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&message, sizeof(uint32_t), &ncp_cmd_size);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

/*Should be called only if oterror != OT_ERROR_NONE for udpsend or otip6send without any condition*/
static void process_otMessageFree(int opCode, uint8_t *payloadIdx)
{
    uint8_t    error   = 0;
    otMessage *message = NULL;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    ncp_memcpy((uint8_t *)&message, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        otMessageFree(message);
        remove_64_mapped_addr(message); // need to remove in a case if udp send is done with error or otip6send. In that
                                        // case process_otMessageFree should not be called by the host
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otMessageAppend(int opCode, uint8_t *payloadIdx)
{
    otMessage *message         = NULL;
    uint16_t   buff_len        = 0;
    uint8_t    error           = 0;
    int        payloadsaved    = 0;
    uint8_t   *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    ncp_memcpy((uint8_t *)&message, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // get buff length
    ncp_memcpy((uint8_t *)&buff_len, (p_payload_param + payloadsaved), sizeof(uint16_t), &payloadsaved);

    // get buff data
    uint8_t *device_buff = pvPortMalloc(buff_len);
    ncp_memcpy((uint8_t *)device_buff, (p_payload_param + payloadsaved), buff_len, &payloadsaved);

    // Call ot API's to send message
    if (gInstance != NULL)
    {
        error = otMessageAppend(message, device_buff, buff_len);
    }
    else
    {
        ret_val = -1;
    }

    vPortFree(device_buff);

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otSrpClientSetHostName(int opCode, uint8_t *payloadIdx)
{
    const char *hostName;
    uint8_t     hostname_len;
    uint16_t    size;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&hostname_len, (p_payload_param + payloadsaved), sizeof(uint8_t), &payloadsaved);

    hostName = otSrpClientBuffersGetHostNameString(gInstance, &size);

    ncp_memcpy((uint8_t *)hostName, (p_payload_param + payloadsaved), hostname_len, &payloadsaved);

    if (gInstance != NULL)
    {
        error = otSrpClientSetHostName(gInstance, hostName);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otSrpClientAddService(int opCode, uint8_t *payloadIdx)
{
    otSrpClientBuffersServiceEntry *entry = NULL;
    uint8_t                         len_mName;
    uint8_t                         len_mInstanceName;
    uint8_t                         len_mSubTypeLabels;
    uint8_t                         len_mTxtEntries;
    char                           *string;
    uint16_t                        size;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        entry = otSrpClientBuffersAllocateService(gInstance);
        if (entry == NULL)
        {
            error = OT_ERROR_NO_BUFS;
            goto exit;
        }

        ncp_memcpy((uint8_t *)&len_mInstanceName, (p_payload_param + payloadsaved), sizeof(uint8_t), &payloadsaved);
        string = otSrpClientBuffersGetServiceEntryInstanceNameString(entry, &size);
        ncp_memcpy((uint8_t *)string, (p_payload_param + payloadsaved), len_mInstanceName, &payloadsaved);

        ncp_memcpy((uint8_t *)&len_mName, (p_payload_param + payloadsaved), sizeof(uint8_t), &payloadsaved);
        string = otSrpClientBuffersGetServiceEntryServiceNameString(entry, &size);
        ncp_memcpy((uint8_t *)string, (p_payload_param + payloadsaved), len_mName, &payloadsaved);

        // mSubTypeLabels handling starts
        ncp_memcpy((uint8_t *)&len_mSubTypeLabels, (p_payload_param + payloadsaved), sizeof(uint8_t), &payloadsaved);
        if (len_mSubTypeLabels == 0) // mSubTypeLabels is null
        {
            entry->mService.mSubTypeLabels = NULL;
        }
        else
        {
            uint16_t arrayLength;
            uint16_t index = 0;
            // we will continue to putt sublabel after InstanceName for the total amount of len_mSubTypeLabels
            ncp_memcpy((uint8_t *)string + len_mName, (p_payload_param + payloadsaved), len_mSubTypeLabels,
                       &payloadsaved);

            const char **subTypeLabels = otSrpClientBuffersGetSubTypeLabelsArray(entry, &arrayLength);

            // we will not entertain last null pointer, because after that there will be no entry to point at
            for (int i = 0; i < ((len_mSubTypeLabels + len_mName) - 1); i++)
            {
                if (index + 1 >= arrayLength) // no buffer left to store further
                {
                    error = OT_ERROR_NO_BUFS;
                    goto exit;
                }
                if (*((char *)string + i) == 0)
                {
                    // should point to the next elemenet after NULL uint8_tacter
                    subTypeLabels[index] = (char *)string + i + 1;
                    index++;
                }
            }
        } // mSubTypeLabels ends starts

        // mTxtEntries start here
        ncp_memcpy((uint8_t *)&len_mTxtEntries, (p_payload_param + payloadsaved), sizeof(len_mTxtEntries),
                   &payloadsaved);
        if (len_mTxtEntries != 0)
        {
            uint8_t *txtBuffer;
            txtBuffer                     = otSrpClientBuffersGetServiceEntryTxtBuffer(entry, &size);
            entry->mTxtEntry.mValueLength = len_mTxtEntries;
            // will be stored in the format len+key+value with total of len_mTxtEntries
            ncp_memcpy((uint8_t *)txtBuffer, (p_payload_param + payloadsaved), len_mTxtEntries, &payloadsaved);
        }

        // mNumTxtEntries
        ncp_memcpy((uint8_t *)&entry->mService.mNumTxtEntries, (p_payload_param + payloadsaved), sizeof(uint8_t),
                   &payloadsaved);

        // mPort
        ncp_memcpy((uint8_t *)&entry->mService.mPort, (p_payload_param + payloadsaved), sizeof(uint16_t),
                   &payloadsaved);

        // mPriority
        ncp_memcpy((uint8_t *)&entry->mService.mPriority, (p_payload_param + payloadsaved), sizeof(uint16_t),
                   &payloadsaved);

        // mWeight
        ncp_memcpy((uint8_t *)&entry->mService.mWeight, (p_payload_param + payloadsaved), sizeof(uint16_t),
                   &payloadsaved);

        // mLease
        ncp_memcpy((uint8_t *)&entry->mService.mLease, (p_payload_param + payloadsaved), sizeof(uint32_t),
                   &payloadsaved);

        //
        ncp_memcpy((uint8_t *)&entry->mService.mKeyLease, (p_payload_param + payloadsaved), sizeof(uint32_t),
                   &payloadsaved);

        error = otSrpClientAddService(gInstance, &entry->mService);

        if (error != OT_ERROR_NONE && entry != NULL)
        {
            otSrpClientBuffersFreeService(gInstance, entry);
        }
    }
    else
    {
        ret_val = -1;
    }

exit:
    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);
    if (error == OT_ERROR_NONE && ret_val != -1)
    {
        // need to send service entry pointer to host for mapping. Host will utilize this for future correspondance
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&entry, sizeof(uint32_t), &ncp_cmd_size);
    }
    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otSrpClientRemoveService(int opCode, uint8_t *payloadIdx)
{
    uint8_t                        *hostName;
    otSrpClientBuffersServiceEntry *entry = NULL;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&entry, (p_payload_param + payloadsaved), sizeof(entry), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otSrpClientRemoveService(
            gInstance,
            &entry->mService); // need to check deletion of entry instance during otSrpClientCallback.
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otSrpClientClearService(int opCode, uint8_t *payloadIdx)
{
    uint8_t                        *hostName;
    otSrpClientBuffersServiceEntry *entry = NULL;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&entry, (p_payload_param + payloadsaved), sizeof(entry), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otSrpClientClearService(gInstance, &entry->mService);
        if (error == OT_ERROR_NONE && ret_val != -1)
        {
            otSrpClientBuffersFreeService(gInstance, entry);
        }
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void ncp_OnSrpClientStateChange(const otSockAddr *aServerSockAddr, void *aContext)
{
    int total_tx_len = 0;

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aServerSockAddr->mAddress, sizeof(otIp6Address),
               &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aServerSockAddr->mPort, sizeof(uint16_t), &total_tx_len);

    /*send as an event to host with evntid value 4 as defined in NCP_EVENT_ID_OT_SRP_CLIENT_STATE_CHANGE*/
    ot_send_response(NCP_OT_CMD_EVENT_OT_SRP_CLIENT_STATE_CHANGE, NCP_CMD_RESULT_OK, ot_ncp_tx_buf, total_tx_len);
}

static void process_otSrpClientEnableAutoStartMode(int opCode, uint8_t *payloadIdx)
{
    otSrpClientAutoStartCallback *aCallback = NULL;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    /*ncp_OnSrpClientStateChange is the callback function on ncp device*/
    if (gInstance != NULL)
    {
        otSrpClientEnableAutoStartMode(gInstance, ncp_OnSrpClientStateChange, NULL);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void ncp_SrpClientCallback(otError                    aError,
                                  const otSrpClientHostInfo *aHostInfo,
                                  const otSrpClientService  *aServices,
                                  const otSrpClientService  *aRemovedServices,
                                  void                      *aContext)
{
    int                       total_tx_len         = 0;
    uint8_t                   len_mName            = (aHostInfo == NULL) ? 0 : (strlen(aHostInfo->mName) + 1);
    uint8_t                   len_aServices        = (aServices == NULL) ? 0 : 1;
    uint8_t                   cnt_Services         = 0;
    uint8_t                   len_aRemovedServices = (aRemovedServices == NULL) ? 0 : 1;
    const otSrpClientService *service;
    otSrpClientService       *next;

    // otError
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aError, sizeof(uint8_t), &total_tx_len);

    // aHostInfo
    // Host name (label) string (NULL if not yet set), so check if its null or not, and send string accordingly
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&len_mName, sizeof(uint8_t), &total_tx_len);
    if (len_mName != 0) // if not zero, copy string along with null uint8_tacter
    {
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)aHostInfo->mName, len_mName, &total_tx_len);
    }

    // IPv6 addresses
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aHostInfo->mNumAddresses, sizeof(uint8_t), &total_tx_len);
    // copy all address based on numberof addresses, if not zero
    if (aHostInfo->mNumAddresses != 0)
    {
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)aHostInfo->mAddresses,
                   (sizeof(otIp6Address) * aHostInfo->mNumAddresses), &total_tx_len);
    }

    // mAutoAddress
    ncp_val_mem_copy(&ot_ncp_tx_buf[total_tx_len], aHostInfo->mAutoAddress, &total_tx_len);
    // Host info state --> mState
    ncp_val_mem_copy(&ot_ncp_tx_buf[total_tx_len], aHostInfo->mState, &total_tx_len);

    /*aServices processing of type otSrpClientServic. aServices Can be NULL. The head of linked-list containing all
     * services (excluding the ones removed). NULL if the list is empty. */
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&len_aServices, sizeof(len_aServices), &total_tx_len);

    /* Zero value in len_Services mean no entry for aServices,
     * else no. of aServices will be determined by cnt_Services*/
    if (len_aServices != 0)
    {
        // first calculate no of aservices using this for loop
        for (service = aServices; service != NULL; service = service->mNext)
        {
            cnt_Services++;
        }
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&cnt_Services, sizeof(cnt_Services),
                   &total_tx_len); // copy count of services

        for (service = aServices; service != NULL; service = service->mNext) // go through aservices and save data
        {
            // store addresses for aservices on device side
            ncp_memcpy((uint8_t *)(&ot_ncp_tx_buf[total_tx_len]), (uint8_t *)&service, sizeof(uint32_t), &total_tx_len);

        } // end of for loop for aservices

    } // aServices processing ends here

    // aRemovedServices processing starts here
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&len_aRemovedServices, sizeof(len_aRemovedServices),
               &total_tx_len);

    /* Zero value in len_aRemovedServices mean no entry for aRemovedServices,
     * else no. of aServices will be determined by cnt_Services*/
    if (len_aRemovedServices != 0)
    {
        cnt_Services = 0;

        // first calculate no of aRemovedServices using this for loop
        for (service = aRemovedServices; service != NULL; service = service->mNext)
        {
            cnt_Services++;
        }
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&cnt_Services, sizeof(cnt_Services),
                   &total_tx_len); // copy count of services

        for (const otSrpClientService *service = aRemovedServices; service != NULL; service = next)
        {
            next = service->mNext;
            ncp_memcpy((uint8_t *)(&ot_ncp_tx_buf[total_tx_len]), (uint8_t *)&service, sizeof(uint32_t), &total_tx_len);
            // free buffer entries fro the removed service
            otSrpClientBuffersFreeService(gInstance, (otSrpClientBuffersServiceEntry *)service);

        } // end of for loop for aservices
    }
    // aRemovedServices processing ends here

    /*send as an event to host with evntid value 5 as defined in NCP_OT_CMD_EVENT_OT_SRP_CLIENT_SET_CALLBACK*/
    ot_send_response(NCP_OT_CMD_EVENT_OT_SRP_CLIENT_SET_CALLBACK, NCP_CMD_RESULT_OK, ot_ncp_tx_buf, total_tx_len);
}

static void process_otSrpClientSetCallback(int opCode, uint8_t *payloadIdx)
{
    otSrpClientCallback *aCallback = NULL;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    /*ncp_SrpClientCallback is the callback function on ncp device*/
    if (gInstance != NULL)
    {
        otSrpClientSetCallback(gInstance, ncp_SrpClientCallback, NULL);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otDnsBrowseResponseGetServiceName(int opCode, uint8_t *payloadIdx)
{
    uint8_t                    error            = 0;
    uint32_t                   device_aResponse = 0;
    uint16_t                   aNameBufferSize  = 0;
    const otDnsBrowseResponse *aResponse        = NULL;
    /// to be impl
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // device_aResponse
    ncp_memcpy((uint8_t *)&device_aResponse, (p_payload_param + payloadsaved), sizeof(device_aResponse), &payloadsaved);
    aResponse = (otDnsBrowseResponse *)device_aResponse;

    // aNameBufferSize
    ncp_memcpy((uint8_t *)&aNameBufferSize, (p_payload_param + payloadsaved), sizeof(aNameBufferSize), &payloadsaved);
    // allocate buffer according to the receievd size
    char buffer_name[aNameBufferSize];

    if (aResponse != NULL)
    {
        error = otDnsBrowseResponseGetServiceName(aResponse, buffer_name, aNameBufferSize);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (error == OT_ERROR_NONE && ret_val != -1)
    {
        // copy buffer (buffer_name) according to the size (aNameBufferSize)
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)buffer_name, aNameBufferSize, &ncp_cmd_size);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otDnsClientBrowse(int opCode, uint8_t *payloadIdx)
{
    uint64_t aContext;
    void    *tempaContext     = ((uint32_t)rand() << 16) | (uint32_t)rand();
    uint8_t  error            = 0;
    uint8_t  len_aServiceName = 0;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // length of aServiceName
    ncp_memcpy((uint8_t *)&len_aServiceName, (p_payload_param + payloadsaved), sizeof(uint8_t), &payloadsaved);

    // copy aServiceName contents
    const char aServiceName[len_aServiceName];
    ncp_memcpy((uint8_t *)&aServiceName, (p_payload_param + payloadsaved), len_aServiceName, &payloadsaved);

    // context
    ncp_memcpy((uint8_t *)&aContext, (p_payload_param + payloadsaved), sizeof(uint64_t), &payloadsaved);

    /*call ot API for otDnsClientBrowse, we are assuming only one ot instance
     *ncp_OnDnsBrowseResult is the callback function on ncp device
     */
    if (gInstance != NULL)
    {
        error = otDnsClientBrowse(gInstance, aServiceName, ncp_OnDnsBrowseResult, tempaContext, NULL);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (error == OT_ERROR_NONE && ret_val != -1)
    {
        /*This mapping will be used during callback function,
         *64bit address will sent to host to call the same callback
         *function as requested on host side
         */
        // register_ptr_eventid(tempaContext, NCP_EVENT_ID_OT_DNS_CLIENT_BROWSE);

        map_32_to_64_addr(tempaContext, aContext);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void ncp_OnDnsBrowseResult(otError aError, const otDnsBrowseResponse *aResponse, void *aContext)
{
    // if callback is received with specific context, can we clear mapping for context?
    int total_tx_len = 0;

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aError, sizeof(uint8_t), &total_tx_len);

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aResponse, sizeof(uint32_t), &total_tx_len);

    uint64_t hostaContext = get_64_mapped_addr(aContext);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&hostaContext, sizeof(uint64_t), &total_tx_len);

    /*Start of handling of APIs which are called within this callback*/

    /*send as an event to host with evntid value 6 as defined in NCP_EVENT_ID_OT_DNS_CLIENT_BROWSE,
    otDnsBrowseResponseGetServiceInstance, otDnsBrowseResponseGetServiceName, otDnsBrowseResponseGetServiceInfo*/
    /*As a first step, assume we have one instance record and we retrie it from index=0, but keep assert if we
    have muliple instance records for the given aresponse and need further implementation in loop manner*/

    /*Need to call first otDnsBrowseResponseGetServiceName, otDnsBrowseResponseGetServiceInstance, and
     otDnsBrowseResponseGetServiceInfo*/

    char             name[OT_DNS_MAX_NAME_SIZE]   = {0}; // 255
    char             label[OT_DNS_MAX_LABEL_SIZE] = {0}; // 64
    uint8_t          txtBuffer[MAX_TXTBUFFER_LEN] = {0}; // 512 kMaxTxtDataSize
    otDnsServiceInfo serviceInfo;
    uint16_t         index = 0;

    uint8_t  oterror_getservicename     = 0;
    uint8_t  oterror_getserviceinstance = 0;
    uint8_t  oterror_getserviceinfo     = 0;
    uint16_t dynamic_datasize           = 0;
    uint64_t dynamic_datasize_addr;

    oterror_getservicename = otDnsBrowseResponseGetServiceName(aResponse, name, sizeof(name));

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&oterror_getservicename, sizeof(uint8_t), &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)name, sizeof(name), &total_tx_len);

    /*retrieve instance records*/
    while (true)
    {
        oterror_getserviceinstance = otDnsBrowseResponseGetServiceInstance(aResponse, index, label, sizeof(label));

        /*Need to retrun in the order as -->
            index, oterror_getserviceinfo, label, serviceinfo, */

        dynamic_datasize      = 0;
        dynamic_datasize_addr = total_tx_len;
        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&dynamic_datasize, sizeof(uint16_t), &total_tx_len);

        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&index, sizeof(index), &total_tx_len);

        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&oterror_getserviceinstance, sizeof(uint8_t),
                   &total_tx_len);

        ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)label, sizeof(label), &total_tx_len);

        if (oterror_getserviceinstance == OT_ERROR_NONE)
        {
            serviceInfo.mHostNameBuffer     = name;
            serviceInfo.mHostNameBufferSize = sizeof(name);
            serviceInfo.mTxtData            = txtBuffer;
            serviceInfo.mTxtDataSize        = sizeof(txtBuffer);
            oterror_getserviceinfo          = otDnsBrowseResponseGetServiceInfo(aResponse, label, &serviceInfo);

            ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&oterror_getserviceinfo, sizeof(uint8_t),
                       &total_tx_len);

            /*  uint32_t     mTtl;                ///< Service record TTL (in seconds).
                uint16_t     mPort;               ///< Service port number.

                uint16_t     mPriority;           ///< Service priority.
                uint16_t     mWeight;             ///< Service weight.
                char        *mHostNameBuffer;     ///< Buffer to output the service host name (can be NULL if not
               needed). uint16_t     mHostNameBufferSize; ///< Size of `mHostNameBuffer`. otIp6Address mHostAddress;
               ///< The host IPv6 address. Set to all zero if not available. uint32_t     mHostAddressTtl;     ///< The
               host address TTL. uint8_t     *mTxtData;            ///< Buffer to output TXT data (can be NULL if not
               needed). uint16_t     mTxtDataSize;        ///< On input, size of `mTxtData` buffer. On output number
               bytes written. bool         mTxtDataTruncated;   ///< Indicates if TXT data could not fit in
               `mTxtDataSize` and was truncated. uint32_t     mTxtDataTtl;
                */

            /*currently leaving mHostNameBuffer, mHostNameBufferSize, *mTxtData, mTxtDataSize */
            ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mTtl), sizeof(uint32_t), &total_tx_len);
            ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mPort), sizeof(uint16_t), &total_tx_len);
            ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mPriority), sizeof(uint16_t),
                       &total_tx_len);
            ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mWeight), sizeof(uint16_t),
                       &total_tx_len);
            ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)name, sizeof(name), &total_tx_len);
            ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mHostNameBufferSize), sizeof(uint16_t),
                       &total_tx_len);
            ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mHostAddress), sizeof(otIp6Address),
                       &total_tx_len);
            ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mHostAddressTtl), sizeof(uint32_t),
                       &total_tx_len);
            ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)txtBuffer, sizeof(txtBuffer), &total_tx_len);

            ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mTxtDataSize), sizeof(uint16_t),
                       &total_tx_len);
            ncp_val_mem_copy(&ot_ncp_tx_buf[total_tx_len], serviceInfo.mTxtDataTruncated, &total_tx_len);
            // ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mTxtDataTruncated), sizeof(uint8_t),
            //&total_tx_len);
            ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mTxtDataTtl), sizeof(uint32_t),
                       &total_tx_len);
        }
        // total dynamic size
        dynamic_datasize = total_tx_len - dynamic_datasize_addr -
                           sizeof(dynamic_datasize); // will not cound dynamic_datasize in total data
        ncp_memcpy(&ot_ncp_tx_buf[dynamic_datasize_addr], (uint8_t *)&dynamic_datasize, sizeof(uint16_t),
                   &total_tx_len); // this line need to check because 2 bytes are already reserved for dynamic_datasize

        if (oterror_getserviceinstance != OT_ERROR_NONE)
        {
            break;
        }

        index++;
    }

    assert(index <= 1); /*need to check if we find scenarios where instance record is more than one,
     and need further implementation*/

    remove_64_mapped_addr(aContext); // no need for mapping now

    ot_send_response(NCP_OT_CMD_EVENT_ID_OT_DNS_CLIENT_BROWSE, NCP_CMD_RESULT_OK, ot_ncp_tx_buf, total_tx_len);
}

static void process_otDnsClientGetDefaultConfig(int opCode, uint8_t *payloadIdx)
{
    const otDnsQueryConfig *defaultConfig = NULL;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        defaultConfig = otDnsClientGetDefaultConfig(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        // pointer value at device side for defaultConfig
        // ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&defaultConfig, sizeof(uint32_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&(defaultConfig->mServerSockAddr.mAddress),
                   sizeof(otIp6Address), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&(defaultConfig->mServerSockAddr.mPort), sizeof(uint16_t),
                   &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&(defaultConfig->mResponseTimeout), sizeof(uint32_t),
                   &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], defaultConfig->mMaxTxAttempts, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], defaultConfig->mRecursionFlag, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], defaultConfig->mNat64Mode, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], defaultConfig->mServiceMode, &ncp_cmd_size);
        ncp_val_mem_copy(&ncp_cmd_buf[ncp_cmd_size], defaultConfig->mTransportProto, &ncp_cmd_size);
        // map_32_to_64_addr(tempaContext, aContext); no need here
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otDnsClientSetDefaultConfig(int opCode, uint8_t *payloadIdx)
{
    otDnsQueryConfig Config;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // ncp_memcpy((uint8_t *)&defaultConfig, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&(Config.mServerSockAddr.mAddress), (p_payload_param + payloadsaved), sizeof(otIp6Address),
               &payloadsaved);
    ncp_memcpy((uint8_t *)&(Config.mServerSockAddr.mPort), (p_payload_param + payloadsaved), sizeof(uint16_t),
               &payloadsaved);
    ncp_memcpy((uint8_t *)&(Config.mResponseTimeout), (p_payload_param + payloadsaved), sizeof(uint32_t),
               &payloadsaved);

    Config.mMaxTxAttempts  = *(uint8_t *)(p_payload_param + payloadsaved++);
    Config.mRecursionFlag  = *(uint8_t *)(p_payload_param + payloadsaved++);
    Config.mNat64Mode      = *(uint8_t *)(p_payload_param + payloadsaved++);
    Config.mServiceMode    = *(uint8_t *)(p_payload_param + payloadsaved++);
    Config.mTransportProto = *(uint8_t *)(p_payload_param + payloadsaved++);

    if (gInstance != NULL)
    {
        otDnsClientSetDefaultConfig(gInstance, &Config);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        // if further processingis needed
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otDnsInitTxtEntryIterator(int opCode, uint8_t *payloadIdx)
{
    uint16_t                     aTxtDataLength;
    static otDnsTxtEntryIterator iterator;
    uint32_t                     device_iterator = &iterator;
    static uint8_t              *txtBuffer       = NULL;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    ncp_memcpy((uint8_t *)&aTxtDataLength, (p_payload_param + payloadsaved), sizeof(aTxtDataLength), &payloadsaved);

    /*The buffer pointer @p txtBuffer and its content MUST persist and remain
    unchanged while @p iterator object * is being used. */
    if (txtBuffer == NULL)
    {
        txtBuffer = pvPortMalloc(aTxtDataLength);
    }
    else
    {
        free(txtBuffer);
        txtBuffer = pvPortMalloc(aTxtDataLength);
    }

    ncp_memcpy((uint8_t *)txtBuffer, (p_payload_param + payloadsaved), aTxtDataLength, &payloadsaved);

    otDnsInitTxtEntryIterator(&iterator, txtBuffer, aTxtDataLength);

    // need to send iterator value for mpping purpose
    ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&device_iterator, sizeof(uint32_t), &ncp_cmd_size);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otDnsGetNextTxtEntry(int opCode, uint8_t *payloadIdx)
{
    otDnsTxtEntry          entry;
    otDnsTxtEntryIterator *iterator = NULL;
    otError                error;
    uint16_t               string_len = 0;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    ncp_memcpy((uint8_t *)&iterator, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    error = otDnsGetNextTxtEntry(iterator, &entry);

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (error != OT_ERROR_NOT_FOUND)
    {
        if (entry.mKey == NULL)
        {
            string_len = 0;
        }
        else
        {
            string_len = strlen(entry.mKey) + 1; //+1 for null character
        }
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&string_len, sizeof(uint16_t), &ncp_cmd_size);
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)(entry.mKey), string_len, &ncp_cmd_size);

        // mVaue and mVaueLength
        string_len = entry.mValueLength;
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&string_len, sizeof(uint16_t), &ncp_cmd_size);
        if (string_len != 0)
        {
            ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)(entry.mValue), string_len, &ncp_cmd_size);
        }
    }
    // should free txtBuffer somehow allocated in otDnsInitTxtEntryIterator

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void ncp_otDnsService_cb(otError aError, const otDnsServiceResponse *aResponse, void *aContext)
{
    char             name[OT_DNS_MAX_NAME_SIZE]   = {0}; // 255
    char             label[OT_DNS_MAX_LABEL_SIZE] = {0}; // 64
    uint8_t          txtBuffer[MAX_TXTBUFFER_LEN] = {0}; // 512 kMaxTxtDataSize
    uint8_t          error                        = 0;
    uint16_t         len_apis_data                = 0;
    uint16_t         addr_apis_data               = 0;
    otDnsServiceInfo serviceInfo;
    //////
    int total_tx_len = 0;

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aError, sizeof(uint8_t), &total_tx_len);

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aResponse, sizeof(uint32_t), &total_tx_len);

    uint64_t hostaContext = get_64_mapped_addr(aContext);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&hostaContext, sizeof(uint64_t), &total_tx_len);

    // handling of otDnsServiceResponseGetServiceName
    error = otDnsServiceResponseGetServiceName(aResponse, label, sizeof(label), name, sizeof(name));

    addr_apis_data = total_tx_len; // location of len_apis_data in ot_ncp_tx_buf
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&len_apis_data, sizeof(uint16_t), &total_tx_len);

    len_apis_data = total_tx_len;
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&error, sizeof(uint8_t), &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)label, OT_DNS_MAX_LABEL_SIZE, &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)name, OT_DNS_MAX_NAME_SIZE, &total_tx_len);

    // handling of otDnsServiceResponseGetServiceInfo
    serviceInfo.mHostNameBuffer     = name;
    serviceInfo.mHostNameBufferSize = OT_DNS_MAX_NAME_SIZE;
    serviceInfo.mTxtData            = txtBuffer;
    serviceInfo.mTxtDataSize        = OT_DNS_MAX_LABEL_SIZE;

    error = otDnsServiceResponseGetServiceInfo(aResponse, &serviceInfo);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&error, sizeof(uint8_t), &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mTtl), sizeof(uint32_t), &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mPort), sizeof(uint16_t), &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mPriority), sizeof(uint16_t), &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mWeight), sizeof(uint16_t), &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)name, OT_DNS_MAX_NAME_SIZE, &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mHostNameBufferSize), sizeof(uint16_t),
               &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mHostAddress), sizeof(otIp6Address),
               &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mHostAddressTtl), sizeof(uint32_t),
               &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)txtBuffer, MAX_TXTBUFFER_LEN, &total_tx_len);

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mTxtDataSize), sizeof(uint16_t), &total_tx_len);
    ncp_val_mem_copy(&ot_ncp_tx_buf[total_tx_len], serviceInfo.mTxtDataTruncated, &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(serviceInfo.mTxtDataTtl), sizeof(uint32_t), &total_tx_len);

    len_apis_data = total_tx_len - len_apis_data;

    memcpy(&ot_ncp_tx_buf[addr_apis_data], &len_apis_data, sizeof(uint16_t));

    remove_64_mapped_addr(aContext); // no need for this mapping now

    ot_send_response(NCP_OT_CMD_EVENT_ID_OT_DNS_CLIENT_RESOLVE_SERVICE, NCP_CMD_RESULT_OK, ot_ncp_tx_buf, total_tx_len);
}
static void process_otDnsClientResolveService(int opCode, uint8_t *payloadIdx)
{
    uint64_t          aContext;
    void             *tempaContext       = ((uint32_t)rand() << 16) | (uint32_t)rand();
    uint8_t           error              = 0;
    uint8_t           len_aInstanceLabel = 0;
    uint8_t           len_aServiceName   = 0;
    otDnsQueryConfig *defaultConfig      = NULL;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // len_aInstanceLabel
    ncp_memcpy((uint8_t *)&len_aInstanceLabel, (p_payload_param + payloadsaved), sizeof(uint8_t), &payloadsaved);

    char aInstanceLabel[len_aInstanceLabel];
    // copy aInstanceLabel contents
    ncp_memcpy((uint8_t *)aInstanceLabel, (p_payload_param + payloadsaved), len_aInstanceLabel, &payloadsaved);

    // len_aServiceName
    ncp_memcpy((uint8_t *)&len_aServiceName, (p_payload_param + payloadsaved), sizeof(len_aServiceName), &payloadsaved);

    char aServiceName[len_aServiceName];
    // copy aServiceName contents
    ncp_memcpy((uint8_t *)aServiceName, (p_payload_param + payloadsaved), len_aServiceName, &payloadsaved);

    // context
    ncp_memcpy((uint8_t *)&aContext, (p_payload_param + payloadsaved), sizeof(uint64_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&defaultConfig, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    /*call ot API for otDnsClientResolveService, we are assuming only one ot instance
     *ncp_otDnsService_cb is the callback function on ncp device
     */
    if (gInstance != NULL)
    {
        error = otDnsClientResolveService(gInstance, aInstanceLabel, aServiceName, ncp_otDnsService_cb, tempaContext,
                                          defaultConfig);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (error == OT_ERROR_NONE && ret_val != -1)
    {
        /*This mapping will be used during callback function,
         *64bit address will sent to host to call the same callback
         *function as requested on host side
         */

        map_32_to_64_addr(tempaContext, aContext);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otIp6IsAddressUnspecified(int opCode, uint8_t *payloadIdx)
{
    otIp6Address address;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    uint8_t  error        = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    ncp_memcpy((uint8_t *)&address, (p_payload_param + payloadsaved), sizeof(otIp6Address), &payloadsaved);

    error = otIp6IsAddressUnspecified(&address);

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void ncp_otDnsAddress_cb(otError aError, const otDnsAddressResponse *aResponse, void *aContext)
{
    otIp6Address address;
    uint32_t     ttl;
    uint8_t      error          = 0;
    uint16_t     len_apis_data  = 0;
    uint16_t     addr_apis_data = 0;

    int total_tx_len = 0;

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aError, sizeof(uint8_t), &total_tx_len);

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&aResponse, sizeof(uint32_t), &total_tx_len);

    uint64_t hostaContext = get_64_mapped_addr(aContext);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&hostaContext, sizeof(uint64_t), &total_tx_len);

    // Currently matter is using index=0 vlaue,
    //  handling of otDnsAddressResponseGetAddress
    error = otDnsAddressResponseGetAddress(aResponse, 0, &address, &ttl);

    addr_apis_data = total_tx_len; // location of len_apis_data in ot_ncp_tx_buf
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&len_apis_data, sizeof(uint16_t), &total_tx_len);

    len_apis_data = total_tx_len;
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&error, sizeof(uint8_t), &total_tx_len);

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&(address), sizeof(otIp6Address), &total_tx_len);

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&ttl, sizeof(uint32_t), &total_tx_len);

    len_apis_data = total_tx_len - len_apis_data;

    memcpy(&ot_ncp_tx_buf[addr_apis_data], &len_apis_data, sizeof(uint16_t));

    remove_64_mapped_addr(aContext); // no need for this mapping now

    ot_send_response(NCP_OT_CMD_EVENT_ID_OT_DNS_CLIENT_RESOLVE_ADDRESS, NCP_CMD_RESULT_OK, ot_ncp_tx_buf, total_tx_len);
}

static void process_otDnsClientResolveAddress(int opCode, uint8_t *payloadIdx)
{
    uint64_t          aContext;
    void             *tempaContext  = ((uint32_t)rand() << 16) | (uint32_t)rand();
    uint8_t           error         = 0;
    uint8_t           len_aHostName = 0;
    otDnsQueryConfig *defaultConfig = NULL;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // len_aHostName
    ncp_memcpy((uint8_t *)&len_aHostName, (p_payload_param + payloadsaved), sizeof(len_aHostName), &payloadsaved);

    char aHostName[len_aHostName];
    // copy aServiceName contents
    ncp_memcpy((uint8_t *)aHostName, (p_payload_param + payloadsaved), len_aHostName, &payloadsaved);

    // context
    ncp_memcpy((uint8_t *)&aContext, (p_payload_param + payloadsaved), sizeof(uint64_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&defaultConfig, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    /*call ot API for otDnsClientResolveAddress, we are assuming only one ot instance
     *ncp_otDnsAddress_cb is the callback function on ncp device
     */
    if (gInstance != NULL)
    {
        error = otDnsClientResolveAddress(gInstance, aHostName, ncp_otDnsAddress_cb, tempaContext, defaultConfig);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    if (error == OT_ERROR_NONE && ret_val != -1)
    {
        /*This mapping will be used during callback function,
         *64bit address will sent to host to call the same callback
         *function as requested on host side
         */

        map_32_to_64_addr(tempaContext, aContext);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otIcmp6SetEchoMode(int opCode, uint8_t *payloadIdx)
{
    uint8_t aMode = 0;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // aMode
    ncp_memcpy((uint8_t *)&aMode, (p_payload_param + payloadsaved), sizeof(uint8_t), &payloadsaved);

    if (gInstance != NULL)
    {
        otIcmp6SetEchoMode(gInstance, (otIcmp6EchoMode)aMode);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otIp6SetReceiveFilterEnabled(int opCode, uint8_t *payloadIdx)
{
    uint8_t aEnabled = 0;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // aEnabled
    ncp_memcpy((uint8_t *)&aEnabled, (p_payload_param + payloadsaved), sizeof(uint8_t), &payloadsaved);

    if (gInstance != NULL)
    {
        otIp6SetReceiveFilterEnabled(gInstance, (bool)aEnabled);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otIp6SetSlaacEnabled(int opCode, uint8_t *payloadIdx)
{
    uint8_t aEnabled = 0;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // aEnabled
    ncp_memcpy((uint8_t *)&aEnabled, (p_payload_param + payloadsaved), sizeof(uint8_t), &payloadsaved);

    if (gInstance != NULL)
    {
        otIp6SetSlaacEnabled(gInstance, (bool)aEnabled);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void ncp_otIp6Receive_cb(otMessage *aMessage, void *aContext)
{
    uint8_t  buf[1500]    = {0};
    uint16_t readlength   = otMessageRead(aMessage, 0, buf, sizeof(buf) - 1);
    uint16_t getlength    = otMessageGetLength(aMessage);
    int      total_tx_len = 0;

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&getlength, sizeof(getlength), &total_tx_len);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&readlength, sizeof(readlength), &total_tx_len);

    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)buf, readlength, &total_tx_len);

    uint64_t hostaContext = get_64_mapped_addr(aContext);
    ncp_memcpy(&ot_ncp_tx_buf[total_tx_len], (uint8_t *)&hostaContext, sizeof(uint64_t), &total_tx_len);

    otMessageFree(aMessage);

    ot_send_response(NCP_OT_CMD_EVENT_ID_OT_SET_RECEIVE_CB, NCP_CMD_RESULT_OK, ot_ncp_tx_buf, total_tx_len);
}

static void process_otIp6SetReceiveCallback(int opCode, uint8_t *payloadIdx)
{
    uint64_t          aContext;
    void             *tempaContext  = ((uint32_t)rand() << 16) | (uint32_t)rand();
    uint8_t           error         = 0;
    uint8_t           len_aHostName = 0;
    otDnsQueryConfig *defaultConfig = NULL;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // context
    ncp_memcpy((uint8_t *)&aContext, (p_payload_param + payloadsaved), sizeof(uint64_t), &payloadsaved);

    /*call ot API for otIp6SetReceiveCallback, we are assuming only one ot instance
     *? is the callback function on ncp device
     */

    otIp6SetReceiveCallback(gInstance, ncp_otIp6Receive_cb, tempaContext);

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    /*This mapping will be used during callback function,
     *64bit address will sent to host to call the same callback
     *function as requested on host side
     */

    map_32_to_64_addr(tempaContext, aContext);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otIp6NewMessage(int opCode, uint8_t *payloadIdx)
{
    otMessage        *message = NULL;
    otMessageSettings messageSettings;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    // otMessageSettings
    messageSettings.mLinkSecurityEnabled = *(uint8_t *)(p_payload_param + payloadsaved++);

    messageSettings.mPriority = *(uint8_t *)(p_payload_param + payloadsaved++);

    if (gInstance != NULL)
    {
        message = otIp6NewMessage(gInstance, &messageSettings);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (message != NULL) // save mapping to free later for otip6send or otMessageFree
    {
        map_32_to_64_addr(message, 0); // Just keep track of device side address
    }

    // return 32 bit pointer value to host for mapping
    ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&message, sizeof(uint32_t), &ncp_cmd_size);

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otIp6Send(int opCode, uint8_t *payloadIdx)
{
    otMessage *message = NULL;
    uint8_t    error   = 0;

    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    ncp_memcpy((uint8_t *)&message, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otIp6Send(gInstance, message);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1)                         = (int)ret_val; // return: ret_val
    *(((uint8_t *)tlv_response) + ncp_cmd_size) = error;
    ncp_cmd_size += sizeof(error);

    remove_64_mapped_addr(message); // if not executed here, host need to call otMessageFree

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}

static void process_otLinkGetChannel(int opCode, uint8_t *payloadIdx)
{
    uint8_t  error           = 0;
    int      payloadsaved    = 0;
    uint8_t *p_payload_param = (uint8_t *)payloadIdx;
    /*common part*/
    ncp_cmd_size          = 0;
    int      ret_val      = 0;
    int     *tlv_response = (int *)(&ncp_cmd_buf[ncp_cmd_size]);
    uint8_t *tlv_payload =
        (&ncp_cmd_buf[ncp_cmd_size]) + NCP_API_RESP_HDR_LEN; // will be used if need to some extra payload
    *tlv_response = (int)opCode;
    ncp_cmd_size += NCP_API_RESP_HDR_LEN;
    /*common part*/

    // otInstance
    otInstance *device_otInstance;
    ncp_memcpy((uint8_t *)&device_otInstance, (p_payload_param + payloadsaved), sizeof(uint32_t), &payloadsaved);

    if (gInstance != NULL)
    {
        error = otLinkGetChannel(gInstance);
    }
    else
    {
        ret_val = -1;
    }

    *(tlv_response + 1) = (int)ret_val; // return: ret_val

    if (ret_val != -1)
    {
        ncp_memcpy(&ncp_cmd_buf[ncp_cmd_size], (uint8_t *)&error, sizeof(uint8_t), &ncp_cmd_size);
    }

    ot_send_response(NCP_OT_CMD_MATTER, NCP_CMD_RESULT_OK, ncp_cmd_buf, ncp_cmd_size);
}