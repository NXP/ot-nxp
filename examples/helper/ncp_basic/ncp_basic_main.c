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

#include "app.h"
#include "fsl_os_abstraction.h"
#include "ncp_serial_intf.h"

static void ncp_process()
{
    ncp_process_events();

#if !USE_RTOS
#if !defined(FSL_OSA_MAIN_FUNC_ENABLE) || (FSL_OSA_MAIN_FUNC_ENABLE == 0)
    /* Called from OSA main() */
    OSA_ProcessTasks();
#endif

    /* NvIdle(); */
#endif

    /* PWR_EnterLowPower(0); */ /* not necessary */
}

static void ncp_basic_init()
{
#if !defined(FSL_OSA_MAIN_FUNC_ENABLE) || (FSL_OSA_MAIN_FUNC_ENABLE == 0)
    /* Called from OSA main() */
    /* Init clock config */
    BOARD_InitHardware();
#endif

    ncp_basic_init_1();

    /* APP_InitServices needs to be called before PLATFORM_InitOT because of function
     *  PLATFORM_FwkSrvRegisterLowPowerCallbacks which needs to register callbacks before NBU is started.
     *  [APP_InitServices=>APP_ServiceInitLowpower=>PWR_Init=>PLATFORM_LowPowerInit=>PLATFORM_FwkSrvRegisterLowPowerCallbacks]
     *  When low power is enabled on the host core, the radio core may need to set/release low power constraints
     *  as some resources needed by it are in the host power domain.
     *  This callback registration needs to be done before starting the radio core to avoid any race condition. */
    /* Usually called from main function but in case it is compiled for OT repo applications
     *  then we call it here in case any hardware like buttons or leds are needed */
    APP_InitServices();

    ncp_basic_init_2();
}

int main()
{
    ncp_basic_init();

    while (1)
    {
        ncp_process();
    }

    return 0;
}
