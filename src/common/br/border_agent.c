/*
 *  Copyright (c) 2023-2024, The OpenThread Authors.
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
 *   This file implements the OpenThread Border Agent functionality.
 *
 */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */
#include "border_agent.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <openthread/backbone_router_ftd.h>
#include <openthread/border_agent.h>
#include <openthread/border_routing.h>
#include <openthread/cli.h>
#include <openthread/dataset.h>
#include <openthread/dns.h>
#include <openthread/ip6.h>
#include <openthread/mdns.h>
#include <openthread/thread.h>
#include <openthread/verhoeff_checksum.h>
#include <openthread/platform/crypto.h>
#include <openthread/platform/entropy.h>
#include <openthread/platform/memory.h>
#include <openthread/platform/radio.h>

#include "utils.h"
#include "common/code_utils.hpp"

/* -------------------------------------------------------------------------- */
/*                                 Definitions                                */
/* -------------------------------------------------------------------------- */

#define BORDER_AGENT_PORT 49152
#define BACKBONE_UDP_PORT 61631

#define MAX_TXT_ENTRIES_NUMBER 18
/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

typedef enum
{
    kConnNotAllowed = 0,
    kConnPskc,
    kConnPskd,
    kConnVendor,
    kConnX509,
} ConnectionMode;

typedef enum
{
    kIfNotInitialized = 0,
    kIfInitialized,
    kIfActive,
} ThreadInterfaceStatus;

typedef enum
{
    kInfrequentAvailability = 0,
    kHighAvailability,
} Availability;

typedef enum
{
    kThreadRoleDetached = 0,
    kThreadRoleEndDevice,
    kThreadRoleRouter,
    kThreadRoleLeader
} ThreadRole;

typedef struct BorderAgentStateBitmap
{
    uint32_t mConnectionMode : 3;
    uint32_t mThreadInterfaceStatus : 2;
    uint32_t mAvailability : 2;
    uint32_t mBBRFunctionStatusIsActive : 1;
    uint32_t mBBRFunctionStatusIsPrimary : 1;
    uint32_t mThreadRole : 2;
    uint32_t mEpskcSupported : 1;
} StateBitmap;

typedef struct MeshCopValues
{
    uint64_t mActiveTimestampSeconds;
    uint32_t mPartitionId;
    uint32_t mBitmapValue;
    uint16_t mPort;
    uint8_t  mBBRSeqNo;
    uint8_t  mOmrLength;
    uint8_t  mTxtOmrPrefix[sizeof(otIp6Prefix)];
} MeshCopValues;

static otInstance *sInstance;
static bool        sBorderAgentIsInit;

static otMdnsService sMeshCopService;
static otMdnsService sEpskcService;
static const char    sMeshCopServiceLabel[] = "_meshcop._udp";
static const char    sEpskcServiceLabel[]   = "_meshcop-e._udp";

#if OPENTHREAD_CONFIG_BORDER_AGENT_EPHEMERAL_KEY_ENABLE
static uint8_t sEphemeralKey[10]; ///< Byte values, 9 bytes for the key, one for null terminator.

static uint32_t sEphemeralKeyTimeout;
static bool     sEpskcActive;
#endif

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

static uint8_t PopulateMeshCopService(otDnsTxtEntry *aTxtEntries, MeshCopValues *aMeshCopValues, otInstance *aInstance);
static void    HandleThreadStateChanged(otChangedFlags flags, void *aContext);
static void    PublishMeshCopService(otInstance *aInstance);
static StateBitmap GetStateBitmap(otInstance *aInstance);
static void        HandleMeshCopRegistrationCallback(otInstance *aInstance, otMdnsRequestId aRequestId, otError aError);
static void        PublishEpskcService(void);
static void        UnpublishEpskcService(void);
static otError     GenerateEphemeralKey(void);
static void        HandleEpskcRegistrationCallback(otInstance *aInstance, otMdnsRequestId aRequestId, otError aError);
static void        HandleBorderAgentEphemeralKeyCallback(void *aContext);
static uint16_t    ToBE16(uint16_t v);
static uint32_t    ToBE32(uint32_t v);
static uint64_t    ToBE64(uint64_t v);
static uint32_t    BitmapToUint32(StateBitmap aBitMap);
static const char *GetThreadVersionAsString(void);

