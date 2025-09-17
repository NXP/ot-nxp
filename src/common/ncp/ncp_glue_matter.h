/* @file ncp_glue_ot.h
 *
 *  @brief This file contains ncp API functions definitions
 *
 *  Copyright 2025 NXP
 *
 *  Licensed under the LA_OPT_NXP_Software_License.txt (the "Agreement")
 */

#ifndef __NCP_GLUE_MATTER_H__
#define __NCP_GLUE_MATTER_H__
#include "ncp_cmd_ot.h"

typedef struct
{
    uint32_t payload_sz;
    void    *payload_buff;
} otmatter_payload_t;

#define MAX_TXTBUFFER_LEN 512 // refer to kMaxTxtDataSize

#define NCP_CMD_15D4_CLASS 2
#define NCP_CMD_15D4_MATTER_SUBCLASS 2
#define NCP_CALLBACK_TO_EVENTID_ARRAY_SZ 13 // Maximum number of members in callback function to eventid mapping array
#define NCP_EVENT_ID_UDP_RECEIVE 0x1        // UDP_RECEIVE
#define NCP_EVENT_ID_OT_STATE_CHANGE 0x2    //
#define NCP_EVENT_ID_OT_THREAD_DISCOVER 0x3 // otThreadDiscover
#define NCP_EVENT_ID_OT_SRP_CLIENT_STATE_CHANGE 0x4    // otSrpClientEnableAutoStartMode
#define NCP_EVENT_ID_OT_SRP_CLIENT_SET_CALLBACK 0x5    // otSrpClientSetCallback
#define NCP_EVENT_ID_OT_DNS_CLIENT_BROWSE 0x6          // otDnsClientBrowse
#define NCP_EVENT_ID_OT_DNS_CLIENT_RESOLVE_SERVICE 0x7 // OtDnsClientResolveService

#define NCP_API_OPCODE_LEN 0x4     // 4 bytes for opcode
#define NCP_API_RESP_HDR_LEN 0x8   // 4 bytes opcode and 4 bytes ret value
#define MAX_32_TO_64_ARRAY_LEN 256 // Maximum size of array to hold 32 to 64 addr mapping
#define NCP_OT_CMD_EVENT_UDPRECV (NCP_CMD_15D4 | NCP_15d4_CMD_MATTER | NCP_MSG_TYPE_EVENT | 0x00000001)
#define NCP_OT_CMD_EVENT_OT_STATE_CHANGE (NCP_CMD_15D4 | NCP_15d4_CMD_MATTER | NCP_MSG_TYPE_EVENT | 0x00000002)
#define NCP_OT_CMD_EVENT_OT_THREAD_DISCOVER (NCP_CMD_15D4 | NCP_15d4_CMD_MATTER | NCP_MSG_TYPE_EVENT | 0x00000003)
#define NCP_OT_CMD_EVENT_OT_SRP_CLIENT_STATE_CHANGE \
    (NCP_CMD_15D4 | NCP_15d4_CMD_MATTER | NCP_MSG_TYPE_EVENT | 0x00000004)
#define NCP_OT_CMD_EVENT_OT_SRP_CLIENT_SET_CALLBACK \
    (NCP_CMD_15D4 | NCP_15d4_CMD_MATTER | NCP_MSG_TYPE_EVENT | 0x00000005)
#define NCP_OT_CMD_EVENT_ID_OT_DNS_CLIENT_BROWSE (NCP_CMD_15D4 | NCP_15d4_CMD_MATTER | NCP_MSG_TYPE_EVENT | 0x00000006)
#define NCP_OT_CMD_EVENT_ID_OT_DNS_CLIENT_RESOLVE_SERVICE \
    (NCP_CMD_15D4 | NCP_15d4_CMD_MATTER | NCP_MSG_TYPE_EVENT | 0x00000007)
#define NCP_OT_CMD_EVENT_ID_OT_DNS_CLIENT_RESOLVE_ADDRESS \
    (NCP_CMD_15D4 | NCP_15d4_CMD_MATTER | NCP_MSG_TYPE_EVENT | 0x00000008)
