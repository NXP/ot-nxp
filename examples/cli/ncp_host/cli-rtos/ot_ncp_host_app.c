/*
 *  Copyright 2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

/* -------------------------------------------------------------------------- */
/*                                Includes                                    */
/* -------------------------------------------------------------------------- */

#include "ot_ncp_host_app.h"
#include "fsl_debug_console.h"
#include "ncp_tlv_adapter.h"
#include "ot_ncp_cmd.h"
#include "ot_ncp_host_cli.h"

/* -------------------------------------------------------------------------- */
/*                              Constants                                     */
/* -------------------------------------------------------------------------- */

#define OT_NCP_COMMAND_QUEUE_NUM 16

/* -------------------------------------------------------------------------- */
/*                                Function prototypes                         */
/* -------------------------------------------------------------------------- */

void ot_ncp_app_task(void *pvParameters);
int  system_process_event(uint8_t *res);

/* -------------------------------------------------------------------------- */
/*                              Variable                                      */
/* -------------------------------------------------------------------------- */

static osa_msgq_handle_t ot_ncp_command_queue;
static OSA_TASK_DEFINE(ot_ncp_app_task, configMAX_PRIORITIES - 4, 1, 4096, 0);
static OSA_TASK_HANDLE_DEFINE(ot_ncp_app_task_handle);
OSA_MSGQ_HANDLE_DEFINE(ot_ncp_command_queue_buff, OT_NCP_COMMAND_QUEUE_NUM, sizeof(ot_ncp_command_t));

extern OSA_SEMAPHORE_HANDLE_DEFINE(gpio_wakelock);

int         mcu_device_status = MCU_DEVICE_STATUS_ACTIVE;
power_cfg_t global_power_config;

/* -------------------------------------------------------------------------- */
/*                                Functions                                   */
/* -------------------------------------------------------------------------- */

int system_process_event(uint8_t *res)
{
    int ret = NCP_STATUS_SUCCESS;

    NCP_COMMAND *evt = (NCP_COMMAND *)res;

    switch (evt->cmd)
    {
    case NCP_EVENT_MCU_SLEEP_ENTER:
    case NCP_EVENT_MCU_SLEEP_EXIT:
        ret = ot_ncp_process_sleep_status(res);
        break;
    default:
        PRINTF("Invalid event!\r\n");
    }

    return ret;
}

void ncp_mcu_sleep_cfm(NCP_COMMAND *header)
{
    header->cmd    = NCP_CMD_SYSTEM_POWERMGMT_MCU_SLEEP_CFM;
    header->size   = NCP_CMD_HEADER_LEN;
    header->result = NCP_CMD_RESULT_OK;
}

int ot_ncp_process_sleep_status(uint8_t *res)
{
    NCP_COMMAND *event  = (NCP_COMMAND *)res;
    int          status = 0;

    if (event->cmd == NCP_EVENT_MCU_SLEEP_ENTER)
    {
        NCP_COMMAND sleep_cfm;

        PRINTF("\r\nGet device sleep event, it will enter sleep mode\r\n");
        memset(&sleep_cfm, 0x0, sizeof(sleep_cfm));
        ncp_mcu_sleep_cfm(&sleep_cfm);
        status = ncp_tlv_send((void *)&sleep_cfm, sleep_cfm.size);

        if (status != NCP_STATUS_SUCCESS)
            PRINTF("Failed to send mcu sleep cfm\r\n");

        OSA_TimeDelay(100);
        mcu_device_status = MCU_DEVICE_STATUS_SLEEP;

        if (global_power_config.wake_mode == WAKE_MODE_GPIO)
        {
            status = OSA_SemaphoreWait((osa_semaphore_handle_t)gpio_wakelock, osaWaitNone_c);
            if (status != NCP_STATUS_SUCCESS)
                (void)PRINTF("Failed to get gpio_wakelock\r\n");
        }
    }
    else
    {
        PRINTF("\r\nMCU device exits sleep mode\r\n");
        mcu_device_status = MCU_DEVICE_STATUS_ACTIVE;

        if (global_power_config.wake_mode == WAKE_MODE_GPIO)
        {
            status = OSA_SemaphorePost((osa_semaphore_handle_t)gpio_wakelock);
            if (status != NCP_STATUS_SUCCESS)
                (void)PRINTF("Failed to put gpio_wakelock\r\n");
        }
    }

    return NCP_STATUS_SUCCESS;
}

static uint32_t ot_ncp_handle_cmd_input(uint8_t *cmd, uint32_t len)
{
    uint32_t msg_type = 0;
    int      ret      = NCP_STATUS_SUCCESS;

    msg_type = GET_MSG_TYPE(((NCP_COMMAND *)cmd)->cmd);
    if (msg_type == NCP_MSG_TYPE_EVENT)
    {
        ret = system_process_event(cmd);
        if (ret != NCP_STATUS_SUCCESS)
        {
            ncp_e("Failed to parse ncp event.");
        }
    }
    else
    {
        cmd[len] = '\0';
        PRINTF("%s", cmd + NCP_CMD_HEADER_LEN);
    }

    return ret;
}

void ot_ncp_callback(void *tlv, size_t tlv_sz, uint32_t status)
{
    uint32_t         ret = 0;
    ot_ncp_command_t cmd_item;
    cmd_item.block_type = 0;
    cmd_item.command_sz = tlv_sz;
    cmd_item.cmd_buff   = (ncp_tlv_qelem_t *)OSA_MemoryAllocate(tlv_sz);

    if (!cmd_item.cmd_buff)
    {
        ncp_adap_e("Failed to allocate memory for tlv queue element.");
        return;
    }
    memcpy(cmd_item.cmd_buff, tlv, tlv_sz);

    ret = OSA_MsgQPut(ot_ncp_command_queue, &cmd_item);
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_e("Send to ot ncp cmd queue failed.");
    }
}

void ot_ncp_app_task(void *pvParameters)
{
    uint32_t         ret = 0;
    ot_ncp_command_t cmd_item;

    while (1)
    {
        ret = OSA_MsgQGet(ot_ncp_command_queue, &cmd_item, osaWaitForever_c);
        if (ret != NCP_STATUS_SUCCESS)
        {
            ncp_e("Recv ot ncp command queue failed.");
            continue;
        }
        else
        {
            ot_ncp_handle_cmd_input(cmd_item.cmd_buff, cmd_item.command_sz);

            OSA_MemoryFree(cmd_item.cmd_buff);
            cmd_item.cmd_buff = NULL;
        }
    }
}

uint32_t ot_ncp_host_app_init(void)
{
    uint32_t ret;

    ot_ncp_command_queue = (osa_msgq_handle_t)ot_ncp_command_queue_buff;
    ret                  = OSA_MsgQCreate(ot_ncp_command_queue, OT_NCP_COMMAND_QUEUE_NUM, sizeof(ot_ncp_command_t));
    if (ret != KOSA_StatusSuccess)
    {
        ncp_e("Create ot ncp command queue fail.");
        return NCP_STATUS_ERROR;
    }

    ncp_tlv_install_handler(GET_CMD_CLASS(NCP_CMD_15D4), (void *)ot_ncp_callback);

    ret = OSA_TaskCreate((osa_task_handle_t)ot_ncp_app_task_handle, OSA_TASK(ot_ncp_app_task), NULL);
    if (ret != KOSA_StatusSuccess)
    {
        ncp_e("Create ot ncp task fail.");
        return NCP_STATUS_ERROR;
    }

    return NCP_STATUS_SUCCESS;
}