/* -------------------------------------------------------------------------- */
/*                              Public functions                             */
/* -------------------------------------------------------------------------- */

void BorderAgentInit(otInstance *aInstance, const char *aHostName)
{
    sInstance = aInstance;

    if (!sBorderAgentIsInit)
    {
        sMeshCopService.mHostName        = aHostName;
        sMeshCopService.mServiceInstance = CreateBaseName(aInstance, baseServiceInstanceName, true);
        sMeshCopService.mServiceType     = sMeshCopServiceLabel;

        PublishMeshCopService(aInstance);

        otSetStateChangedCallback(aInstance, HandleThreadStateChanged, aInstance);

#if OPENTHREAD_CONFIG_BORDER_AGENT_EPHEMERAL_KEY_ENABLE
        otBorderAgentSetEphemeralKeyCallback(aInstance, HandleBorderAgentEphemeralKeyCallback, aInstance);
#endif
        sBorderAgentIsInit = true;
    }
}

void BorderAgentDeInit()
{
    sBorderAgentIsInit = false;
}

#if OPENTHREAD_CONFIG_BORDER_AGENT_EPHEMERAL_KEY_ENABLE
otError BorderAgentEnableEpskcService(uint32_t aTimeout)
{
    sEphemeralKeyTimeout = (aTimeout && aTimeout >= OT_BORDER_AGENT_DEFAULT_EPHEMERAL_KEY_TIMEOUT &&
                            aTimeout <= OT_BORDER_AGENT_MAX_EPHEMERAL_KEY_TIMEOUT)
                               ? aTimeout
                               : OT_BORDER_AGENT_DEFAULT_EPHEMERAL_KEY_TIMEOUT;

    if (!sBorderAgentIsInit)
    {
        return OT_ERROR_INVALID_STATE;
    }

    PublishEpskcService();

    return OT_ERROR_NONE;
}

// Defining functions below as weak allows applications to implement their specific behaviour, i.e., custom
// messages/used print function.
void __attribute__((weak)) PrintEphemeralKey(const char *aEphemeralKey, uint32_t aTimeout)
{
#if OT_APP_CLI_EPHEMERAL_KEY_ADDON
    otCliOutputFormat("\r\n Use this passcode to enable an additional device to administer "
                      "and manage your Thread network, including adding new devices to it. This passcode is "
                      "not required for an app to communicate with existing "
                      "devices on your Thread network.");
    otCliOutputFormat("\r\n\nePSKc : %s", aEphemeralKey);
    otCliOutputFormat("\r\n\nValid for %lu seconds.\r\n%s", aTimeout, "> ");
#else
    ; // do nothing, avoid build failure when ot-cli is not enabled
#endif
}

void __attribute__((weak)) PrintEphemeralKeyExpiredMessage(void)
{
#if OT_APP_CLI_EPHEMERAL_KEY_ADDON
    otCliOutputFormat("\r\nEphemeral Key disabled, PSKc is now in use.\r\n%s", "> ");
#else
    ; // do nothing, avoid build failure when ot-cli is not enabled
#endif
}
#endif
/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

