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

#include "RNG_Interface.h"
#include "fsl_adapter_rpmsg.h"
#include "fsl_component_mem_manager.h"
#include "fsl_component_timer_manager.h"
#include "fsl_device_registers.h"
#include "fsl_os_abstraction.h"
#include "fwk_platform.h"
#include "fwk_platform_ics.h"
#include "fwk_platform_lowpower.h"

#include "openthread-system.h"
#include "openthread/platform/misc.h"
#include "utils/uart.h"

#include "platform-mcxw72_nbu.h"

#if !defined(configUSE_TICKLESS_IDLE) || (defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 0))
#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0)
#include <openthread/tasklet.h>
#endif /*gAppLowpowerEnabled_d*/
#endif /*!defined(configUSE_TICKLESS_IDLE) || (defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE==0))*/

extern uint32_t m_shared_ram_start[];
extern uint32_t m_shared_ram_end[];

#define SMU2_CM33_BASE_ADDR ((uint32_t)(&m_shared_ram_start))
#define SMU2_CM33_END_ADDR ((uint32_t)(&m_shared_ram_end) + 1)
#define SMU2_MAIR_IDX 1

#define SVC_CMD_RST 0
#define SVC_CMD_15_4_ADDR 1
#define SVC_CMD_NV_LOAD 2
#define SVC_CMD_NV_SAVE 3

#define TMP_TMR_DELTA 100 /* ms */

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

const NbuInfo_t nbu_version;

static TIMER_MANAGER_HANDLE_DEFINE(tmp_timer_handle);

static volatile struct ieee_addr_t ieee_addr __attribute__((section(".m_154_addr_region")));

static void tmp_timer_callback(void *p)
{
    (void)p;
}

OT_TOOL_WEAK uint32_t Controller_HandleNbuApiReq(uint8_t *api_return, uint8_t *data, uint32_t data_len)
{
    return 0;
}

OT_TOOL_WEAK bool Controller_EnableSecurityFeature()
{
    return false;
}

OT_TOOL_WEAK void APP_SysInitHook(void)
{
    /* Intentionally left empty */
}

void plat_settings_load(void *data)
{
    volatile struct plat_settings_t *s = data;
    struct svc_req                   tmp;

    if (!data)
    {
        return;
    }

    tmp.cmd_id = SVC_CMD_NV_LOAD;
    tmp.addr   = (uint32_t)(uintptr_t)data;

    s->status = 0; /* invalid */

    /* request data from main core */
    otPlatUartSend((uint8_t *)(&tmp), sizeof(tmp));

    while (!s->status)
    {
    }
}

void plat_settings_save(void *data)
{
    volatile struct plat_settings_t *s = data;
    struct svc_req                   tmp;

    if (!data)
    {
        return;
    }

    tmp.cmd_id = SVC_CMD_NV_SAVE;
    tmp.addr   = (uint32_t)(uintptr_t)data;

    s->status = 0; /* invalid */

    /* send data to main core */
    otPlatUartSend((uint8_t *)(&tmp), sizeof(tmp));

    while (!s->status)
    {
    }
}

void PLATFORM_GetIeee802_15_4Addr(uint8_t *eui64)
{
    if (!ieee_addr.status)
    {
        struct svc_req tmp;

        tmp.cmd_id = SVC_CMD_15_4_ADDR;
        tmp.addr   = (uint32_t)(uintptr_t)(&ieee_addr);

        /* request addr from main core */
        otPlatUartSend((uint8_t *)(&tmp), sizeof(tmp));

        while (!ieee_addr.status)
        {
        }
    }

    if (eui64)
    {
        memcpy(eui64, (void *)ieee_addr.addr, sizeof(ieee_addr.addr));
    }
}

static void plat_init_ieee_802_15_4_addr()
{
    ieee_addr.status = 0; /* invalid */
    PLATFORM_GetIeee802_15_4Addr(NULL);
}

void otPlatReset(otInstance *aInstance)
{
    struct svc_req tmp;

    OT_UNUSED_VARIABLE(aInstance);

    tmp.cmd_id = SVC_CMD_RST;
    tmp.addr   = 0;

    /* request main core to reset */
    otPlatUartSend((uint8_t *)(&tmp), sizeof(tmp));

    NVIC_SystemReset();

    while (1)
    {
    }
}

