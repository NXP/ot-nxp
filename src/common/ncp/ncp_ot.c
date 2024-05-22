/* @file ncp_ot.c
 *
 *  @brief This file contains ot ncp command functions.
 *
 *  Copyright 2008-2024 NXP
 *
 *  Licensed under the LA_OPT_NXP_Software_License.txt (the "Agreement")
 */

#include "app_ot.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "ncp_glue_ot.h"
#include "ot_platform_common.h"
#include <stdio.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------- */
/*                                Definitions                                 */
/* -------------------------------------------------------------------------- */

#ifndef OT_NCP_COMMAND_QUEUE_NUM
#define OT_NCP_COMMAND_QUEUE_NUM 16
#endif

#ifndef OT_NCP_TASK_PRIORITY
#define OT_NCP_TASK_PRIORITY 2
#endif

#ifndef OT_NCP_TASK_SIZE
#define OT_NCP_TASK_SIZE ((configSTACK_DEPTH_TYPE)4096 / sizeof(portSTACK_TYPE))
#endif

#define OT_NCP_RSP_MAX_SIZE (1024)

/* -------------------------------------------------------------------------- */
/*                                 Prototypes                                 */
/* -------------------------------------------------------------------------- */

typedef struct ncp_cmd_t ot_ncp_command_t;

/* -------------------------------------------------------------------------- */
/*                                 Variables                                  */
/* -------------------------------------------------------------------------- */

extern uint8_t  otCurrentCmd[OT_COMMANDS_MAX_LEN];
extern uint32_t otCmdTotalLengh;

static uint32_t sAutoRspFlag = 0;
static uint8_t  otNcpTxBuffer[OT_NCP_RSP_MAX_SIZE];
static uint16_t otNcpTxLength;

static TaskHandle_t  sOtNcpTask     = NULL;
static QueueHandle_t sOtNcpCmdQueue = NULL;

/* -------------------------------------------------------------------------- */
/*                                 Functions                                  */
/* -------------------------------------------------------------------------- */

void Copy_to_NCP_buff(const uint8_t *aBuf, uint16_t aBufLength)
{
    assert(aBuf != NULL);
    assert((otNcpTxLength + aBufLength) <= OT_NCP_RSP_MAX_SIZE);

    memcpy((otNcpTxBuffer + otNcpTxLength), aBuf, aBufLength);

    otNcpTxLength += aBufLength;
}

ncp_status_t ot_ncp_command_handle_input(uint8_t *cmd)
{
    NCP_COMMAND  *input_cmd = (NCP_COMMAND *)cmd;
    struct cmd_t *command   = NULL;
    ncp_status_t  ret       = NCP_STATUS_ERROR;

    uint32_t cmd_class    = GET_CMD_CLASS(input_cmd->cmd);
    uint32_t cmd_subclass = GET_CMD_SUBCLASS(input_cmd->cmd);
    uint32_t cmd_id       = GET_CMD_ID(input_cmd->cmd);
    void    *cmd_tlv      = GET_CMD_TLV(input_cmd);

    command = lookup_class(cmd_class, cmd_subclass, cmd_id);
    if (NULL == command)
    {
        OT_PLAT_ERR("ncp ot lookup cmd failed\r\n");
        return ret;
    }

    ret = command->handler(cmd_tlv);

    return ret;
}

uint8_t *ot_ncp_handle_response(uint8_t *pbuf, uint16_t *p_len)
{
    uint8_t *rsp_buf = NULL;

    // Remove the ot command echo feature
    if ((otCmdTotalLengh != 0) && (memcmp(pbuf, otCurrentCmd, otCmdTotalLengh) == 0))
    {
        pbuf += otCmdTotalLengh;
        *p_len -= otCmdTotalLengh;
        otCmdTotalLengh = 0;
    }

    rsp_buf = (uint8_t *)pvPortMalloc(*p_len);

    if (rsp_buf == NULL)
    {
        OT_PLAT_ERR("failed to allocate memory for response");
        return NULL;
    }

    if (*p_len > 0)
    {
        memcpy(rsp_buf, pbuf, *p_len);
    }

    return rsp_buf;
}