static void PublishMeshCopService(otInstance *aInstance)
{
    otDnsTxtEntry mTxtEntries[MAX_TXT_ENTRIES_NUMBER];
    MeshCopValues meshCopValues = {0};
    uint8_t       numTxtEntries = 0;

    numTxtEntries = PopulateMeshCopService(mTxtEntries, &meshCopValues, aInstance);

    uint8_t  txtBuffer[255]  = {0};
    uint32_t txtBufferOffset = 0;

    for (uint32_t i = 0; i < numTxtEntries; i++)
    {
        uint32_t keySize = strlen(mTxtEntries[i].mKey);
        // add TXT entry len + 1 is for '='
        *(txtBuffer + txtBufferOffset++) = keySize + mTxtEntries[i].mValueLength + 1;

        // add TXT entry key
        memcpy(txtBuffer + txtBufferOffset, mTxtEntries[i].mKey, keySize);
        txtBufferOffset += keySize;

        // add TXT entry value if pointer is not null, if pointer is null it means we have bool value
        if (mTxtEntries[i].mValue)
        {
            *(txtBuffer + txtBufferOffset++) = '=';
            memcpy(txtBuffer + txtBufferOffset, mTxtEntries[i].mValue, mTxtEntries[i].mValueLength);
            txtBufferOffset += mTxtEntries[i].mValueLength;
        }
    }
    sMeshCopService.mTxtData       = txtBuffer;
    sMeshCopService.mTxtDataLength = txtBufferOffset;

    if (otBorderAgentGetState(aInstance) != OT_BORDER_AGENT_STATE_STOPPED)
    {
        sMeshCopService.mPort = otBorderAgentGetUdpPort(aInstance);
    }
    else
    {
        sMeshCopService.mPort = BORDER_AGENT_PORT;
    }

    otMdnsRegisterService(aInstance, &sMeshCopService, 0, HandleMeshCopRegistrationCallback);
}

static void HandleThreadStateChanged(otChangedFlags flags, void *aContext)
{
    if (flags & (OT_CHANGED_THREAD_ROLE | OT_CHANGED_THREAD_EXT_PANID | OT_CHANGED_THREAD_NETWORK_NAME |
                 OT_CHANGED_THREAD_BACKBONE_ROUTER_STATE | OT_CHANGED_THREAD_NETDATA))
    {
        PublishMeshCopService((otInstance *)aContext);
    }
}

