/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*${header:start}*/
#include "board.h"
#include "clock_config.h"
#include "fsl_device_registers.h"
#include "pin_mux.h"
/*${header:end}*/

#ifndef SOC_TM_INSTANCE
#define SOC_TM_INSTANCE 0U
#endif

#ifndef SOC_TM_CLK_FREQ
#define SOC_TM_CLK_FREQ SystemCoreClock
#endif

#ifndef SOC_TM_CLK_SELECT
#define SOC_TM_CLK_SELECT 2U
#endif

static gpio_pin_config_t pinConfig = {
    kGPIO_DigitalOutput,
    0,
};

/*${function:start}*/
void BOARD_InitHardware(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitAppConsole();

#ifdef OT_STACK_ENABLE_LOG
    BOARD_InitDebugConsole();
#endif

    CLOCK_AttachClk(kSFRO_to_CTIMER0);

    /* Reset GMDA */
    RESET_PeripheralReset(kGDMA_RST_SHIFT_RSTn);

    /* Enable GPIO2 */
    GPIO_PortInit(GPIO, 0);
    GPIO_PinInit(GPIO, 0, 2, &pinConfig);
    // GPIO_PinWrite(GPIO, 0, 2, 1);
}
/*${function:end}*/