static void plat_init_mpu()
{
    /* add SMU2 as regular memory to avoid unaligned access exception */
    ARM_MPU_SetRegion(SMU2_MAIR_IDX, SMU2_CM33_BASE_ADDR,
                      SMU2_CM33_END_ADDR | (MPU_RLAR_EN_Msk << MPU_RLAR_EN_Pos) |
                          (SMU2_MAIR_IDX << MPU_RLAR_AttrIndx_Pos));

    ARM_MPU_SetMemAttr(SMU2_MAIR_IDX, ARM_MPU_ATTR(ARM_MPU_ATTR_NON_CACHEABLE, ARM_MPU_ATTR_NON_CACHEABLE));

    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
}

void otSysInit(int argc, char *argv[])
{
    if (argc != 1)
    {
        /* do basic init */
#if !defined(FSL_OSA_MAIN_FUNC_ENABLE) || (FSL_OSA_MAIN_FUNC_ENABLE == 0)
        /* Called from OSA main() */
        OSA_Init();
#endif

        MEM_Init();
    }

    plat_init_mpu();

    HAL_RpmsgMcmgrInit();
    PLATFORM_FwkSrvInit();
    RNG_Init();

    PLATFORM_InitTimerManager();

    /* enable LPTRM counter used by otPlatTimeGet() */
    TM_Open(tmp_timer_handle);
    TM_InstallCallback(tmp_timer_handle, tmp_timer_callback, NULL);
    TM_Start(tmp_timer_handle, kTimerModeSingleShot, TMP_TMR_DELTA);

    /* Hook used to call OT repo application functions */
    APP_SysInitHook();

    otPlatRandomInit();
    otPlatRadioInit();
    otPlatAlarmInit();

    /* necessary for requesting host services */
    otPlatUartEnable();

    plat_init_ieee_802_15_4_addr();
}

bool otSysPseudoResetWasRequested(void)
{
    return false;
}

void otSysDeinit(void)
{
}

OT_TOOL_WEAK void otSysEventSignalPending(void)
{
    /* Intentionally left empty */
}

void otSysProcessDrivers(otInstance *aInstance)
{
    otPlatRadioProcess(aInstance);
    otPlatAlarmProcess(aInstance);
    otPlatUartProcess();

#if !USE_RTOS
#if !defined(FSL_OSA_MAIN_FUNC_ENABLE) || (FSL_OSA_MAIN_FUNC_ENABLE == 0)
    /* Called from OSA main() */
    OSA_ProcessTasks();
#endif

    /* NvIdle(); */
    /* SFC_Process(); */
#endif

#if !defined(configUSE_TICKLESS_IDLE) || (defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 0))
#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0)
    {
        /*
         * We need to protect PWR_EnterLowPower with interrupt disable/enable because PWR_EnterLowPower
         * does not used interrupt disable/enable protection at all.
         * Cover the case when at beginning PWR_EnterLowPower check the lpDisallowCount counter and it
         * find it 0 which means can enter to low power, however if an interrupt which disallow entering
         * into low power (like timerCallback for alarm milli handler) happens between lpDisallowCount check
         * and WFI instruction, then IRQ will be serviced and device will end up into an inconsistent state,
         * i.e. lpDisallowCount will be set to 1 from serviced IRQ but the device will manage to enter in low power,
         * leading to fail to process the event in the main loop and possible stays in low power for ever
         * if it was the single wake-up interrupt.
         * Also otTaskletsArePending is included into interrupt protection to cover the case when between
         * otTaskletsArePending check and WFI/PWR_EnterLowPower some Task.Post() is happening.
         * If not using DisableGlobalIRQ here and Task.Post() from interrupt happens between
         * otTaskletsArePending check and PWR_EnterLowPower entering, then task processing will be missed.
         */
        uint32_t intMask = DisableGlobalIRQ();
        if (otTaskletsArePending(aInstance) == false)
        {
            PLATFORM_EnterLowPower();
        }
        EnableGlobalIRQ(intMask);
    }
#endif /*defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0)*/
#endif /*!defined(configUSE_TICKLESS_IDLE) || (defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE==0))*/
}

void PWR_AllowDeviceToSleep()
{
}

void PWR_DisallowDeviceToSleep()
{
}

void SystemInitHook()
{
    /* Configure NBU memory mapping as early as possible in the SystemInitHook()
       to prevent any potential issues */
    PLATFORM_ConfigureSmuDmemMapping();
}