static uint8_t PopulateMeshCopService(otDnsTxtEntry *aTxtEntries, MeshCopValues *aMeshCopValues, otInstance *aInstance)
{
    uint8_t i = 0;

    StateBitmap bitmap;
    memset(&bitmap, 0x00, sizeof(bitmap));
    bitmap = GetStateBitmap(aInstance);

    aMeshCopValues->mBitmapValue = ToBE32(BitmapToUint32(bitmap));

    // RV TXT
    aTxtEntries[i].mKey         = "rv";
    aTxtEntries[i].mValue       = (uint8_t *)"1";
    aTxtEntries[i].mValueLength = strlen("1");
    i++;

    // Thread Specification Version TXT
    const char *version         = GetThreadVersionAsString();
    aTxtEntries[i].mKey         = "tv";
    aTxtEntries[i].mValue       = (uint8_t *)(version);
    aTxtEntries[i].mValueLength = strlen(version);
    i++;

    // State bitmap TXT (value is binary encoded)
    aTxtEntries[i].mKey         = "sb";
    aTxtEntries[i].mValue       = (uint8_t *)&aMeshCopValues->mBitmapValue;
    aTxtEntries[i].mValueLength = sizeof(aMeshCopValues->mBitmapValue);
    i++;

    // Network name TXT
    aTxtEntries[i].mKey         = "nn";
    aTxtEntries[i].mValue       = (uint8_t *)otThreadGetNetworkName(aInstance);
    aTxtEntries[i].mValueLength = strlen(otThreadGetNetworkName(aInstance));
    i++;

    // Extended PAN Id TXT (value is binary encoded)
    aTxtEntries[i].mKey         = "xp";
    aTxtEntries[i].mValue       = (uint8_t *)otThreadGetExtendedPanId(aInstance)->m8;
    aTxtEntries[i].mValueLength = sizeof(otThreadGetExtendedPanId(aInstance)->m8);
    i++;

    // Vendor name TXT
    aTxtEntries[i].mKey         = "vn";
    aTxtEntries[i].mValue       = (uint8_t *)"NXP";
    aTxtEntries[i].mValueLength = strlen("NXP");
    i++;

    // Model name TXT
    aTxtEntries[i].mKey         = "mn";
    aTxtEntries[i].mValue       = (uint8_t *)"BorderRouter";
    aTxtEntries[i].mValueLength = strlen("BorderRouter");
    i++;

    /* Active timestamp TXT and Partition Id TXT (values binary encoded, shall be included only if Thread Interface
     Status is set to '2')*/
    if (bitmap.mThreadInterfaceStatus == kIfActive)
    {
        otOperationalDataset dataSet;
        otDatasetGetActive(aInstance, &dataSet);
        aMeshCopValues->mActiveTimestampSeconds = ToBE64(dataSet.mActiveTimestamp.mSeconds);

        aTxtEntries[i].mKey         = "at";
        aTxtEntries[i].mValue       = (uint8_t *)&aMeshCopValues->mActiveTimestampSeconds;
        aTxtEntries[i].mValueLength = sizeof(aMeshCopValues->mActiveTimestampSeconds);
        i++;

        aMeshCopValues->mPartitionId = ToBE32(otThreadGetPartitionId(aInstance));
        aTxtEntries[i].mKey          = "pt";
        aTxtEntries[i].mValue        = (uint8_t *)&aMeshCopValues->mPartitionId;
        aTxtEntries[i].mValueLength  = sizeof(aMeshCopValues->mPartitionId);
        i++;
    }

#if OPENTHREAD_CONFIG_BACKBONE_ROUTER_ENABLE
    // Domain Name, BBR Sequence number (binary encoded), BBR Port TXT (binary encoded)
    aTxtEntries[i].mKey         = "dn";
    aTxtEntries[i].mValue       = (uint8_t *)otThreadGetDomainName(aInstance);
    aTxtEntries[i].mValueLength = strlen(otThreadGetDomainName(aInstance));
    i++;

    if (bitmap.mBBRFunctionStatusIsActive)
    {
        otBackboneRouterConfig config;
        otBackboneRouterGetConfig(aInstance, &config);

        aMeshCopValues->mBBRSeqNo = config.mSequenceNumber;

        aTxtEntries[i].mKey         = "sq";
        aTxtEntries[i].mValue       = (uint8_t *)&aMeshCopValues->mBBRSeqNo;
        aTxtEntries[i].mValueLength = sizeof(aMeshCopValues->mBBRSeqNo);
        i++;

        aMeshCopValues->mPort = ToBE16(BACKBONE_UDP_PORT);

        aTxtEntries[i].mKey         = "bb";
        aTxtEntries[i].mValue       = (uint8_t *)&aMeshCopValues->mPort;
        aTxtEntries[i].mValueLength = sizeof(aMeshCopValues->mPort);
        i++;
    }
#endif

#if OPENTHREAD_CONFIG_BORDER_ROUTING_ENABLE
    // OMR TXT (binary encoded)
    otIp6Prefix       prefix;
    otRoutePreference preference;

    if (otBorderRoutingGetFavoredOmrPrefix(aInstance, &prefix, &preference) == OT_ERROR_NONE)
    {
        uint8_t omrLength = (prefix.mLength + 7) / 8;

        memcpy(aMeshCopValues->mTxtOmrPrefix, prefix.mPrefix.mFields.m8, omrLength);
        aMeshCopValues->mOmrLength = omrLength;

        aTxtEntries[i].mKey         = "omr";
        aTxtEntries[i].mValue       = (uint8_t *)aMeshCopValues->mTxtOmrPrefix;
        aTxtEntries[i].mValueLength = aMeshCopValues->mOmrLength;
        i++;
    }

#endif

    // Extended Address TXT (binary encoded)
    aTxtEntries[i].mKey         = "xa";
    aTxtEntries[i].mValue       = (uint8_t *)otLinkGetExtendedAddress(aInstance)->m8;
    aTxtEntries[i].mValueLength = sizeof(otLinkGetExtendedAddress(aInstance)->m8);
    i++;

    // Border Agent ID (binary encoded)
    otBorderAgentId id;
    otBorderAgentGetId(aInstance, &id);
    aTxtEntries[i].mKey         = "id";
    aTxtEntries[i].mValue       = (uint8_t *)id.mId;
    aTxtEntries[i].mValueLength = sizeof(otBorderAgentId);
    i++;

    return i;
}

