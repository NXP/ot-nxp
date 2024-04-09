/* @file ncp_cmd_ot.h
 *
 *  @brief This file contains ncp bridge command/response/event definitions
 *
 *  Copyright 2008-2024 NXP
 *
 *  Licensed under the LA_OPT_NXP_Software_License.txt (the "Agreement")
 */

#ifndef __NCP_CMD_OT_H__
#define __NCP_CMD_OT_H__

#include "ncp_cmd_common.h"

/*OT NCP Bridge subclass*/
#define NCP_15d4_CMD_FORWARD 0x00010000
#define NCP_BRIDGE_CMD_OT_OTHER 0x00020000

/*NCP Bridge Command definitions*/
#define NCP_BRIDGE_OT_CMD_FORWARD                                                       \
    (NCP_BRIDGE_CMD_15D4 | NCP_15d4_CMD_FORWARD | 0x00000001) /* ot ncp forward command \
                                                               */
#define NCP_BRIDGE_CMD_INVALID_CMD (NCP_BRIDGE_CMD_15D4 | NCP_BRIDGE_CMD_OT_OTHER | 0x00000001)

#define OT_COMMANDS_MAX_LEN 640

typedef NCP_TLV_PACK_START struct _NCPCmd_DS_COMMAND
{
    /** Command Header : Command */
    NCP_BRIDGE_COMMAND header;
    /** Command Body */
    uint8_t ncp_params[OT_COMMANDS_MAX_LEN];
} NCP_TLV_PACK_END NCPCmd_DS_COMMAND;

#endif /* __NCP_CMD_OT_H__ */
