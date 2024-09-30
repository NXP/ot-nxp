/* @file app_notify.c
 *
 *  @brief This file contains declaration of the API functions.
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#include "app_notify.h"
#include "fsl_os_abstraction.h"
#include "ncp_cmd_common.h"
#include "ncp_cmd_ot.h"
#include "ncp_lp_sys.h"
#include "ncp_lpm.h"
#include "ncp_ot.h"
#if CONFIG_NCP_USB
#include "ncp_intf_usb_device_cdc.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_NOTIFY_MAX_EVENTS 20

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern volatile uint8_t OtNcpDataHandle;

/*******************************************************************************
 * Variables
 ******************************************************************************/

static OSA_MSGQ_HANDLE_DEFINE(app_notify_event_queue,
                              APP_NOTIFY_MAX_EVENTS,
                              sizeof(app_notify_msg_t)); /* app notify event queue */

static void app_notify_event_handler(void *argv);
static OSA_TASK_HANDLE_DEFINE(app_notify_event_thread); /* app notify event processing task */
static OSA_TASK_DEFINE(app_notify_event_handler,
                       PRIORITY_RTOS_TO_OSA(2),
                       1,
                       2048,
                       0); /* app notify event processing task stack*/

extern uint32_t current_cmd;

#if CONFIG_NCP_USB
extern usb_cdc_vcom_struct_t s_cdcVcom;
#endif

#if (CONFIG_NCP_USB) || (CONFIG_NCP_SDIO)
extern uint8_t lpmInterfaceReinitState;
#endif

#ifndef WM_FAIL
#define WM_FAIL 1
#endif

#ifndef WM_SUCCESS
#define WM_SUCCESS 0
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * This function is used to send event to app_notify_event_task through the
 * message queue.
 */
int app_notify_event(uint16_t event, int result, void *data, int len)
{
    app_notify_msg_t msg;

    memset(&msg, 0, sizeof(msg));
    msg.event    = event;
    msg.reason   = result;
    msg.data_len = len;
    msg.data     = data;

    if (OSA_MsgQPut((osa_msgq_handle_t)app_notify_event_queue, &msg) != KOSA_StatusSuccess)
    {
        return -WM_FAIL;
    }

    return WM_SUCCESS;
}

static uint8_t *ncp_sys_evt_status(uint32_t evt_id, void *msg)
{
    uint8_t          *event_buf = NULL;
    app_notify_msg_t *message   = (app_notify_msg_t *)msg;
    int               total_len = 0;

    total_len = message->data_len + NCP_CMD_HEADER_LEN;
    event_buf = (uint8_t *)OSA_MemoryAllocate(total_len);
    if (event_buf == NULL)
    {
        ncp_e("failed to allocate memory for event");
        return NULL;
    }

    NCP_COMMAND *evt_hdr = (NCP_COMMAND *)event_buf;
    evt_hdr->cmd         = evt_id;
    evt_hdr->size        = total_len;
    evt_hdr->seqnum      = 0x00;
    evt_hdr->result      = message->reason;
    if (message->data_len)
        memcpy(event_buf + NCP_CMD_HEADER_LEN, message->data, message->data_len);

    return event_buf;
}

/**
 * This function handles wifi_ncp_lock release for asynchronous commands
 * and sends response to uart.
 */
static void app_notify_event_handler(void *argv)
{
    int              ret = WM_SUCCESS;
    osa_status_t     status;
    app_notify_msg_t msg;
    uint8_t         *event_buf = NULL;
#if CONFIG_NCP_USB
    int lpm_usb_retry_cnt = 20;
#endif
    uint8_t ot_chk_rsp_cnt = 10;

    while (1)
    {
        /* Receive message on queue */
        status = OSA_MsgQGet((osa_msgq_handle_t)app_notify_event_queue, &msg, osaWaitForever_c);
        if (status != KOSA_StatusSuccess)
        {
            continue;
        }

        switch (msg.event)
        {
        case APP_EVT_MCU_SLEEP_ENTER:
            /* it should be ensured that the OT data has been sent first and then
             * start sleep handshake.
             * */
            ot_chk_rsp_cnt = 10;
            do
            {
                OSA_TimeDelay(50);
                ot_chk_rsp_cnt--;
            } while ((ot_chk_rsp_cnt > 0) && (OtNcpDataHandle == OT_NCP_CMD_HANDLING));

            if (ot_chk_rsp_cnt == 0)
            {
                assert(0);
            }

            // app_d("got MCU sleep enter report");
            event_buf = ncp_sys_evt_status(NCP_EVENT_MCU_SLEEP_ENTER, &msg);
            if (!event_buf)
                ret = -WM_FAIL;
            break;
        case APP_EVT_MCU_SLEEP_EXIT:
#if CONFIG_NCP_USB
            lpm_setNcpInterfaceReinitState(NCP_INTERFACE_REINIT_ONGOING);
            /* Wait for USB re-init done */
            lpm_usb_retry_cnt = 20;
            while (lpm_usb_retry_cnt > 0 && 1 != s_cdcVcom.attach)
            {
                OSA_TimeDelay(50);
                lpm_usb_retry_cnt--;
            }

            if (0 == lpm_usb_retry_cnt)
            {
                assert(0);
            }
            lpm_setNcpInterfaceReinitState(NCP_INTERFACE_REINIT_DONE);
#endif
#if CONFIG_NCP_SDIO
            lpm_setNcpInterfaceReinitState(NCP_INTERFACE_REINIT_ONGOING);
            /* Wait for SDIO re-init done */
            OSA_TimeDelay(800);
            lpm_setNcpInterfaceReinitState(NCP_INTERFACE_REINIT_DONE);
#endif
            // app_d("got MCU sleep exit report");
            event_buf = ncp_sys_evt_status(NCP_EVENT_MCU_SLEEP_EXIT, &msg);
            if (!event_buf)
                ret = -WM_FAIL;
            break;

        default:
            // app_d("no matching case");
            ret = -WM_FAIL;
            break;
        }

        if (ret == WM_SUCCESS)
        {
            if (event_buf)
            {
                if (msg.event == APP_EVT_MCU_SLEEP_ENTER || msg.event == APP_EVT_MCU_SLEEP_EXIT)
                {
                    system_ncp_send_response(event_buf);
                }
                else
                    assert(0);
            }
            else
                assert(0);
        }

        if (event_buf)
        {
            OSA_MemoryFree(event_buf);
            event_buf = NULL;
        }
    }
}

int app_notify_init(void)
{
    osa_status_t status;

    status = OSA_MsgQCreate((osa_msgq_handle_t)app_notify_event_queue, APP_NOTIFY_MAX_EVENTS, sizeof(app_notify_msg_t));
    if (status != KOSA_StatusSuccess)
    {
        // app_e("failed to create app notify event queue: %d", status);
        return -WM_FAIL;
    }

    status = OSA_TaskCreate((osa_task_handle_t)app_notify_event_thread, OSA_TASK(app_notify_event_handler), NULL);
    if (status != KOSA_StatusSuccess)
    {
        // app_e("failed to create app notify event thread: %d", status);
        return -WM_FAIL;
    }

    return WM_SUCCESS;
}