static StateBitmap GetStateBitmap(otInstance *aInstance)
{
    StateBitmap bitmap;
    memset(&bitmap, 0x00, sizeof(bitmap));

    bitmap.mConnectionMode = kConnPskc;
    bitmap.mAvailability   = kHighAvailability;

    switch (otThreadGetDeviceRole(aInstance))
    {
    case OT_DEVICE_ROLE_DISABLED:
        bitmap.mThreadInterfaceStatus = kIfNotInitialized;
        bitmap.mThreadRole            = kThreadRoleDetached;
        break;
    case OT_DEVICE_ROLE_DETACHED:
        bitmap.mThreadInterfaceStatus = kIfInitialized;
        bitmap.mThreadRole            = kThreadRoleDetached;
        break;
    case OT_DEVICE_ROLE_CHILD:
        bitmap.mThreadInterfaceStatus = kIfActive;
        bitmap.mThreadRole            = kThreadRoleEndDevice;
        break;
    case OT_DEVICE_ROLE_ROUTER:
        bitmap.mThreadInterfaceStatus = kIfActive;
        bitmap.mThreadRole            = kThreadRoleRouter;
        break;
    case OT_DEVICE_ROLE_LEADER:
        bitmap.mThreadInterfaceStatus = kIfActive;
        bitmap.mThreadRole            = kThreadRoleLeader;
        break;

    default:
        bitmap.mThreadInterfaceStatus = kIfActive;
    }

#if OPENTHREAD_CONFIG_BACKBONE_ROUTER_ENABLE
    if (bitmap.mThreadInterfaceStatus == kIfActive)
    {
        bitmap.mBBRFunctionStatusIsActive  = otBackboneRouterGetState(aInstance) != OT_BACKBONE_ROUTER_STATE_DISABLED;
        bitmap.mBBRFunctionStatusIsPrimary = otBackboneRouterGetState(aInstance) == OT_BACKBONE_ROUTER_STATE_PRIMARY;
    }
#endif

#if OPENTHREAD_CONFIG_BORDER_AGENT_EPHEMERAL_KEY_ENABLE
    bitmap.mEpskcSupported = 1;
#endif

    return bitmap;
}

static uint32_t BitmapToUint32(StateBitmap aBitMap)
{
    uint32_t bitmap = 0;

    bitmap |= aBitMap.mConnectionMode << 0;
    bitmap |= aBitMap.mThreadInterfaceStatus << 3;
    bitmap |= aBitMap.mAvailability << 5;
    bitmap |= aBitMap.mBBRFunctionStatusIsActive << 7;
    bitmap |= aBitMap.mBBRFunctionStatusIsPrimary << 8;
    bitmap |= aBitMap.mThreadRole << 9;
    bitmap |= aBitMap.mEpskcSupported << 11;

    return bitmap;
}

static void HandleMeshCopRegistrationCallback(otInstance *aInstance, otMdnsRequestId aRequestId, otError aError)
{
    if (aError != OT_ERROR_NONE)
    {
        sMeshCopService.mServiceInstance = CreateAlternativeBaseName(aInstance, sMeshCopService.mServiceInstance);
        PublishMeshCopService(aInstance);
    }
}

#if OPENTHREAD_CONFIG_BORDER_AGENT_EPHEMERAL_KEY_ENABLE
static void PublishEpskcService(void)
{
    static bool init = false;
    if (!init)
    {
        memset(&sEpskcService, 0, sizeof(sEpskcService));

        sEpskcService.mHostName        = sMeshCopService.mHostName;
        sEpskcService.mServiceInstance = sMeshCopService.mServiceInstance;
        sEpskcService.mServiceType     = sEpskcServiceLabel;
        init                           = true;
    }

    sEpskcService.mPort = otBorderAgentGetUdpPort(sInstance);

    otMdnsRegisterService(sInstance, &sEpskcService, 0, HandleEpskcRegistrationCallback);
}

static void UnpublishEpskcService(void)
{
    sEpskcActive = false;
    (void)otMdnsUnregisterService(sInstance, &sEpskcService);
}

