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
#include "fsl_adapter_gpio.h"
#include "fsl_lpuart_freertos.h"
#include "ncp_adapter.h"
#include "ncp_tlv_adapter.h"
#include "ot_ncp_cmd.h"
#include "otopcode.h"
#if CONFIG_NCP_USB
#include "usb_host_cdc_app.h"
#endif

/* -------------------------------------------------------------------------- */
/*                            Constants                                       */
/* -------------------------------------------------------------------------- */

#define MCU_CLI_STRING_SIZE 500
#define NCP_HOST_INPUT_UART_BUF_SIZE 32
#define NCP_HOST_INPUT_UART_SIZE 1
#define NCP_HOST_COMMAND_LEN 4096
#define OT_OPCODE_SIZE 1

#define SDHOST_RESCAN_START 0x01

/* LPUART1: NCP Host input uart */
#define NCP_HOST_INPUT_UART_CLK_FREQ BOARD_DebugConsoleSrcFreq()
#define NCP_HOST_INPUT_UART LPUART1
#define NCP_HOST_INPUT_UART_IRQ LPUART1_IRQn
#define NCP_HOST_INPUT_UART_NVIC_PRIO 5
/* Task: Host input */
#define TASK_HOST_INPUT_PRIO (configMAX_PRIORITIES - 3)
#define TASK_HOST_INPUT_STACK_SIZE (configSTACK_DEPTH_TYPE)4096

#if CONFIG_NCP_USB
extern uint8_t usb_enter_pm2;
#endif

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

GPIO_HANDLE_DEFINE(ncp_mcu_host_wakeup_handle);
extern uint8_t     mcu_device_status;
extern power_cfg_t global_power_config;
OSA_SEMAPHORE_HANDLE_DEFINE(gpio_wakelock);

/* -------------------------------------------------------------------------- */
/*                                Function prototypes                         */
/* -------------------------------------------------------------------------- */

#if (CONFIG_NCP_SDIO)
extern void sdhost_rescan_set_event(osa_event_flags_t flagsToWait);
#endif

/* -------------------------------------------------------------------------- */
/*                               functions                                    */
/* -------------------------------------------------------------------------- */

void *ot_ncp_host_get_command_buffer(void)
{
    return cli_tlv_command_buff;
}

static uint32_t ot_get_input(uint8_t *inbuf, uint8_t *inlen)
{
    uint32_t ret;
    size_t   n;

    uint8_t  front_space = 0;
    uint32_t input_len   = 0;

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
                *inlen      = input_len;
                input_len   = 0;
                front_space = 0;
                return NCP_STATUS_SUCCESS;
            }
        }

        if (!front_space && *ot_recv_buffer == ' ')
        {
            PRINTF(" ");
            continue;
        }

        front_space = 1;

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
    uint32_t     ret     = NCP_STATUS_SUCCESS;
    uint16_t     cmd_len = 0;
    NCP_COMMAND *mcu_cmd = ot_ncp_host_get_command_buffer();

    cmd_len = mcu_cmd->size;
    /* set cmd seqno */
    mcu_cmd->seqnum = ot_cmd_seqno;

    if (cmd_len == 0 || cmd_len + CHECKSUM_LEN >= NCP_HOST_COMMAND_LEN)
    {
        PRINTF("The command length exceeds the receiving capacity of mcu application!\r\n");
        mcu_cmd->size = 0;
        return NCP_STATUS_ERROR;
    }

    if (cmd_len >= NCP_CMD_HEADER_LEN)
    {
        /* Wakeup MCU device through GPIO if host configured GPIO wake mode */
        if ((global_power_config.wake_mode == WAKE_MODE_GPIO) && (mcu_device_status == MCU_DEVICE_STATUS_SLEEP))
        {
            GPIO_PinWrite(GPIO1, 27, 0);
            PRINTF("get gpio_wakelock after GPIO wakeup\r\n");
            /* Block here to wait for MCU device complete the PM3 exit process */
            OSA_SemaphoreWait((osa_semaphore_handle_t)gpio_wakelock, osaWaitForever_c);
            GPIO_PinWrite(GPIO1, 27, 1);
            /* Release semaphore here to make sure software can get it successfully when receiving sleep enter event for
             * next sleep loop. */
            OSA_SemaphorePost((osa_semaphore_handle_t)gpio_wakelock);
        }
        ncp_tlv_send(cli_tlv_command_buff, cmd_len);

        /*Increase command sequence number*/
        ot_cmd_seqno++;
    }
    else
    {
        ncp_e("Command length is less than ncp_host_app header length (%d), cmd_len = %d", NCP_CMD_HEADER_LEN, cmd_len);
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
    uint8_t wakeup_mode;

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

            NCP_COMMAND *command = ot_ncp_host_get_command_buffer();
            memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);

            command->cmd    = NCP_CMD_15D4 | NCP_15d4_CMD_FORWARD | NCP_MSG_TYPE_CMD | 0x00000001;
            command->size   = NCP_CMD_HEADER_LEN;
            command->result = NCP_CMD_RESULT_OK;
            command->rsvd   = 0;

            *((uint8_t *)command + NCP_CMD_HEADER_LEN) = opcode;
            memcpy((uint8_t *)command + NCP_CMD_HEADER_LEN + OT_OPCODE_SIZE, cli_string_command_buff + otcommandlen,
                   cli_input_len - otcommandlen);

            command->size += OT_OPCODE_SIZE + cli_input_len - otcommandlen;