static void otNcpTask(void *pvParameters)
{
    uint32_t         ret = 0;
    ot_ncp_command_t cmd_item;
    uint8_t         *cmd_buf = NULL;
    uint8_t         *rsp_buf = NULL;

    while (1)
    {
        /* we need to use ot mutex here because otPlatUartReceived(),
         * otNcpTxBuffer, and otNcpTxLength are shared with
         * ot task.
         */
        appOtLockOtTask(true);

        ret = xQueueReceive(sOtNcpCmdQueue, &cmd_item, (TickType_t)0);
        if (ret == pdPASS)
        {
            cmd_buf = cmd_item.cmd_buff;

            // should parse the tlv structure and entry ot commands handle
            ot_ncp_command_handle_input(cmd_buf);

            vPortFree(cmd_buf);
            cmd_buf = NULL;
        }

        if (otNcpTxLength > 1)
        {
            rsp_buf = ot_ncp_handle_response((uint8_t *)otNcpTxBuffer, &otNcpTxLength);
            if (rsp_buf != NULL)
            {
                if ((sAutoRspFlag == 0) && (memcmp(otNcpTxBuffer, "> ", otNcpTxLength) == 0))
                {
                    sAutoRspFlag = 1;
                }
                else
                {
                    ot_send_response(NCP_OT_CMD_FORWARD, NCP_CMD_RESULT_OK, rsp_buf, otNcpTxLength);
                }

                vPortFree(rsp_buf);
            }
            else
            {
                ot_send_response(NCP_OT_CMD_FORWARD, NCP_CMD_RESULT_ERROR, NULL, 0);
                OT_PLAT_ERR("failed to aquire allocate ncp buffer.\r\n");
            }

            rsp_buf       = NULL;
            otNcpTxLength = 0;
        }

        appOtLockOtTask(false);

        vTaskDelay(50);
    }
}

static void otNcpCallback(void *tlv, size_t tlv_sz, uint32_t status)
{
    uint32_t         ret = 0;
    ot_ncp_command_t cmd_item;
    cmd_item.block_type = 0;
    cmd_item.command_sz = tlv_sz;
    cmd_item.cmd_buff   = (ncp_tlv_qelem_t *)pvPortMalloc(tlv_sz);

    if (!cmd_item.cmd_buff)
    {
        OT_PLAT_ERR("failed to allocate memory for tlv queue element.\r\n");
        return;
    }
    memcpy(cmd_item.cmd_buff, tlv, tlv_sz);

    ret = xQueueSend(sOtNcpCmdQueue, &cmd_item, (TickType_t)0);
    if (ret != pdPASS)
    {
        OT_PLAT_ERR("send to ot ncp cmd queue failed.\r\n");
    }
}

ncp_status_t ot_ncp_init(void)
{
    sOtNcpCmdQueue = xQueueCreate(OT_NCP_COMMAND_QUEUE_NUM, sizeof(ot_ncp_command_t));
    if (sOtNcpCmdQueue == NULL)
    {
        OT_PLAT_ERR("failed to create ot ncp command queue.\r\n");
        goto fail;
    }

    ncp_tlv_install_handler(GET_CMD_CLASS(NCP_CMD_15D4), (void *)otNcpCallback);

    if (xTaskCreate(otNcpTask, "ot_ncp_task", OT_NCP_TASK_SIZE, NULL, OT_NCP_TASK_PRIORITY, &sOtNcpTask) != pdPASS)
    {
        OT_PLAT_ERR("failed to create ncp ot task: %d\r\n");
        goto fail;
    }

    /* Since after reset board, ot stack will auto send "> " string to ot console and host side,
     * before SDIO and USB interface establish communication, in order to prevent data
     * pending, filtering is required here.
     */
    sAutoRspFlag = 0;

    return NCP_STATUS_SUCCESS;

fail:
    return NCP_STATUS_ERROR;
}