static void HandleEpskcRegistrationCallback(otInstance *aInstance, otMdnsRequestId aRequestId, otError aError)
{
    if (aError == OT_ERROR_NONE)
    {
        if (otBorderAgentGetState(sInstance) != OT_BORDER_AGENT_STATE_STOPPED && !sEpskcActive)
        {
            SuccessOrExit(GenerateEphemeralKey());
            (void)otBorderAgentSetEphemeralKey(sInstance, (const char *)sEphemeralKey, sEphemeralKeyTimeout,
                                               otBorderAgentGetUdpPort(sInstance));
        }
    }
exit:
    return;
}

static otError GenerateEphemeralKey(void)
{
    otError  error = OT_ERROR_NONE;
    uint8_t  i     = 0;
    uint32_t randomResult;
    char     verhoeffChecksum;

    memset(sEphemeralKey, 0, sizeof(sEphemeralKey));

    VerifyOrExit(otPlatEntropyGet((uint8_t *)&randomResult, sizeof(randomResult)) == OT_ERROR_NONE,
                 error = OT_ERROR_FAILED);
    randomResult %= 100000000;
    i += snprintf((char *)sEphemeralKey, sizeof(sEphemeralKey), "%08lu", randomResult);
    VerifyOrExit(otVerhoeffChecksumCalculate((const char *)sEphemeralKey, &verhoeffChecksum) == OT_ERROR_NONE,
                 error = OT_ERROR_FAILED);
    i += snprintf((char *)&sEphemeralKey[i], sizeof(sEphemeralKey) - i, "%c", verhoeffChecksum);
exit:
    return error;
}

static void HandleBorderAgentEphemeralKeyCallback(void *aContext)
{
    bool sEpskcActive = otBorderAgentIsEphemeralKeyActive((otInstance *)aContext);

    if (sEpskcActive)
    {
        {
            char formattedEpskc[12];
            snprintf(formattedEpskc, sizeof(formattedEpskc), "%.3s %.3s %.3s", sEphemeralKey, sEphemeralKey + 3,
                     sEphemeralKey + 6);
            PrintEphemeralKey(formattedEpskc, (uint32_t)(sEphemeralKeyTimeout / 1000UL));
        }
    }
    else
    {
        UnpublishEpskcService();
        PrintEphemeralKeyExpiredMessage();
    }
}
#endif // OPENTHREAD_CONFIG_BORDER_AGENT_EPHEMERAL_KEY_ENABLE

static uint16_t ToBE16(uint16_t v)
{
    return (((v & 0x00ffU) << 8) & 0xff00) | (((v & 0xff00U) >> 8) & 0x00ff);
}

static uint32_t ToBE32(uint32_t v)
{
    return ((v & (uint32_t)(0x000000ffUL)) << 24) | ((v & (uint32_t)(0x0000ff00UL)) << 8) |
           ((v & (uint32_t)(0x00ff0000UL)) >> 8) | ((v & (uint32_t)(0xff000000UL)) >> 24);
}

static uint64_t ToBE64(uint64_t v)
{
    return ((v & (uint64_t)(0x00000000000000ffULL)) << 56) | ((v & (uint64_t)(0x000000000000ff00ULL)) << 40) |
           ((v & (uint64_t)(0x0000000000ff0000ULL)) << 24) | ((v & (uint64_t)(0x00000000ff000000ULL)) << 8) |
           ((v & (uint64_t)(0x000000ff00000000ULL)) >> 8) | ((v & (uint64_t)(0x0000ff0000000000ULL)) >> 24) |
           ((v & (uint64_t)(0x00ff000000000000ULL)) >> 40) | ((v & (uint64_t)(0xff00000000000000ULL)) >> 56);
}

static const char *GetThreadVersionAsString(void)
{
    const char *version = NULL;

    switch (otThreadGetVersion())
    {
    case OT_THREAD_VERSION_1_1:
        version = "1.1.1";
        break;
    case OT_THREAD_VERSION_1_2:
        version = "1.2.1";
        break;
    case OT_THREAD_VERSION_1_3:
        version = "1.3.0";
        break;
    case OT_THREAD_VERSION_1_4:
        version = "1.4.0";
        break;
    default:
        break;
    }

    return version;
}
