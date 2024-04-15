/*
 *  Copyright 2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

/* -------------------------------------------------------------------------- */
/*                                Includes                                    */
/* -------------------------------------------------------------------------- */

#include "ot_ncp_host_cli.h"
#include "board.h"
#include "crc.h"
#include "fsl_lpuart_freertos.h"
#include "ncp_adapter.h"
#include "ncp_tlv_adapter.h"
#include "ot_ncp_bridge_cmd.h"
#include "otopcode.h"

/* -------------------------------------------------------------------------- */
/*                            Constants                                       */
/* -------------------------------------------------------------------------- */

#define MCU_CLI_STRING_SIZE 500
#define NCP_HOST_INPUT_UART_BUF_SIZE 32
#define NCP_HOST_INPUT_UART_SIZE 1
#define NCP_HOST_COMMAND_LEN 4096
#define OT_OPCODE_SIZE 1

/* LPUART1: NCP Host input uart */
#define NCP_HOST_INPUT_UART_CLK_FREQ BOARD_DebugConsoleSrcFreq()
#define NCP_HOST_INPUT_UART LPUART1
#define NCP_HOST_INPUT_UART_IRQ LPUART1_IRQn
#define NCP_HOST_INPUT_UART_NVIC_PRIO 5
/* Task: Host input */
#define TASK_HOST_INPUT_PRIO (configMAX_PRIORITIES - 3)
#define TASK_HOST_INPUT_STACK_SIZE (configSTACK_DEPTH_TYPE)4096

/* -------------------------------------------------------------------------- */
/*                            Variable                                        */
/* -------------------------------------------------------------------------- */

/*Host input uart */
lpuart_rtos_handle_t  ot_ncp_host_input_uart_handle;
struct _lpuart_handle ot_ncp_host_lpuart_handle;
static uint8_t        background_buffer[NCP_HOST_INPUT_UART_BUF_SIZE];

lpuart_rtos_config_t ot_ncp_host_input_uart_config = {
    .baudrate    = BOARD_DEBUG_UART_BAUDRATE,
    .parity      = kLPUART_ParityDisabled,
    .stopbits    = kLPUART_OneStopBit,
    .buffer      = background_buffer,
    .buffer_size = sizeof(background_buffer),
};

/* Host input buffer*/
uint8_t        ot_recv_buffer[NCP_HOST_INPUT_UART_SIZE];
static uint8_t cli_string_command_buff[MCU_CLI_STRING_SIZE];
static uint8_t cli_tlv_command_buff[NCP_HOST_COMMAND_LEN] = {0};

/* Host input task*/
TaskHandle_t task_host_input_handler = NULL;

/*command sequence number*/
uint16_t ot_cmd_seqno = 0;

/* -------------------------------------------------------------------------- */
/*                               unctions                                     */
/* -------------------------------------------------------------------------- */

void *ot_ncp_host_get_command_buffer(void)
{
    return cli_tlv_command_buff;
}

static uint32_t ot_get_input(uint8_t *inbuf, uint8_t *inlen)
{
    uint32_t ret;
    size_t   n;

    uint32_t input_len = 0;

    while (true)
    {
        ret = LPUART_RTOS_Receive(&ot_ncp_host_input_uart_handle, ot_recv_buffer, sizeof(ot_recv_buffer), &n);

        if (ret == kStatus_LPUART_RxRingBufferOverrun)
        {
            memset(background_buffer, 0, NCP_HOST_INPUT_UART_BUF_SIZE);
            memset(inbuf, 0, MCU_CLI_STRING_SIZE);
            ncp_e("Ring buffer overrun, please enter string command again");
            continue;
        }

        /*User pressed enter */
        if (*ot_recv_buffer == '\r')
        {
            if (input_len == 0)
            {
                PRINTF("\n> ");
                continue;
            }
            else
            {
                *(inbuf + input_len) = *ot_recv_buffer;
                input_len++;
                *inlen    = input_len;
                input_len = 0;
                return NCP_STATUS_SUCCESS;
            }
        }

        /*User pressed backspace */
        if (*ot_recv_buffer == '\b')
        {
            input_len--;
            *(inbuf + input_len) = '\0';
            PRINTF("\b \b");
            continue;
        }

        /* Echo input char*/
        PRINTF("%c", *ot_recv_buffer);
        *(inbuf + input_len) = *ot_recv_buffer;
        input_len++;
    }
}