#define NCP_OT_CMD_EVENT_ID_OT_SET_RECEIVE_CB (NCP_CMD_15D4 | NCP_15d4_CMD_MATTER | NCP_MSG_TYPE_EVENT | 0x00000009)

#define NCP_CMD_OPCODE_DoInit 0x1                              // DoInit
#define NCP_CMD_OPCODE_SetThreadEnabled 0x2                    // createnetwork
#define NCP_CMD_OPCODE_GET_IPADDR 0x3                          // getipaddr
#define NCP_CMD_OPCODE_GET_ROLE 0x4                            // device role
#define NCP_CMD_OPCODE_IsThreadEnabled 0x5                     // check if ThreadEnabled
#define NCP_CMD_OPCODE_otUdpOpen 0x6                           // to open udp socket
#define NCP_CMD_OPCODE_otUdpBind 0x7                           // udp bind
#define NCP_CMD_OPCODE_otUdpClose 0x8                          // otUdpClose
#define NCP_CMD_OPCODE_otUdpSend 0x9                           // otUdpSend
#define NCP_CMD_OPCODE_otdatasetinit 0xA                       // "dataset init new"
#define NCP_CMD_OPCODE_otdataset_val_set 0xB                   // "set value for specific dataset member
#define NCP_CMD_OPCODE_otIp6SetEnabled 0xC                     //
#define NCP_CMD_OPCODE_otIp6IsEnabled 0xD                      // otIp6IsEnabled
#define NCP_CMD_OPCODE_otSysInit 0xE                           // otSysInit
#define NCP_CMD_OPCODE_otInstanceInitSingle 0xF                // otInstanceInitSingle
#define NCP_CMD_OPCODE_otUdpNewMessage 0x10                    // otUdpNewMessage
#define NCP_CMD_OPCODE_otMessageFree 0x11                      // otMessageFree
#define NCP_CMD_OPCODE_otMessageAppend 0x12                    // otMessageAppend
#define NCP_CMD_OPCODE_otUdpIsOpen 0x13                        // otUdpIsOpen
#define NCP_CMD_OPCODE_otMessageGetLength 0x14                 // otMessageGetLength
#define NCP_CMD_OPCODE_otMessageRead 0x15                      // otMessageRead
#define NCP_CMD_OPCODE_otDatasetSetActiveTlvs 0x16             // set tlvs received
#define NCP_CMD_OPCODE_otDatasetIsCommissioned 0x17            // otDatasetIsCommissioned
#define NCP_CMD_OPCODE_otDatasetGetActiveTlvs 0x18             // otDatasetGetActiveTlvs
#define NCP_CMD_OPCODE_otDatasetGetActive 0x19                 // otDatasetGetActive
#define NCP_CMD_OPCODE_otDatasetGetPendingTlvs 0x1A            // otDatasetGetPendingTlvs
#define NCP_CMD_OPCODE_otDatasetSetPendingTlvs 0x1B            // otDatasetSetPendingTlvs
#define NCP_CMD_OPCODE_otInstanceErasePersistentInfo 0x1C      // otInstanceErasePersistentInfo
#define NCP_CMD_OPCODE_otThreadIsRouterEligible 0x1D           // otThreadIsRouterEligible
#define NCP_CMD_OPCODE_otLinkGetCslPeriod 0x1E                 // otLinkGetCslPeriod
#define NCP_CMD_OPCODE_otThreadSetRouterEligible 0x1F          // otThreadSetRouterEligible
#define NCP_CMD_OPCODE_otThreadGetRloc16 0x20                  // otThreadGetRloc16
#define NCP_CMD_OPCODE_otThreadGetLeaderRouterId 0x21          // otThreadGetLeaderRouterId
#define NCP_CMD_OPCODE_otThreadGetPartitionId 0x22             // otThreadGetPartitionId
#define NCP_CMD_OPCODE_otPlatRadioGetRssi 0x23                 // otPlatRadioGetRssi
#define NCP_CMD_OPCODE_otThreadGetLeaderWeight 0x24            // otThreadGetLeaderWeight
#define NCP_CMD_OPCODE_otThreadGetLocalLeaderWeight 0x25       // otThreadGetLocalLeaderWeight
#define NCP_CMD_OPCODE_otThreadGetVersion 0x26                 // otThreadGetVersion
#define NCP_CMD_OPCODE_otLinkGetPollPeriod 0x27                // otLinkGetPollPeriod
#define NCP_CMD_OPCODE_otLinkSetCslPeriod 0x28                 // otLinkSetCslPeriod
#define NCP_CMD_OPCODE_otLinkSetPollPeriod 0x29                // otLinkSetPollPeriod
#define NCP_CMD_OPCODE_otLinkGetPanId 0x2A                     // otLinkGetPanId
#define NCP_CMD_OPCODE_otNetDataGetStableVersion 0x2B          // otNetDataGetStableVersion
#define NCP_CMD_OPCODE_otSrpClientSetLeaseInterval 0x2C        // otSrpClientSetLeaseInterval
#define NCP_CMD_OPCODE_otSrpClientSetKeyLeaseInterval 0x2D     // otSrpClientSetKeyLeaseInterval
#define NCP_CMD_OPCODE_otSrpClientRemoveHostAndServices 0x2E   // otSrpClientRemoveHostAndServices
#define NCP_CMD_OPCODE_otSrpClientEnableAutoHostAddress 0x2F   // otSrpClientEnableAutoHostAddress
#define NCP_CMD_OPCODE_otAppCliInit 0x30                       // otAppCliInit
#define NCP_CMD_OPCODE_otThreadSetLinkMode 0x31                // otThreadSetLinkMode
#define NCP_CMD_OPCODE_otThreadGetLinkMode 0x32                // otThreadGetLinkMode
#define NCP_CMD_OPCODE_otThreadGetParentAverageRssi 0x33       // otThreadGetParentAverageRssi
#define NCP_CMD_OPCODE_otThreadGetParentLastRssi 0x34          // otThreadGetParentLastRssi
#define NCP_CMD_OPCODE_otThreadGetNetworkKey 0x35              // otThreadGetNetworkKey
#define NCP_CMD_OPCODE_otThreadErrorToString 0x36              // otThreadErrorToString
#define NCP_CMD_OPCODE_otBorderAgentGetId 0x37                 // otBorderAgentGetId
#define NCP_CMD_OPCODE_otThreadGetNetworkName 0x38             // otThreadGetNetworkName
#define NCP_CMD_OPCODE_otLinkGetExtendedAddress 0x39           // otLinkGetExtendedAddress
#define NCP_CMD_OPCODE_otThreadGetExtendedPanId 0x3A           // otThreadGetExtendedPanId
#define NCP_CMD_OPCODE_otThreadGetMeshLocalPrefix 0x3B         // otThreadGetMeshLocalPrefix
#define NCP_CMD_OPCODE_otThreadGetLeaderRloc 0x3C              // otThreadGetLeaderRloc
#define NCP_CMD_OPCODE_otNetDataGet 0x3D                       // otNetDataGet
#define NCP_CMD_OPCODE_otIp6SubscribeMulticastAddress 0x3E     // otIp6SubscribeMulticastAddress
#define NCP_CMD_OPCODE_otIp6UnsubscribeMulticastAddress 0x3F   // otIp6UnsubscribeMulticastAddress
#define NCP_CMD_OPCODE_otThreadGetNextNeighborInfo 0x40        // otThreadGetNextNeighborInfo
#define NCP_CMD_OPCODE_otNetDataGetNextRoute 0x41              // otNetDataGetNextRoute
#define NCP_CMD_OPCODE_otLinkGetCounters 0x42                  // otLinkGetCounters
#define NCP_CMD_OPCODE_otThreadGetIp6Counters 0x43             // otThreadGetIp6Counters
#define NCP_CMD_OPCODE_otSetStateChangedCallback 0x44          // otSetStateChangedCallback
#define NCP_CMD_OPCODE_otIp6AddressFromString 0x45             // otIp6AddressFromString
#define NCP_CMD_OPCODE_otIp6AddressToString 0x46               // otIp6AddressToString
#define NCP_CMD_OPCODE_otNetDataGetVersion 0x47                // otNetDataGetVersion
#define NCP_CMD_OPCODE_otSysProcessDrivers 0x48                // otSysProcessDrivers
#define NCP_CMD_OPCODE_otTaskletsProcess 0x49                  // otTaskletsProcess
#define NCP_CMD_OPCODE_otThreadGetChildInfoById 0x4A           // otThreadGetChildInfoById
#define NCP_CMD_OPCODE_otIp6GetMulticastAddresses 0x4B         // otIp6GetMulticastAddresses
#define NCP_CMD_OPCODE_otThreadDiscover 0x4C                   // otThreadDiscover
#define NCP_CMD_OPCODE_otSrpClientSetHostName 0x4D             // otSrpClientSetHostName
#define NCP_CMD_OPCODE_otSrpClientAddService 0x4E              // otSrpClientAddService
#define NCP_CMD_OPCODE_otSrpClientRemoveService 0x4F           // otSrpClientRemoveService
#define NCP_CMD_OPCODE_otSrpClientClearService 0x50            // otSrpClientClearService
#define NCP_CMD_OPCODE_otSrpClientEnableAutoStartMode 0x51     // otSrpClientEnableAutoStartMode
#define NCP_CMD_OPCODE_otSrpClientSetCallback 0x52             // otSrpClientSetCallback
#define NCP_CMD_OPCODE_otDnsBrowseResponseGetServiceName 0x53  // otDnsBrowseResponseGetServiceName
#define NCP_CMD_OPCODE_otDnsClientBrowse 0x54                  // otDnsClientBrowse
#define NCP_CMD_OPCODE_otDnsClientGetDefaultConfig 0x55        // otDnsClientGetDefaultConfig
#define NCP_CMD_OPCODE_otDnsClientSetDefaultConfig 0x56        // otDnsClientSetDefaultConfig
#define NCP_CMD_OPCODE_otDnsInitTxtEntryIterator 0x57          // otDnsInitTxtEntryIterator
#define NCP_CMD_OPCODE_otDnsGetNextTxtEntry 0x58               // otDnsGetNextTxtEntry
#define NCP_CMD_OPCODE_otDnsClientResolveService 0x59          // OtDnsClientResolveService
#define NCP_CMD_OPCODE_otDnsServiceResponseGetServiceName 0x5A // otDnsServiceResponseGetServiceName
#define NCP_CMD_OPCODE_otDnsServiceResponseGetServiceInfo 0x5B // otDnsServiceResponseGetServiceInfo
#define NCP_CMD_OPCODE_otIp6IsAddressUnspecified 0x5C          // otIp6IsAddressUnspecified
#define NCP_CMD_OPCODE_otDnsClientResolveAddress 0x5D          // otDnsClientResolveAddress
#define NCP_CMD_OPCODE_otIcmp6SetEchoMode 0x5E                 // otIcmp6SetEchoMode
#define NCP_CMD_OPCODE_otIp6SetReceiveFilterEnabled 0x5F       // otIp6SetReceiveFilterEnabled
#define NCP_CMD_OPCODE_otIp6SetSlaacEnabled 0x60               // otIp6SetSlaacEnabled
#define NCP_CMD_OPCODE_otIp6SetReceiveCallback 0x61            // otIp6SetReceiveCallback
#define NCP_CMD_OPCODE_otIp6NewMessage 0x62                    // otIp6NewMessage
#define NCP_CMD_OPCODE_otIp6Send 0x63                          // otIp6Send
#define NCP_CMD_OPCODE_otLinkGetChannel 0x64                   // otLinkGetChannel

int ncp_matter_ot_cmd_handle(void *cmd, int payloadsize);

bool ncp_ot_fct_process(void);

#endif /* __NCP_GLUE_MATTER_H__ */
