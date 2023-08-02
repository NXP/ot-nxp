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
#include "fsl_common.h"

#include <openthread-core-config.h>
#include <openthread/cli.h>
#include <openthread/error.h>
#include <openthread/instance.h>
#include "common/logging.hpp"

#include "ot_platform_common.h"
#include "radio_cli.h"
#include <openthread/platform/radio.h>
#include "lib/spinel/spinel.h"
/* -------------------------------------------------------------------------- */
/*                             Private definitions                            */
/* -------------------------------------------------------------------------- */
#define MFG_CMD_ACTION_GET 0
#define MFG_CMD_ACTION_SET 1

#define MFG_CMD_GET_SET_CHANNEL 0x0b       // 11
#define MFG_CMD_GET_SET_TXPOWER 0x0f       // 15
#define MFG_CMD_CONTINUOUS_TX 0x11         // 17
#define MFG_CMD_GET_SET_PAYLOAD_SIZE 0x14  // 20
#define MFG_CMD_GET_RX_RESULT 0x1f         // 31
#define MFG_CMD_START_RX_TEST 0x20         // 32
#define MFG_CMD_BURST_TX 0x21              // 33
#define MFG_CMD_DUTY_CYCLE_TX 0x23         // 35
#define MFG_CMD_GET_SET_CCA_THRESHOLD 0x2F // 47
#define MFG_CMD_CONTINOUS_CCA_TEST 0X31    // 49
#define MFG_CMD_GET_CCA_STATUS 0x32        // 50
#define MFG_CMD_CONTINOUS_ED_TEST 0x37     // 55
#define MFG_CMD_GET_ED_VALUE 0x38          // 56
#define MFG_CMD_PHY_TX_TEST_PSDU 0x39      // 57
#define MFG_CMD_PHY_RX_TX_ACK_TEST 0x3A    // 58
#define MFG_CMD_SET_GENERIC_PARAM 0x3B     // 59
#define MAX_VERSION_STRING_SIZE 128        //< Max size of version string.

// NXP Spinel commands
// Independent Reset Properties range [0x100 - 0X110]

// IR CONFIG <uint8 mode> : Configure IR Mode (O:Disable 1:OutOfBand 2:InBand)
#define SPINEL_PROP_VENDOR_NXP_IR_CONFIG (SPINEL_PROP_VENDOR__BEGIN + 0x100)

// IR CMD : Execute Independent Reset without noticed (no arg)
#define SPINEL_PROP_VENDOR_NXP_IR_CMD (SPINEL_PROP_VENDOR__BEGIN + 0x101)

// SET IEEE.802.15.4 MAC Address <uint64> : Set MAC address
#define SPINEL_PROP_VENDOR_NXP_SET_EUI64_CMD (SPINEL_PROP_VENDOR__BEGIN + 0x10A)

// SET / GET TX Power Limit of 15.4 Transmissions
#define SPINEL_PROP_VENDOR_NXP_GET_SET_TXPOWERLIMIT_CMD (SPINEL_PROP_VENDOR__BEGIN + 0x10B)

// SET IEEE.802.15.4 CCA configuration
#define SPINEL_PROP_VENDOR_NXP_GET_SET_CCA_CONFIGURE_CMD (SPINEL_PROP_VENDOR__BEGIN + 0x10C)

// GET transceiver FW VERSION
#define SPINEL_PROP_VENDOR_NXP_GET_FW_VERSION_CMD (SPINEL_PROP_VENDOR__BEGIN + 0x10D)

// Manufacturing Properties range [0x3F0 - 0x3FF]
#define SPINEL_CMD_VENDOR_NXP_MFG (SPINEL_CMD_VENDOR__BEGIN + 0x3FF)

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

