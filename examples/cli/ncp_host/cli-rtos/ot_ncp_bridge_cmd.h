/*
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NCP_BRIDGE_CMD_H__
#define __NCP_BRIDGE_CMD_H__

#define NCP_BRIDGE_CMD_SIZE_LOW_BYTES 4
#define NCP_BRIDGE_CMD_SIZE_HIGH_BYTES 5
#define NCP_BRIDGE_CMD_SEQUENCE_LOW_BYTES 6
#define NCP_BRIDGE_CMD_SEQUENCE_HIGH_BYTES 7

#define NCP_BRIDGE_SEND_DATA_INBUF_SIZE 1600
#define NCP_BRIDGE_INBUF_SIZE 4096
#define NCP_BRIDGE_CMD_HEADER_LEN sizeof(NCP_BRIDGE_COMMAND)
#define NCP_BRIDGE_TLV_HEADER_LEN sizeof(TypeHeader_t)

/* Get command class/subclass/cmd_id/tlv */
#define GET_CMD_CLASS(cmd) (((cmd)&0xff000000) >> 24)
#define GET_CMD_SUBCLASS(cmd) (((cmd)&0x00ff0000) >> 16)
#define GET_CMD_ID(cmd) ((cmd)&0x0000ffff)
#define GET_CMD_TLV(cmd) \
    (((cmd)->size == NCP_BRIDGE_CMD_HEADER_LEN) ? NULL : (uint8_t *)((uint8_t *)(cmd) + NCP_BRIDGE_CMD_HEADER_LEN))
#define GET_CMD_TLV_LEN(cmd) ((cmd)->size - NCP_BRIDGE_CMD_HEADER_LEN)

/*NCP Bridge command class*/
#define NCP_BRIDGE_CMD_WLAN 0x00
#define NCP_BRIDGE_CMD_BLE 0x01
#define NCP_BRIDGE_CMD_15D4 0x02
#define NCP_BRIDGE_CMD_MATTER 0x03
#define NCP_BRIDGE_CMD_SYSTEM 0x04

#define NCP_BRIDGE_CMD_INVALID 0xFFFFFFFF

#define NCP_15d4_CMD_FORWARD 0x00010000

/*NCP Bridge Message Type*/
#define NCP_BRIDGE_MSG_TYPE_CMD 0x0000
#define NCP_BRIDGE_MSG_TYPE_RESP 0x0001
#define NCP_BRIDGE_MSG_TYPE_EVENT 0x0002

/*NCP Bridge CMD response state*/
/*General result code ok*/
#define NCP_BRIDGE_CMD_RESULT_OK 0x0000
/*General error*/
#define NCP_BRIDGE_CMD_RESULT_ERROR 0x0001
/*NCP Bridge Command is not valid*/
#define NCP_BRIDGE_CMD_RESULT_NOT_SUPPORT 0x0002
/*NCP Bridge Command is pending*/
#define NCP_BRIDGE_CMD_RESULT_PENDING 0x0003
/*System is busy*/
#define NCP_BRIDGE_CMD_RESULT_BUSY 0x0004
/*Data buffer is not big enough*/
#define NCP_BRIDGE_CMD_RESULT_PARTIAL_DATA 0x0005

#define CMD_SYNC 0
#define CMD_ASYNC 1

#define NCP_HASH_TABLE_SIZE 64
#define NCP_HASH_INVALID_KEY (uint8_t)(-1)

/*NCP Bridge command header*/
typedef struct bridge_command_header
{
    /*bit0 ~ bit15 cmd id  bit16 ~ bit23 cmd subclass bit24 ~ bit31 cmd class*/
    uint32_t cmd;
    uint16_t size;
    uint16_t seqnum;
    uint16_t result;
    uint16_t msg_type;
} NCP_BRIDGE_COMMAND, NCP_BRIDGE_RESPONSE;

struct cmd_t
{
    uint32_t    cmd;
    const char *help;
    int (*handler)(void *tlv, int len);
    /* The field `async' is:
     * CMD_SYNC   (or 0) if the command is executed synchronously
     * CMD_ASYNC  (or 1) if the command is executed asynchronously
     */
    bool async;
};

struct cmd_subclass_t
{
    uint32_t      cmd_subclass;
    struct cmd_t *cmd;
    /* Mapping of subclass list */
    uint8_t hash[NCP_HASH_TABLE_SIZE];
};

struct cmd_class_t
{
    uint32_t               cmd_class;
    struct cmd_subclass_t *cmd_subclass;
    /* Length of subclass list */
    uint16_t subclass_len;
    /* Mapping of cmd list */
    uint8_t hash[NCP_HASH_TABLE_SIZE];
};

typedef struct
{
    uint32_t block_type;
    uint32_t command_sz;
    void    *cmd_buff;
} ot_ncp_command_t;

#endif
