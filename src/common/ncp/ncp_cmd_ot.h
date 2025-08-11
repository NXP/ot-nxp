/* @file ncp_cmd_ot.h
 *
 *  @brief This file contains ncp command/response/event definitions
 *
 *  Copyright 2008-2025 NXP
 *
 *  Licensed under the LA_OPT_NXP_Software_License.txt (the "Agreement")
 */

#ifndef __NCP_CMD_OT_H__
#define __NCP_CMD_OT_H__

#include "ncp_cmd_common.h"

/*OT NCP subclass*/
#define NCP_15d4_CMD_FORWARD 0x00100000
#define NCP_15d4_CMD_MATTER 0x00200000
#define NCP_CMD_OT_OTHER 0x00f00000

/*NCP Command definitions*/
#define NCP_OT_CMD_FORWARD (NCP_CMD_15D4 | NCP_15d4_CMD_FORWARD | NCP_MSG_TYPE_RESP | 0x00000001)
#define NCP_CMD_INVALID_CMD (NCP_CMD_15D4 | NCP_CMD_OT_OTHER | NCP_MSG_TYPE_RESP | 0x00000001)
#define NCP_OT_CMD_MATTER (NCP_CMD_15D4 | NCP_15d4_CMD_MATTER | NCP_MSG_TYPE_RESP | 0x00000001)

/* System NCP subclass */
/** subclass type for system configure */
#define NCP_CMD_SYSTEM_CONFIG 0x00000000
/** subclass type for system test */
#define NCP_CMD_SYSTEM_TEST 0x00100000
/** subclass type for system power managerment */
#define NCP_CMD_SYSTEM_POWERMGMT 0x00200000
/** subclass type for system asynchronous event */
#define NCP_CMD_SYSTEM_ASYNC_EVENT 0x00300000

/* System Configure command */
#define NCP_CMD_SYSTEM_POWERMGMT_MCU_SLEEP_CFM (NCP_CMD_15D4 | NCP_CMD_SYSTEM_POWERMGMT | NCP_MSG_TYPE_CMD | 0x00000004)
#define NCP_RSP_SYSTEM_POWERMGMT_MCU_SLEEP_CFM \
    (NCP_CMD_15D4 | NCP_CMD_SYSTEM_POWERMGMT | NCP_MSG_TYPE_RESP | 0x00000004)

/** system configuration encrypted communication command ID */
#define NCP_CMD_SYSTEM_CONFIG_ENCRYPT \
    (NCP_CMD_15D4 | NCP_CMD_SYSTEM_CONFIG | NCP_MSG_TYPE_CMD | 0x00000003) /* ncp_encrypt */
/** system configuration encrypted communication command response ID */
#define NCP_RSP_SYSTEM_CONFIG_ENCRYPT (NCP_CMD_15D4 | NCP_CMD_SYSTEM_CONFIG | NCP_MSG_TYPE_RESP | 0x00000003)

#define NCP_EVENT_MCU_SLEEP_ENTER (NCP_CMD_15D4 | NCP_CMD_SYSTEM_ASYNC_EVENT | NCP_MSG_TYPE_EVENT | 0x00000001)
#define NCP_EVENT_MCU_SLEEP_EXIT (NCP_CMD_15D4 | NCP_CMD_SYSTEM_ASYNC_EVENT | NCP_MSG_TYPE_EVENT | 0x00000002)

/** NCP host device encrypted communication event ID */
#define NCP_EVENT_SYSTEM_ENCRYPT (NCP_CMD_15D4 | NCP_CMD_SYSTEM_CONFIG | NCP_MSG_TYPE_EVENT | 0x00000003)
/** NCP host device encrypted communication stop event ID */
#define NCP_EVENT_SYSTEM_ENCRYPT_STOP (NCP_CMD_15D4 | NCP_CMD_SYSTEM_CONFIG | NCP_MSG_TYPE_EVENT | 0x00000004)

#define NCP_CMD_ENCRYPT_ACTION_INIT 0
#define NCP_CMD_ENCRYPT_ACTION_DATA 1
#define NCP_CMD_ENCRYPT_ACTION_VERIFY 2
#define NCP_CMD_ENCRYPT_ACTION_STOP 3

#define OT_COMMANDS_MAX_LEN 640

/** NCP host device encrypted communication . */
typedef NCP_TLV_PACK_START struct _NCP_CMD_ENCRYPT
{
    /**
    0: trigger encrypted communication flow
    1: send handshake data to NCP device
    2: verify the encryption communication
    */
    uint8_t action;
    /**
    checksum of keys and IVs when action is 2
    */
    uint32_t arg;
} NCP_TLV_PACK_END NCP_CMD_ENCRYPT;

typedef NCP_TLV_PACK_START struct _NCPCmd_DS_COMMAND
{
    /** Command Header : Command */
    NCP_COMMAND header;
    /** Command Body */
    uint8_t ncp_params[OT_COMMANDS_MAX_LEN];
} NCP_TLV_PACK_END NCPCmd_DS_COMMAND;

/** NCP system command */
typedef NCP_TLV_PACK_START struct _NCPCmd_DS_SYS_COMMAND
{
    /** Command Header : Command */
    NCP_COMMAND header;
    /** Command Body */
    union
    {
        /** NCP host and device encrypted communication. */
        NCP_CMD_ENCRYPT encrypt;
    } params;
} NCP_TLV_PACK_END NCPCmd_DS_SYS_COMMAND, MCU_NCPCmd_DS_SYS_COMMAND;

#endif /* __NCP_CMD_OT_H__ */