static otError ProcessIRCmd(void *aContext, uint8_t aArgsLength, char *aArgs[]);
static otError ProcessSetEui64(void *aContext, uint8_t aArgsLength, char *aArgs[]);
static otError ProcessTxPowerLimit(void *aContext, uint8_t aArgsLength, char *aArgs[]);
static otError ProcessMfgGetInt8(void *aContext, uint8_t cmdId, uint8_t aArgsLength);
static otError ProcessMfgSetInt8(void   *aContext,
                                 uint8_t cmdId,
                                 uint8_t aArgsLength,
                                 char   *aArgs[],
                                 int8_t  min,
                                 int8_t  max);
static otError ProcessMfgCommands(void *aContext, uint8_t aArgsLength, char *aArgs[]);
static otError ProcessGetSetCcaCfg(void *aContext, uint8_t aArgsLength, char *aArgs[]);
static otError ProcessGetFwVersion(void *aContext, uint8_t aArgsLength, char *aArgs[]);

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */
static uint8_t mfgEnable = 0;

static const otCliCommand radioCommands[] = {
    {"ircmd", ProcessIRCmd},             //=> InBand Independent Reset command
    {"seteui64", ProcessSetEui64},       //=> Set ieee.802.15.4 MAC Address
    {"txpwrlimit", ProcessTxPowerLimit}, //=> Set/Get TX power limit for 15.4
    {"mfgcmd", ProcessMfgCommands},      //=> Generic VSC for MFG RF commands
    {"ccacfg", ProcessGetSetCcaCfg},     //=> Set/Get CCA configuration for 802.15.4 CCA Before Tx operation
    {"fwversion", ProcessGetFwVersion},  //=> Get firmware version for 15.4
};

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

otError ProcessRadio(void *aContext, uint8_t aArgsLength, char *aArgs[])
{
    otError error  = OT_ERROR_NONE;
    int     n_cmds = (sizeof(radioCommands) / sizeof(radioCommands[0]));
    int     i      = 0;
    do
    {
        if (aArgsLength == 0)
        {
            error = OT_ERROR_INVALID_ARGS;
            break;
        }

        for (i = 0; i < n_cmds; i++)
        {
            if (!strcmp(radioCommands[i].mName, aArgs[0]))
            {
                error = radioCommands[i].mCommand(aContext, aArgsLength - 1, &aArgs[1]);
                break;
            }
        }
        if (i == n_cmds)
        {
            error = OT_ERROR_INVALID_ARGS;
        }
        break;
    } while (false);

    return error;
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

static otError ProcessIRCmd(void *aContext, uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aContext);
    otError error = OT_ERROR_INVALID_ARGS;

    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);
    otLogInfoPlat("ProcessIRCmd");
    error = otPlatResetOt();

    return error;
}

static otError ProcessSetEui64(void *aContext, uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aContext);
    otError error = OT_ERROR_INVALID_ARGS;

    otLogInfoPlat("ProcessSetEui64");

    if (aArgsLength == 1)
    {
        otExtAddress addr;
        char        *hex = *aArgs;

        otLogInfoPlat("+ seteui64 %s (len %ld)", *aArgs, (uint32_t)strlen(*aArgs));

        if ((hex[1] == 'x') && (strlen(*aArgs) == 18))
        {
            hex   = hex + 2;
            error = OT_ERROR_NONE;

            for (uint32_t i = 0; (i < 8) && (error == OT_ERROR_NONE); i++)
            {
                addr.m8[i] = 0;
                for (uint32_t k = 0; k < 2; k++)
                {
                    // get current character then increment
                    uint8_t byte = *hex++;
                    // transform hex character to the 4bit equivalent number, using the ascii table indexes
                    if (byte >= '0' && byte <= '9')
                        byte = byte - '0';
                    else if (byte >= 'a' && byte <= 'f')
                        byte = byte - 'a' + 10;
                    else if (byte >= 'A' && byte <= 'F')
                        byte = byte - 'A' + 10;
                    else
                    {
                        error = OT_ERROR_INVALID_ARGS;
                        break;
                    }
                    // shift 4 to make space for new digit, and add the 4 bits of the new digit
                    addr.m8[i] = (addr.m8[i] << 4) | (byte & 0xF);
                }
            }

            if (error == OT_ERROR_NONE)
            {
                uint64_t addr64t = 0;

                for (size_t i = 0; i < sizeof(addr64t); i++)
                {
                    addr64t |= ((uint64_t)addr.m8[sizeof(addr64t) - 1 - i]) << (i * 8);
                }

                error = otPlatRadioSendSetPropVendorUint64Cmd(SPINEL_PROP_VENDOR_NXP_SET_EUI64_CMD, addr64t);
            }
        }
    }

    return error;
}

