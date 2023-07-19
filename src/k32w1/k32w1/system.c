/*
 *  Copyright (c) 2022, The OpenThread Authors.
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

/**
 * @file
 *   This file includes the platform-specific initializers.
 *
 */

#include "clock_config.h"
#include "fsl_clock.h"
#include "fsl_device_registers.h"
#include "fsl_port.h"
#include "pin_mux.h"
#if defined(HDI_MODE) && (HDI_MODE == 1)
#include "hdi.h"
#endif
#include "NVM_Interface.h"
#include "RNG_Interface.h"
#include "SecLib.h"
#include "app.h"
#include "board_comp.h"
#include "fsl_component_mem_manager.h"
#include "fsl_os_abstraction.h"
#include "fwk_platform.h"
#include "platform-k32w1.h"
#include <stdint.h>
#include "utils/uart.h"

void otSysInit(int argc, char *argv[])
{
    bool alreadyInit = false;

    if ((argc == 1) && (!strcmp(argv[0], "sdk_app")))
    {
        alreadyInit = true;
    }

    if (!alreadyInit)
    {
        /* Init clock config */
        BOARD_InitHardware();

        PLATFORM_InitNbu();

        PLATFORM_InitTimerManager();

        /* init framework */
        MEM_Init();

        /* Usually called from main function but in case it is compiled for OT repo applications
         * then we call it here in case any hardware like buttons or leds are needed */
        APP_InitServices();
    }

    otPlatRadioInit();
    otPlatAlarmInit();
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
    OSA_ProcessTasks();
    NvIdle();
#endif
}