uint32_t ot_ncp_host_send_tlv_command(void)
{
    uint32_t            ret     = NCP_STATUS_SUCCESS;
    uint16_t            cmd_len = 0;
    NCP_BRIDGE_COMMAND *mcu_cmd = ot_ncp_host_get_command_buffer();

    cmd_len = mcu_cmd->size;
    /* set cmd seqno */
    mcu_cmd->seqnum = ot_cmd_seqno;

    if (cmd_len == 0 || cmd_len + CHECKSUM_LEN >= NCP_HOST_COMMAND_LEN)
    {
        PRINTF("The command length exceeds the receiving capacity of mcu bridge application!\r\n");
        mcu_cmd->size = 0;
        return NCP_STATUS_ERROR;
    }

    if (cmd_len >= NCP_BRIDGE_CMD_HEADER_LEN)
    {
        ncp_tlv_send(cli_tlv_command_buff, cmd_len);
        /*Increase command sequence number*/
        ot_cmd_seqno++;
    }
    else
    {
        ncp_e("Command length is less than ncp_host_app header length (%d), cmd_len = %d", NCP_BRIDGE_CMD_HEADER_LEN,
              cmd_len);
        ret = NCP_STATUS_ERROR;
    }

    mcu_cmd->size = 0;

    return ret;
}

static void ot_ncp_host_input_task(void *pvParameters)
{
    uint8_t cli_input_len = 0;
    uint8_t otcommandlen;
    int8_t  opcode;

    ot_ncp_host_input_uart_config.srcclk = NCP_HOST_INPUT_UART_CLK_FREQ;
    ot_ncp_host_input_uart_config.base   = NCP_HOST_INPUT_UART;

    NVIC_SetPriority(NCP_HOST_INPUT_UART_IRQ, NCP_HOST_INPUT_UART_NVIC_PRIO);

    if (LPUART_RTOS_Init(&ot_ncp_host_input_uart_handle, &ot_ncp_host_lpuart_handle, &ot_ncp_host_input_uart_config) !=
        kStatus_Success)
    {
        vTaskSuspend(NULL);
    }

    while (true)
    {
        /* Receive user input */
        if (ot_get_input(cli_string_command_buff, &cli_input_len) == NCP_STATUS_SUCCESS)
        {
            // Determine the size of ot command excluding parameters
            for (otcommandlen = 0; otcommandlen < cli_input_len; otcommandlen++)
            {
                if (cli_string_command_buff[otcommandlen] == ' ' || otcommandlen == cli_input_len - 1)
                {
                    /* we break either first space is encountered or user just entered a
                     * command without any parameters.
                     */
                    break;
                }
            }

            opcode = ot_get_opcode(cli_string_command_buff, otcommandlen);
            if (opcode == -1)
            {
                PRINTF("\nNot supported command.\n> ");
                continue;
            }

            NCP_BRIDGE_COMMAND *command = ot_ncp_host_get_command_buffer();
            memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);

            command->cmd      = (NCP_BRIDGE_CMD_15D4 << 24) | NCP_15d4_CMD_FORWARD | (0x01);
            command->size     = NCP_BRIDGE_CMD_HEADER_LEN;
            command->result   = NCP_BRIDGE_CMD_RESULT_OK;
            command->msg_type = NCP_BRIDGE_MSG_TYPE_CMD;

            *((uint8_t *)command + NCP_BRIDGE_CMD_HEADER_LEN) = opcode;
            memcpy((uint8_t *)command + NCP_BRIDGE_CMD_HEADER_LEN + OT_OPCODE_SIZE,
                   cli_string_command_buff + otcommandlen, cli_input_len - otcommandlen);

            command->size += OT_OPCODE_SIZE + cli_input_len - otcommandlen;

            ot_ncp_host_send_tlv_command();

            memset(cli_string_command_buff, 0, MCU_CLI_STRING_SIZE);
            memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
        }
    }
}

uint32_t ot_ncp_host_cli_init(void)
{
    uint32_t ret;

    ncp_tlv_chksum_init();

    ret = xTaskCreate(ot_ncp_host_input_task, "ncp host input task", TASK_HOST_INPUT_STACK_SIZE, NULL,
                      TASK_HOST_INPUT_PRIO, &task_host_input_handler);
    if (ret != pdPASS)
    {
        PRINTF("Error: Failed to create ncp host input uart thread: %d\r\n", ret);
        return NCP_STATUS_ERROR;
    }

    return NCP_STATUS_SUCCESS;
}