static otError ProcessTxPowerLimit(void *aContext, uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aContext);
    otError error        = OT_ERROR_INVALID_ARGS;
    uint8_t txPowerLimit = 0;

    otLogInfoPlat("TxPowerLimit");

    if (aArgsLength == 1) // set tx power limit
    {
        txPowerLimit = (uint8_t)atoi(aArgs[0]);

        if ((txPowerLimit >= 1) && (txPowerLimit <= OT_NXP_PLAT_TX_PWR_LIMIT_MAX))
        {
            otLogInfoPlat("Set TX power limit: %d", txPowerLimit);
            error = otPlatRadioSendSetPropVendorUint8Cmd(SPINEL_PROP_VENDOR_NXP_GET_SET_TXPOWERLIMIT_CMD, txPowerLimit);
        }
        else
        {
            otLogInfoPlat("The TX power limit set is out of range");
        }
    }
    else if (aArgsLength == 0) // get tx power limit
    {
        error = otPlatRadioSendGetPropVendorUint8Cmd(SPINEL_PROP_VENDOR_NXP_GET_SET_TXPOWERLIMIT_CMD, &txPowerLimit);
        otLogInfoPlat("Get TX power limit: %d", txPowerLimit);

        otCliOutputFormat("%d\r\n", txPowerLimit);
    }

    return error;
}

static otError ProcessMfgGetInt8(void *aContext, uint8_t cmdId, uint8_t aArgsLength)
{
    otError error       = OT_ERROR_INVALID_ARGS;
    uint8_t outputLen   = 12;
    uint8_t payload[12] = {11};
    uint8_t payloadLen  = 12;

    if (aArgsLength == 1)
    {
        payload[1] = cmdId;
        payload[2] = MFG_CMD_ACTION_GET;

        otPlatRadioMfgCommand(aContext, SPINEL_CMD_VENDOR_NXP_MFG, (uint8_t *)payload, payloadLen, &outputLen);

        if ((outputLen >= 5) && (payload[3] == 0))
        {
            if ((cmdId == MFG_CMD_GET_SET_TXPOWER) && OT_NXP_PLAT_TX_PWR_HALF_DBM)
            {
                otCliOutputFormat("%d\r\n", ((int8_t)payload[4]) / 2);
            }
            else
            {
                otCliOutputFormat("%d\r\n", (int8_t)payload[4]);
            }
            error = OT_ERROR_NONE;
        }
        else
        {
            error = OT_ERROR_FAILED;
        }
    }

    return error;
}

static otError ProcessMfgSetInt8(void   *aContext,
                                 uint8_t cmdId,
                                 uint8_t aArgsLength,
                                 char   *aArgs[],
                                 int8_t  min,
                                 int8_t  max)
{
    otError error       = OT_ERROR_INVALID_ARGS;
    uint8_t outputLen   = 12;
    uint8_t payload[12] = {11};
    uint8_t payloadLen  = 12;
    int8_t  setValue    = 0;

    if (aArgsLength == 2)
    {
        setValue = (int8_t)atoi(aArgs[1]);
        if ((setValue >= min) && (setValue <= max))
        {
            payload[1] = cmdId;
            payload[2] = MFG_CMD_ACTION_SET;
            payload[4] = (uint8_t)setValue;

            if ((cmdId == MFG_CMD_GET_SET_TXPOWER) && OT_NXP_PLAT_TX_PWR_HALF_DBM)
            {
                payload[4] = ((uint8_t)setValue) << 1; // convert dBm to half dBm
            }
            else
            {
                payload[4] = (uint8_t)setValue;
            }

            otPlatRadioMfgCommand(aContext, SPINEL_CMD_VENDOR_NXP_MFG, (uint8_t *)payload, payloadLen, &outputLen);

            if ((outputLen >= 4) && (payload[3] == 0))
            {
                error = OT_ERROR_NONE;
            }
            else
            {
                error = OT_ERROR_FAILED;
            }
        }
    }

    return error;
}

