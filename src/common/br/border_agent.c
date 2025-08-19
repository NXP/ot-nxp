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
 *   This file implements the OpenThread Border Agent functionality.
 *
 */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */
#include "border_agent.h"
#include "utils.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <openthread/border_agent.h>
#include <openthread/cli.h>
#include <openthread/dns.h>
#include <openthread/verhoeff_checksum.h>
#include "common/code_utils.hpp"
/* -------------------------------------------------------------------------- */
/*                                 Definitions                                */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

static otInstance *sInstance;
static bool        sBorderAgentIsInit;

#ifndef OT_NXP_PLAT_BR_BASE_SERVICE_NAME
const char sBaseServiceInstanceName[] = "NXP-BorderRouter#";
#else
const char         sBaseServiceInstanceName[] = OT_NXP_PLAT_BR_BASE_SERVICE_NAME;
#endif

#ifndef OT_NXP_PLAT_BR_VENDOR_NAME
static const char *sVendorValue = "NXP";
#else
static const char *sVendorValue               = OT_NXP_PLAT_BR_VENDOR_NAME;
#endif

#ifndef OT_NXP_PLAT_BR_MODEL_NAME
static const char *sModelValue = "BorderRouter";
#else
static const char *sModelValue                = OT_NXP_PLAT_BR_MODEL_NAME;
#endif

#if OPENTHREAD_CONFIG_BORDER_AGENT_EPHEMERAL_KEY_ENABLE
static uint8_t sEphemeralKey[10]; ///< Byte values, 9 bytes for the key, one for null terminator.

static uint32_t sEphemeralKeyTimeout;
static bool     sEpskcActive;
#endif

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */
#if OPENTHREAD_CONFIG_BORDER_AGENT_EPHEMERAL_KEY_ENABLE
static otError GenerateEphemeralKey(void);
static void    HandleBorderAgentEphemeralKeyCallback(void *aContext);
#endif

static void CreateVendorTxtData(uint8_t *aTxtBuffer, uint16_t *aTxtDataLen);
/* -------------------------------------------------------------------------- */
/*                              Public functions                             */
/* -------------------------------------------------------------------------- */

void BorderAgentInit(otInstance *aInstance)
{
    sInstance = aInstance;

    if (!sBorderAgentIsInit)
    {
        uint8_t  txtBuffer[128] = {0};
        uint16_t txtBufferLen;

        otBorderAgentSetMeshCoPServiceBaseName(aInstance, sBaseServiceInstanceName);
        CreateVendorTxtData(txtBuffer, &txtBufferLen);
        otBorderAgentSetVendorTxtData(aInstance, txtBuffer, txtBufferLen);

#if OPENTHREAD_CONFIG_BORDER_AGENT_EPHEMERAL_KEY_ENABLE
        otBorderAgentEphemeralKeySetEnabled(aInstance, true);
        otBorderAgentEphemeralKeySetCallback(aInstance, HandleBorderAgentEphemeralKeyCallback, aInstance);
#endif
        sBorderAgentIsInit = true;
    }
}

void BorderAgentDeInit()
{
    otBorderAgentEphemeralKeySetEnabled(sInstance, false);
    sBorderAgentIsInit = false;
    sEpskcActive       = false;
}

#if OPENTHREAD_CONFIG_BORDER_AGENT_EPHEMERAL_KEY_ENABLE
otError BorderAgentEnableEpskcService(uint32_t aTimeout)
{
    otError error = OT_ERROR_NONE;

    VerifyOrExit(sBorderAgentIsInit, error = OT_ERROR_INVALID_STATE);

    sEphemeralKeyTimeout = (aTimeout && aTimeout >= OT_BORDER_AGENT_DEFAULT_EPHEMERAL_KEY_TIMEOUT &&
                            aTimeout <= OT_BORDER_AGENT_MAX_EPHEMERAL_KEY_TIMEOUT)
                               ? aTimeout
                               : OT_BORDER_AGENT_DEFAULT_EPHEMERAL_KEY_TIMEOUT;

    VerifyOrExit((GenerateEphemeralKey() == OT_ERROR_NONE), error = OT_ERROR_FAILED);
    error = otBorderAgentEphemeralKeyStart(sInstance, (const char *)sEphemeralKey, sEphemeralKeyTimeout, 0);

exit:
    return error;
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
    otCliOutputFormat("\r\nEphemeral Key disabled.\r\n%s", "> ");
#else
    ; // do nothing, avoid build failure when ot-cli is not enabled
#endif
}
#endif
/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

static void CreateVendorTxtData(uint8_t *aTxtBuffer, uint16_t *aTxtDataLen)
{
    *aTxtDataLen = 0;

    otDnsTxtEntry sTxtEntries[] = {
        {.mKey = "vn", .mValue = (uint8_t *)sVendorValue, .mValueLength = strlen(sVendorValue)},
        {.mKey = "mn", .mValue = (uint8_t *)sModelValue, .mValueLength = strlen(sModelValue)}};

    for (size_t i = 0; i < sizeof(sTxtEntries) / sizeof(sTxtEntries[0]); ++i)
    {
        const otDnsTxtEntry *entry    = &sTxtEntries[i];
        uint8_t              keyLen   = (uint8_t)strlen(entry->mKey);
        uint8_t              totalLen = keyLen + 1 + entry->mValueLength; // +1 for '='

        aTxtBuffer[(*aTxtDataLen)++] = totalLen;
        memcpy(aTxtBuffer + *aTxtDataLen, entry->mKey, keyLen);
        *aTxtDataLen += keyLen;

        aTxtBuffer[(*aTxtDataLen)++] = '=';
        memcpy(aTxtBuffer + *aTxtDataLen, entry->mValue, entry->mValueLength);
        *aTxtDataLen += entry->mValueLength;
    }
}

#if OPENTHREAD_CONFIG_BORDER_AGENT_EPHEMERAL_KEY_ENABLE
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
    char                           formattedEpskc[12];
    otBorderAgentEphemeralKeyState epKeyState = otBorderAgentEphemeralKeyGetState((otInstance *)aContext);

    switch (epKeyState)
    {
    case OT_BORDER_AGENT_STATE_STOPPED:
        if (sEpskcActive)
        {
            sEpskcActive = false;
            PrintEphemeralKeyExpiredMessage();
        }
        else
        {
            // TODO: EPH key ready
        }
        break;

    case OT_BORDER_AGENT_STATE_STARTED:
        snprintf(formattedEpskc, sizeof(formattedEpskc), "%.3s %.3s %.3s", sEphemeralKey, sEphemeralKey + 3,
                 sEphemeralKey + 6);
        PrintEphemeralKey(formattedEpskc, (uint32_t)(sEphemeralKeyTimeout / 1000UL));
        sEpskcActive = true;
        break;

    case OT_BORDER_AGENT_STATE_CONNECTED:
        // connected to
        break;

    case OT_BORDER_AGENT_STATE_ACCEPTED:
        break;

    default:
        break;
    }
}
#endif // OPENTHREAD_CONFIG_BORDER_AGENT_EPHEMERAL_KEY_ENABLE
