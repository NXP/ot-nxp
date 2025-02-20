/*
 *  Copyright (c) 2024, The OpenThread Authors.
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
#include "ncp_cmd_ot.h"
#include "ncp_mbedtls_device.h"
#include <openthread/cli.h>

/* -------------------------------------------------------------------------- */
/*                             Private definitions                            */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

otError ProcessNcpLinkEncrypt(void *aContext, uint8_t aArgsLength, char *aArgs[])
{
    otError         error = OT_ERROR_NONE;
    NCP_CMD_ENCRYPT enc;
    int             arg = 0;

    memset((uint8_t *)&enc, 0, sizeof(enc));

    do
    {
        if (aArgsLength == 0)
        {
            error = OT_ERROR_INVALID_ARGS;
            break;
        }

        if (!strcmp(aArgs[arg], "1"))
        {
            otCliOutputFormat("Enable ncp encrypted communication\r\n");
            enc.action = NCP_CMD_ENCRYPT_ACTION_INIT;
            ncp_sys_encrypt(&enc);
        }
        else if (!strcmp(aArgs[arg], "0"))
        {
            otCliOutputFormat("Disable ncp encrypted communication\r\n");
            enc.action = NCP_CMD_ENCRYPT_ACTION_STOP;
            ncp_sys_encrypt(&enc);
        }
        else if (!strcmp(aArgs[arg], "help"))
        {
            otCliOutputFormat("Usage:\r\n");
            otCliOutputFormat("\tncp-sys-encrypt <mode>\r\n");
            otCliOutputFormat("\r\n");
            otCliOutputFormat("This command is used to enable/disable ncp encrypted communication.\r\n");
            otCliOutputFormat("\r\n");
            otCliOutputFormat("mode:\r\n");
            otCliOutputFormat("\t0 - Disable ncp encrypted communication\r\n");
            otCliOutputFormat("\t1 - Enable ncp encrypted communication\r\n");
        }
        else
        {
            error = OT_ERROR_INVALID_ARGS;
            break;
        }
    } while (false);

    return error;
}