static otError ProcessMfgCommands(void *aContext, uint8_t aArgsLength, char *aArgs[])
{
    uint8_t payload[12] = {11};
    uint8_t payloadLen  = 12;
    uint8_t outputLen   = 12;
    otError error       = OT_ERROR_INVALID_ARGS;
    uint8_t cmdId, idx;

    do
    {
        if (aArgsLength == 1)
        {
            cmdId = (uint8_t)atoi(aArgs[0]);
            if ((cmdId == 0) || (cmdId == 1))
            {
                mfgEnable = cmdId;
                otLogInfoPlat("MFG command SUCCESS");
                error = OT_ERROR_NONE;
                break;
            }
        }

        if (mfgEnable == 0)
        {
            otLogInfoPlat("MFG command not enabled");
            otCliOutputFormat("MFG command not enabled. to enable it : radio_nxp mfgcmd 1\r\n");
            error = OT_ERROR_FAILED;
            break;
        }

        if ((aArgsLength > 0) && (mfgEnable == 1))
        {
            cmdId = (uint8_t)atoi(aArgs[0]);

            switch (cmdId)
            {
            case MFG_CMD_GET_SET_CHANNEL: // get channel
                error = ProcessMfgGetInt8(aContext, MFG_CMD_GET_SET_CHANNEL, aArgsLength);
                break;

            case MFG_CMD_GET_SET_CHANNEL + 1: // set channel
                error = ProcessMfgSetInt8(aContext, MFG_CMD_GET_SET_CHANNEL, aArgsLength, aArgs, 11, 26);
                break;

            case MFG_CMD_GET_SET_TXPOWER: // get txpower
                error = ProcessMfgGetInt8(aContext, MFG_CMD_GET_SET_TXPOWER, aArgsLength);
                break;

            case MFG_CMD_GET_SET_TXPOWER + 1: // set txpower
                error = ProcessMfgSetInt8(aContext, MFG_CMD_GET_SET_TXPOWER, aArgsLength, aArgs, -20, 22);
                break;

            case MFG_CMD_CONTINUOUS_TX:
                error = ProcessMfgSetInt8(aContext, MFG_CMD_CONTINUOUS_TX, aArgsLength, aArgs, 0, 1);
                break;

            case MFG_CMD_GET_SET_PAYLOAD_SIZE: // get
                error = ProcessMfgGetInt8(aContext, MFG_CMD_GET_SET_PAYLOAD_SIZE, aArgsLength);
                break;

            case MFG_CMD_GET_SET_PAYLOAD_SIZE + 1: // set
                error = ProcessMfgSetInt8(aContext, MFG_CMD_GET_SET_PAYLOAD_SIZE, aArgsLength, aArgs, 0, 127);
                // actual limits are set in MFG function and error is return in case of wrong parameter
                break;

            case MFG_CMD_GET_RX_RESULT:
            {
                if (aArgsLength == 1)
                {
                    payload[1] = MFG_CMD_GET_RX_RESULT;
                    payload[2] = MFG_CMD_ACTION_GET;
                    otPlatRadioMfgCommand(aContext, SPINEL_CMD_VENDOR_NXP_MFG, (uint8_t *)payload, payloadLen,
                                          &outputLen);
                    if (outputLen >= 11)
                    {
                        otCliOutputFormat("status : %d\r\n", payload[4]);
                        otCliOutputFormat("rx_pkt_count : %d\r\n", payload[5] | (payload[6] << 8));
                        otCliOutputFormat("total_pkt_count : %d\r\n", payload[7] | (payload[8] << 8));
                        otCliOutputFormat("rssi : %d\r\n", (int8_t)payload[9]);
                        otCliOutputFormat("lqi : %d\r\n", payload[10]);
                        error = OT_ERROR_NONE;
                    }
                    else
                    {
                        error = OT_ERROR_FAILED;
                    }
                }
            }
            break;

            case MFG_CMD_START_RX_TEST:
            {
                if (aArgsLength == 1)
                {
                    payload[1] = MFG_CMD_START_RX_TEST;
                    otPlatRadioMfgCommand(aContext, SPINEL_CMD_VENDOR_NXP_MFG, (uint8_t *)payload, payloadLen,
                                          &outputLen);
                    error = OT_ERROR_NONE;
                }
            }
            break;

            case MFG_CMD_BURST_TX:
            {
                uint8_t mode = 0;
                if (aArgsLength == 3)
                {
                    mode = (uint8_t)atoi(aArgs[1]);
                    if (mode < 8)
                    {
                        payload[1] = MFG_CMD_BURST_TX;
                        payload[4] = mode;
                        payload[5] = (uint8_t)atoi(aArgs[2]);
                        otPlatRadioMfgCommand(aContext, SPINEL_CMD_VENDOR_NXP_MFG, (uint8_t *)payload, payloadLen,
                                              &outputLen);
                        error = OT_ERROR_NONE;
                    }
                }
            }
            break;

            case MFG_CMD_DUTY_CYCLE_TX:
                error = ProcessMfgSetInt8(aContext, MFG_CMD_DUTY_CYCLE_TX, aArgsLength, aArgs, 0, 1);
                break;

            case MFG_CMD_GET_SET_CCA_THRESHOLD: // get
                error = ProcessMfgGetInt8(aContext, MFG_CMD_GET_SET_CCA_THRESHOLD, aArgsLength);
                break;

            case MFG_CMD_GET_SET_CCA_THRESHOLD + 1: // set
                error = ProcessMfgSetInt8(aContext, MFG_CMD_GET_SET_CCA_THRESHOLD, aArgsLength, aArgs, -110, 0);
                break;

            case MFG_CMD_CONTINOUS_CCA_TEST:
            {
                if (aArgsLength == 3)
                {
                    payload[1] = MFG_CMD_CONTINOUS_CCA_TEST;
                    payload[2] = MFG_CMD_ACTION_SET;
                    payload[4] = (uint8_t)atoi(aArgs[1]);
                    payload[5] = (uint8_t)atoi(aArgs[2]);
                    otPlatRadioMfgCommand(aContext, SPINEL_CMD_VENDOR_NXP_MFG, (uint8_t *)payload, payloadLen,
                                          &outputLen);
                    if ((outputLen >= 4) && (payload[3] == 0))
                    {
                        error = OT_ERROR_NONE;
                    }
                    else
                    {
                        error = OT_ERROR_FAILED;
                    }
                }
            }
            break;

            case MFG_CMD_GET_CCA_STATUS: // get
                error = ProcessMfgGetInt8(aContext, MFG_CMD_GET_CCA_STATUS, aArgsLength);
                break;

            case MFG_CMD_CONTINOUS_ED_TEST:
                error = ProcessMfgSetInt8(aContext, MFG_CMD_CONTINOUS_ED_TEST, aArgsLength, aArgs, -127, 127);
                break;

            case MFG_CMD_GET_ED_VALUE:
                error = ProcessMfgGetInt8(aContext, MFG_CMD_GET_ED_VALUE, aArgsLength);
                break;

            case MFG_CMD_PHY_TX_TEST_PSDU:
            {
                uint8_t count_opt, gap, ackEnable;
                if (aArgsLength == 4)
                {
                    payload[1] = MFG_CMD_PHY_TX_TEST_PSDU;
                    payload[2] = MFG_CMD_ACTION_SET;

                    count_opt = (uint8_t)atoi(aArgs[1]);
                    gap       = (uint8_t)atoi(aArgs[2]);
                    ackEnable = (uint8_t)atoi(aArgs[3]);
                    if ((count_opt < 8) && (gap > 5) && (ackEnable < 2))
                    {
                        payload[4] = count_opt;
                        payload[5] = gap;
                        payload[6] = ackEnable;
                        otPlatRadioMfgCommand(aContext, SPINEL_CMD_VENDOR_NXP_MFG, (uint8_t *)payload, payloadLen,
                                              &outputLen);
                        if ((outputLen >= 5) && (payload[3] == 0))
                        {
                            error = OT_ERROR_NONE;
                        }
                        else
                        {
                            error = OT_ERROR_FAILED;
                        }
                    }
                    else
                    {
                        error = OT_ERROR_FAILED;
                    }
                }
            }
            break;

            case MFG_CMD_PHY_RX_TX_ACK_TEST:
                error = ProcessMfgSetInt8(aContext, MFG_CMD_PHY_RX_TX_ACK_TEST, aArgsLength, aArgs, 0, 1);
                break;

            case MFG_CMD_SET_GENERIC_PARAM:
            {
                uint16_t panid, destaddr, srcaddr;
                if (aArgsLength == 5)
                {
                    panid    = (uint16_t)strtol(aArgs[2], NULL, 16);
                    destaddr = (uint16_t)strtol(aArgs[3], NULL, 16);
                    srcaddr  = (uint16_t)strtol(aArgs[4], NULL, 16);

                    payload[1]  = MFG_CMD_SET_GENERIC_PARAM;
                    payload[2]  = MFG_CMD_ACTION_SET;
                    payload[4]  = (uint8_t)atoi(aArgs[1]);           // SEQ_NUM
                    payload[5]  = (uint8_t)(panid & 0xFF);           // PAN ID LSB
                    payload[6]  = (uint8_t)((panid >> 8) & 0xFF);    // PAN ID MSB
                    payload[7]  = (uint8_t)(destaddr & 0xFF);        // DEST ADDR LSB
                    payload[8]  = (uint8_t)((destaddr >> 8) & 0xFF); // DEST ADDR MSB
                    payload[9]  = (uint8_t)(srcaddr & 0xFF);         // SRC ADDR LSB
                    payload[10] = (uint8_t)((srcaddr >> 8) & 0xFF);  // SRC ADDR MSB

                    otPlatRadioMfgCommand(aContext, SPINEL_CMD_VENDOR_NXP_MFG, (uint8_t *)payload, payloadLen,
                                          &outputLen);
                    if ((outputLen >= 5) && (payload[3] == 0))
                    {
                        error = OT_ERROR_NONE;
                    }
                    else
                    {
                        error = OT_ERROR_FAILED;
                    }
                }
            }
            break;

            default:
                error = OT_ERROR_NOT_IMPLEMENTED;
                break;
            }
        }

        // HANDLE ERRORS
        if (error == OT_ERROR_NONE)
        {
            otLogInfoPlat("MFG command SUCCESS");
        }
        else if (aArgsLength == payloadLen)
        {
            // If user passed all the payload, this means this is a direct message for the RCP.
            // Send it and print the return results.
            for (idx = 0; idx < payloadLen; idx++)
            {
                payload[idx] = (uint8_t)atoi(aArgs[idx]);
            }
            otPlatRadioMfgCommand(aContext, SPINEL_CMD_VENDOR_NXP_MFG, (uint8_t *)payload, payloadLen, &outputLen);
            for (idx = 0; idx < outputLen; idx++)
            {
                otCliOutputFormat("%d ", payload[idx]);
            }
            otCliOutputFormat("\r\n");
            error = OT_ERROR_NONE;
            otLogInfoPlat("MFG command SUCCESS");
        }
        else if (error == OT_ERROR_INVALID_ARGS)
        {
            otLogInfoPlat("MFG command Invalid parameter");
        }
        else if (error == OT_ERROR_NOT_IMPLEMENTED)
        {
            otLogInfoPlat("MFG command not implemented");
        }
        else
        {
            otLogInfoPlat("MFG command FAILED");
        }
    } while (false);

    return error;
}

