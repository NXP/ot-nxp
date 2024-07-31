/*
 *  Copyright (c) 2021, The OpenThread Authors.
 *  Copyright (c) 2022, 2024 NXP.
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
 *   This file implements the OpenThread platform abstraction for ncp communication.
 *
 */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include "ncp_ot.h"
#include "openthread-system.h"
#include <utils/uart.h>

/* -------------------------------------------------------------------------- */
/*                               Private macros                               */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

static volatile bool txDone = false;
static uint8_t       rxBuffer[OT_COMMANDS_MAX_LEN];

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void otPlatUartSetInstance(uint8_t newInstance)
{
    OT_UNUSED_VARIABLE(newInstance);
    return;
}

otError otPlatUartEnable(void)
{
    return OT_ERROR_NONE;
}

otError otPlatUartDisable(void)
{
    return OT_ERROR_NONE;
}

otError otPlatUartSend(const uint8_t *aBuf, uint16_t aBufLength)
{
    assert(aBuf != NULL);
    assert(aBufLength != 0);

    Copy_to_NCP_buff(aBuf, aBufLength);

    return OT_ERROR_NONE;
}

otError otPlatUartFlush(void)
{
    txDone = false;
    return OT_ERROR_NONE;
}

void otPlatCliUartProcess(void)
{
    uint16_t BytesRead = 0;
    uint32_t intMask;

    /* Get ot command from ot ncp task */
    if (ot_ncp_check_cmd_ready() == NCP_COMMAND_READY)
    {
        ot_ncp_copy_cmd_buff(rxBuffer, &BytesRead);
        otPlatUartReceived(rxBuffer, BytesRead);
        ot_ncp_clear_cmd_ready();
    }

    intMask = DisableGlobalIRQ();
    if (txDone == true)
    {
        otPlatUartSendDone();
        txDone = false;
    }
    EnableGlobalIRQ(intMask);
}

void Ot_Data_RxDone(void)
{
    /* notify the main loop that a RX buffer is available */
    otSysEventSignalPending();
}

void Ot_Data_TxDone(void)
{
    /* notify the main loop that the TX is done */
    txDone = true;
    otSysEventSignalPending();
}

/**
 * Week function in case the ot_cli is disabled
 *
 */
OT_TOOL_WEAK void otPlatUartSendDone(void)
{
}

OT_TOOL_WEAK void otPlatUartReceived(const uint8_t *aBuf, uint16_t aBufLength)
{
    OT_UNUSED_VARIABLE(aBuf);
    OT_UNUSED_VARIABLE(aBufLength);
}
