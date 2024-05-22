/*
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __OT_NCP_CMD_H__
#define __OT_NCP_CMD_H__

#define NCP_CMD_SIZE_LOW_BYTES 4
#define NCP_CMD_SIZE_HIGH_BYTES 5
#define NCP_CMD_SEQUENCE_LOW_BYTES 6
#define NCP_CMD_SEQUENCE_HIGH_BYTES 7

#define NCP_SEND_DATA_INBUF_SIZE 1600
#define NCP_INBUF_SIZE 4096
#define NCP_CMD_HEADER_LEN sizeof(NCP_COMMAND)
#define NCP_TLV_HEADER_LEN sizeof(TypeHeader_t)

/* Get command class/subclass/cmd_id/tlv */
#define GET_CMD_CLASS(cmd) (((cmd)&0xf0000000) >> 28)
#define GET_CMD_SUBCLASS(cmd) (((cmd)&0x0ff00000) >> 20)
#define GET_CMD_ID(cmd) ((cmd)&0x0000ffff)
#define GET_CMD_TLV(cmd) \
    (((cmd)->size == NCP_CMD_HEADER_LEN) ? NULL : (uint8_t *)((uint8_t *)(cmd) + NCP_CMD_HEADER_LEN))
#define GET_CMD_TLV_LEN(cmd) ((cmd)->size - NCP_CMD_HEADER_LEN)

/*NCP command class*/
#define NCP_CMD_WLAN 0x00000000
#define NCP_CMD_BLE 0x10000000
#define NCP_CMD_15D4 0x20000000
#define NCP_CMD_MATTER 0x30000000
#define NCP_CMD_SYSTEM 0x40000000

#define NCP_CMD_INVALID 0xFFFFFFFF

#define NCP_15d4_CMD_FORWARD 0x00100000

/*NCP Message Type*/
#define NCP_MSG_TYPE_CMD 0x00010000
#define NCP_MSG_TYPE_RESP 0x00020000
#define NCP_MSG_TYPE_EVENT 0x00030000

/*NCP CMD response state*/
/*General result code ok*/
#define NCP_CMD_RESULT_OK 0x0000
/*General error*/
#define NCP_CMD_RESULT_ERROR 0x0001
/*NCP Command is not valid*/
#define NCP_CMD_RESULT_NOT_SUPPORT 0x0002
/*NCP Command is pending*/
#define NCP_CMD_RESULT_PENDING 0x0003
/*System is busy*/
#define NCP_CMD_RESULT_BUSY 0x0004
/*Data buffer is not big enough*/
#define NCP_CMD_RESULT_PARTIAL_DATA 0x0005

#define CMD_SYNC 0
#define CMD_ASYNC 1

#define NCP_HASH_TABLE_SIZE 64
#define NCP_HASH_INVALID_KEY (uint8_t)(-1)

/*NCP command header*/
typedef struct command_header
{
    /*bit0 ~ bit15 cmd id,  bit16 ~ bit19 message type, bit20 ~ bit27 cmd subclass, bit28 ~ bit31 cmd class*/
    uint32_t cmd;
    uint16_t size;
    uint16_t seqnum;
    uint16_t result;
    uint16_t rsvd;
} NCP_COMMAND, NCP_RESPONSE;

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