static otError ProcessGetSetCcaCfg(void *aContext, uint8_t aArgsLength, char *aArgs[])
{
    otError         error = OT_ERROR_INVALID_ARGS;
    otCCAModeConfig aCcaCfg;

    if (aArgsLength == 4) // set cca configuration
    {
        aCcaCfg.mCcaMode            = (uint8_t)strtol(aArgs[0], NULL, 16);
        aCcaCfg.mCca1Threshold      = (uint8_t)strtol(aArgs[1], NULL, 16);
        aCcaCfg.mCca2CorrThreshold  = (uint8_t)strtol(aArgs[2], NULL, 16);
        aCcaCfg.mCca2MinNumOfCorrTh = (uint8_t)strtol(aArgs[3], NULL, 16);
        if ((((aCcaCfg.mCcaMode >= 1) && (aCcaCfg.mCcaMode <= 4)) || (aCcaCfg.mCcaMode == 0xFF)) &&
            (aCcaCfg.mCca2MinNumOfCorrTh <= 6))
        {
            error = otPlatRadioCcaConfigValue(SPINEL_PROP_VENDOR_NXP_GET_SET_CCA_CONFIGURE_CMD, &aCcaCfg, 0x1);
        }
    }
    else if (aArgsLength == 0) // get cca configuration
    {
        error = otPlatRadioCcaConfigValue(SPINEL_PROP_VENDOR_NXP_GET_SET_CCA_CONFIGURE_CMD, &aCcaCfg, 0x0);

        otCliOutputFormat("CCA Configuration:\r\n");
        otCliOutputFormat(
            "CCA Mode type [CCA1=1, CCA2=2, CCA3=3[CCA1 AND CCA2], CCA3=4[CCA1 OR CCA2], NoCCA=0xFF], : 0x%x\r\n",
            aCcaCfg.mCcaMode);
        otCliOutputFormat("CCA1 Threshold Value : 0x%x\r\n", aCcaCfg.mCca1Threshold);
        otCliOutputFormat("CCA2 Correlation Threshold Value : 0x%x\r\n", aCcaCfg.mCca2CorrThreshold);
        otCliOutputFormat("CCA2 Minimim Number of Correlation Threshold Value : 0x%x\r\n", aCcaCfg.mCca2MinNumOfCorrTh);
    }
    else
    {
        otCliOutputFormat("CCA configuration FAILED! Invalid input arg\r\n \
                           Format: ccacfg <CcaMode> <Cca1Threshold> \
                           <Cca2CorrThreshold> <Cca2MinNumOfCorrTh>\r\n \
                           CcaMode: CCA Mode type [CCA1=1, CCA2=2, CCA3=3[CCA1 AND CCA2], CCA3=4[CCA1 OR CCA2], NoCCA=0xFF]\r\n \
                           Cca1Threshold[1Byte Hex value]: Energy threshold for CCA Mode1\r\n \
                           Cca2CorrThreshold[1Byte Hex value]: CCA Mode 2 Correlation Threshold\r\n \
                           Cca2MinNumOfCorrTh: [0 to 6]\r\n");
    }

    return error;
}

static otError ProcessGetFwVersion(void *aContext, uint8_t aArgsLength, char *aArgs[])
{
    otError error = OT_ERROR_INVALID_ARGS;
    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aArgs);

    if (aArgsLength == 0)
    {
        const char version[MAX_VERSION_STRING_SIZE] = {0};
        error = otPlatRadioSendGetPropVendorCmd(SPINEL_PROP_VENDOR_NXP_GET_FW_VERSION_CMD, version,
                                                MAX_VERSION_STRING_SIZE);
        if (error == OT_ERROR_NONE)
        {
            otCliOutputFormat("%s\r\n", version);
        }
    }
    return error;
}
