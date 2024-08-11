/* @file ncp_cmd_ot.h
 *
 *  @brief This file contains ncp command/response/event definitions
 *
 *  Copyright 2008-2024 NXP
 *
 *  Licensed under the LA_OPT_NXP_Software_License.txt (the "Agreement")
 */

#ifndef __NCP_CMD_OT_H__
#define __NCP_CMD_OT_H__

#include "ncp_cmd_common.h"

/*OT NCP subclass*/
#define NCP_15d4_CMD_FORWARD 0x00100000
#define NCP_CMD_OT_OTHER 0x00f00000

/*NCP Command definitions*/
#define NCP_OT_CMD_FORWARD (NCP_CMD_15D4 | NCP_15d4_CMD_FORWARD | NCP_MSG_TYPE_RESP | 0x00000001)
#define NCP_CMD_INVALID_CMD (NCP_CMD_15D4 | NCP_CMD_OT_OTHER | NCP_MSG_TYPE_RESP | 0x00000001)

/* System NCP subclass */
#define NCP_CMD_SYSTEM_POWERMGMT 0x00200000
#define NCP_CMD_SYSTEM_ASYNC_EVENT 0x00300000

/* System Configure command */
#define NCP_CMD_SYSTEM_POWERMGMT_MCU_SLEEP_CFM (NCP_CMD_15D4 | NCP_CMD_SYSTEM_POWERMGMT | NCP_MSG_TYPE_CMD | 0x00000004)
#define NCP_RSP_SYSTEM_POWERMGMT_MCU_SLEEP_CFM \
    (NCP_CMD_15D4 | NCP_CMD_SYSTEM_POWERMGMT | NCP_MSG_TYPE_RESP | 0x00000004)

#define NCP_EVENT_MCU_SLEEP_ENTER (NCP_CMD_15D4 | NCP_CMD_SYSTEM_ASYNC_EVENT | NCP_MSG_TYPE_EVENT | 0x00000001)
#define NCP_EVENT_MCU_SLEEP_EXIT (NCP_CMD_15D4 | NCP_CMD_SYSTEM_ASYNC_EVENT | NCP_MSG_TYPE_EVENT | 0x00000002)

#define OT_COMMANDS_MAX_LEN 640

typedef NCP_TLV_PACK_START struct _NCPCmd_DS_COMMAND
{
    /** Command Header : Command */
    NCP_COMMAND header;
    /** Command Body */
    uint8_t ncp_params[OT_COMMANDS_MAX_LEN];
} NCP_TLV_PACK_END NCPCmd_DS_COMMAND;

#endif /* __NCP_CMD_OT_H__ */