#if CONFIG_NCP_USB
            if (opcode == ot_get_opcode("ncp-usb-pm2", strlen("ncp-usb-pm2")))
            {
                if (*((uint8_t *)command + NCP_CMD_HEADER_LEN + 2) == USB_PM2_ENTER_PARAM)
                {
                    usb_enter_pm2 = USB_PM2_ACTION_ENTER;
                    PRINTF("Set usb enter PM2\r\n");
                }
                else if (*((uint8_t *)command + NCP_CMD_HEADER_LEN + 2) == USB_PM2_EXIT_PARAM)
                {
                    usb_enter_pm2 = USB_PM2_ACTION_EXIT;
                    PRINTF("Set usb exit PM2\r\n");
                }
                else
                {
                    PRINTF("\r\nThe param is wrong, please use:\r\n");
                    PRINTF("\tncp-usb-pm2 1 --> enter usb pm2 mode\r\n");
                    PRINTF("\tncp-usb-pm2 2 --> exit usb pm2 mode\r\n");
                    usb_enter_pm2 = USB_PM2_ACTION_NONE;
                }
            }
            else
#endif
            {
                ot_ncp_host_send_tlv_command();

                memset(cli_string_command_buff, 0, MCU_CLI_STRING_SIZE);

#if (CONFIG_NCP_SDIO)
                /* Reset sdio host if command is reset/factoryreset */
                if (opcode == ot_get_opcode("reset", strlen("reset")) ||
                    opcode == ot_get_opcode("factoryreset", strlen("factoryreset")))
                {
                    PRINTF("\nSDIO reseting...\n");
                    vTaskDelay(pdMS_TO_TICKS(2000));
                    sdhost_rescan_set_event(SDHOST_RESCAN_START);
                }
#endif
                if (opcode == ot_get_opcode("ncp-wake-cfg", strlen("ncp-wake-cfg")))
                {
                    wakeup_mode = *((uint8_t *)command + NCP_CMD_HEADER_LEN + 2);

                    if (wakeup_mode == INBAND_WAKEUP_PARAM)
                    {
                        PRINTF("select inband mode to wake up\r\n");
                        global_power_config.wake_mode = WAKE_MODE_INTF;
                    }
                    else if (wakeup_mode == OUTBAND_WAKEUP_PARAM)
                    {
                        PRINTF("select outband mode to wake up\r\n");
                        global_power_config.wake_mode = WAKE_MODE_GPIO;
                    }
                    else
                    {
                        PRINTF("select wrong wake up param, please try again\r\n");
                    }
                }
            }
            memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
        }
    }
}

static void wakeup_int_callback(void *param)
{
    /* This function will be called when device wakes up host through GPIO */
    PRINTF("Wakeup host sucessfully\r\n");
}

static void ncp_host_lpm_gpio_init()
{
    /* Define the init structure for the input/output switch pin */
    gpio_pin_config_t gpio_in_config = {
        .direction = kGPIO_DigitalInput, .outputLogic = 0, .interruptMode = kGPIO_IntRisingEdge};
    gpio_pin_config_t gpio_out_config = {
        .direction = kGPIO_DigitalOutput, .outputLogic = 1, .interruptMode = kGPIO_NoIntmode};
    /* Init input GPIO for wakeup MCU host */
    GPIO_PinInit(GPIO1, 26, &gpio_in_config);
    /* Init output GPIO for wakeup NCP device */
    GPIO_PinInit(GPIO1, 27, &gpio_out_config);
    hal_gpio_pin_config_t wakeup_config = {kHAL_GpioDirectionIn, 0, 1, 26};
    HAL_GpioInit(ncp_mcu_host_wakeup_handle, &wakeup_config);
    HAL_GpioSetTriggerMode(ncp_mcu_host_wakeup_handle, kHAL_GpioInterruptRisingEdge);
    HAL_GpioInstallCallback(ncp_mcu_host_wakeup_handle, wakeup_int_callback, NULL);
}

uint32_t ot_ncp_host_cli_init(void)
{
    uint32_t ret;

    ncp_host_lpm_gpio_init();

    // inband as the default wake up mode
    global_power_config.wake_mode = WAKE_MODE_INTF;

    if (OSA_SemaphoreCreateBinary((osa_semaphore_handle_t)gpio_wakelock) != NCP_STATUS_SUCCESS)
    {
        ncp_e("Failed to create gpio_wakelock");
        return -NCP_STATUS_ERROR;
    }

    OSA_SemaphorePost((osa_semaphore_handle_t)gpio_wakelock);

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
