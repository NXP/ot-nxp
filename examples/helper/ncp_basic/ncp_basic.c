/*
 *  Copyright (c) 2025, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include "NVM_Interface.h"
#include "RNG_Interface.h"
#include "app.h"
#include "fsl_adapter_rpmsg.h"
#include "fsl_component_mem_manager.h"
#include "fsl_os_abstraction.h"
#include "fwk_platform.h"
#include "fwk_platform_ot.h"
#include "ncp_serial_intf.h"

#undef PLAT_SETTINGS_NVM_ID
#define PLAT_SETTINGS_NVM_ID 0xf000

#define SVC_CMD_RST 0
#define SVC_CMD_15_4_ADDR 1
#define SVC_CMD_NV_LOAD 2
#define SVC_CMD_NV_SAVE 3
#define SVC_CMD_END SVC_CMD_NV_SAVE

#define SVC_EXT_CMD_LEN sizeof(struct svc_req)

#define PLAT_SETTINGS_BUFF_SIZE 1028

struct plat_settings_t
{
    uint8_t ram_buff[PLAT_SETTINGS_BUFF_SIZE];
    uint8_t status;
} __attribute__((packed));

struct ieee_addr_t
{
    uint8_t addr[sizeof(uint64_t)];
    uint8_t status;
} __attribute__((packed));

struct svc_req
{
    uint8_t  cmd_id;
    uint32_t addr;
} __attribute__((packed));

static struct plat_settings_t plat_settings __attribute__((aligned(4)));

NVM_RegisterDataSet(&plat_settings, 1, sizeof(plat_settings.ram_buff), PLAT_SETTINGS_NVM_ID, gNVM_MirroredInRam_c);

static struct svc_req tmp_cmd;
static uint32_t       tmp_cmd_len;

static void *convert_addr(uintptr_t addr)
{
    /* we can use a linker script symbol */
    return (void *)((addr & 0x0000ffff) + 0x489c0000);
}

static void plat_reset()
{
    NvShutdown();
    NVIC_SystemReset();

    while (1)
    {
    }
}

static void plat_cmd(struct svc_req *cmd)
{
    if (!cmd)
    {
        return;
    }

    /* request for reset */
    if (cmd->cmd_id == SVC_CMD_RST)
    {
        plat_reset();
        return;
    }

    /* request for IEEE 802.15.4 addr */
    if (cmd->cmd_id == SVC_CMD_15_4_ADDR)
    {
        uintptr_t tmp = (uintptr_t)(cmd->addr);

        volatile struct ieee_addr_t *ieee_addr = convert_addr(tmp);

        PLATFORM_GetIeee802_15_4Addr((uint8_t *)(&ieee_addr->addr));

        ieee_addr->status = 1;
        return;
    }

    /* request for data load */
    if (cmd->cmd_id == SVC_CMD_NV_LOAD)
    {
        uintptr_t tmp = (uintptr_t)(cmd->addr);

        volatile struct plat_settings_t *s = convert_addr(tmp);

        /* copy data if valid */
        if (plat_settings.status)
        {
            memcpy((void *)s, &plat_settings, sizeof(plat_settings.ram_buff));
        }

        s->status = 1;
        return;
    }

    /* request for data save */
    if (cmd->cmd_id == SVC_CMD_NV_SAVE)
    {
        uintptr_t tmp = (uintptr_t)(cmd->addr);

        volatile struct plat_settings_t *s = convert_addr(tmp);

        /* copy and save data */
        memcpy(&plat_settings, (void *)s, sizeof(plat_settings.ram_buff));

        s->status            = 1;
        plat_settings.status = 1;

        NvSyncSave(&plat_settings, TRUE);
        return;
    }
}

void plat_cmd_process(uint8_t *data, uint32_t len)
{
    uint32_t cnt = 0;

    if (!data || !len)
    {
        return;
    }

    while (cnt < len)
    {
        if (tmp_cmd_len || (data[cnt] <= SVC_CMD_END))
        {
            /* fill an incomplete command */
            uint32_t delta = MIN(len - cnt, SVC_EXT_CMD_LEN - tmp_cmd_len);

            memcpy(&tmp_cmd.cmd_id + tmp_cmd_len, &data[cnt], delta);

            tmp_cmd_len += delta;
            if (tmp_cmd_len == SVC_EXT_CMD_LEN)
            {
                /* full command available */
                plat_cmd(&tmp_cmd);

                tmp_cmd_len = 0;
            }

            cnt += delta;
            continue;
        }

        if (data[cnt] > SVC_CMD_END)
        {
            /* print regular characters */
            serial_uart_tx(&data[cnt], 1);
            cnt++;
            continue;
        }
    }
}

static void plat_restore_plat_settings()
{
    NvModuleInit();

    plat_settings.status = 0; /* invalid */

    if (NvRestoreDataSet(&plat_settings, TRUE) == gNVM_OK_c)
    {
        plat_settings.status = 1;
    }
}

static void process_events()
{
    serial_uart_process();
    serial_rpmsg_process();

#if !USE_RTOS
#if !defined(FSL_OSA_MAIN_FUNC_ENABLE) || (FSL_OSA_MAIN_FUNC_ENABLE == 0)
    /* Called from OSA main() */
    OSA_ProcessTasks();
#endif

    /* NvIdle(); */
#endif

    /* PWR_EnterLowPower(0); */ /* not necessary */
}

int main()
{
#if !defined(FSL_OSA_MAIN_FUNC_ENABLE) || (FSL_OSA_MAIN_FUNC_ENABLE == 0)
    /* Called from OSA main() */
    /* Init clock config */
    BOARD_InitHardware();
#endif

    MEM_Init();

    /* restore NVM before NBU init to have the data available for it */
    plat_restore_plat_settings();

    /* APP_InitServices needs to be called before PLATFORM_InitOT because of function
     *  PLATFORM_FwkSrvRegisterLowPowerCallbacks which needs to register callbacks before NBU is started.
     *  [APP_InitServices=>APP_ServiceInitLowpower=>PWR_Init=>PLATFORM_LowPowerInit=>PLATFORM_FwkSrvRegisterLowPowerCallbacks]
     *  When low power is enabled on the host core, the radio core may need to set/release low power constraints
     *  as some resources needed by it are in the host power domain.
     *  This callback registration needs to be done before starting the radio core to avoid any race condition. */
    /* Usually called from main function but in case it is compiled for OT repo applications
     *  then we call it here in case any hardware like buttons or leds are needed */
    APP_InitServices();

    PLATFORM_InitOT();

    RNG_Init();

    serial_uart_init();
    serial_rpmsg_init();

    while (1)
    {
        process_events();
    }

    return 0;
}
