/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "ncp_lp_sys.h"
#include "app_notify.h"
#include "board.h"
#include "fsl_gpio.h"
#include "fsl_io_mux.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"
#include "fsl_power.h"
#include "fsl_usart.h"
#include "ncp_cmd_common.h"
#include "ncp_cmd_ot.h"
#include "ncp_glue_ot.h"
#include "ncp_lpm.h"
#if CONFIG_CRC32_HW_ACCELERATE
#include "fsl_crc.h"
#endif
#include "PWR_Interface.h"
#include "ncp_intf_pm.h"

#ifndef NCP_NOTIFY_HOST_GPIO
#define NCP_NOTIFY_HOST_GPIO 27
#endif

#ifndef NCP_NOTIFY_HOST_GPIO_MASK
#define NCP_NOTIFY_HOST_GPIO_MASK 0x8000000
#endif

typedef struct
{
    uint32_t selA;
    uint32_t selB;
    uint32_t frgSel;
    uint32_t frgctl;
    uint32_t osr;
    uint32_t brg;
} uart_clock_context_t;

uint8_t                     suspend_notify_flag = 0;
uint8_t                     ncp_wake_up_mode    = 0;
static uart_clock_context_t s_uartClockCtx;
#if CONFIG_NCP_USB
bool usb_allow_pm2_lowpower = false;
#endif

OSA_SEMAPHORE_HANDLE_DEFINE(hs_cfm);

#if CONFIG_NCP_USB
extern void USB_DevicePmStartResume(void);
extern void lpm_config_next_lp_mode(PWR_LowpowerMode_t nextMode);
#endif

static void ncp_hs_delay(uint32_t loop)
{
    if (loop > 0U)
    {
        __ASM volatile("1:                             \n"
                       "    SUBS   %0, %0, #1          \n"
                       "    CMP    %0, #0              \n"
                       "    BNE    1b                  \n"
                       :
                       : "r"(loop));
    }
}

static void ncp_hs_delay_us(uint32_t us)
{
    uint32_t instNum;

    instNum = ((SystemCoreClock + 999999UL) / 1000000UL) * us;
    ncp_hs_delay((instNum + 2U) / 3U);
}

void PIN1_INT_IRQHandler()
{
    POWER_ConfigWakeupPin(kPOWER_WakeupPin1, kPOWER_WakeupEdgeHigh);
    NVIC_ClearPendingIRQ(PIN1_INT_IRQn);
    DisableIRQ(PIN1_INT_IRQn);
    POWER_ClearWakeupStatus(PIN1_INT_IRQn);
    POWER_DisableWakeup(PIN1_INT_IRQn);
}

void ncp_gpio_wakeup_host()
{
    GPIO_PortToggle(GPIO, 1, 0x40000);
    ncp_hs_delay_us(1);
    GPIO_PortToggle(GPIO, 1, 0x40000);
}

AT_QUICKACCESS_SECTION_CODE(void host_sleep_pre_hook(void))
{
    uint32_t freq = CLK_XTAL_OSC_CLK / 40U; // frequency of LPOSC

    /* In PM2, only LPOSC and CLK32K are available. To use interface as wakeup source,
       We have to use main_clk as system clock source, and main_clk comes from LPOSC.
       Use register access directly to avoid possible flash access in function call */
    s_uartClockCtx.selA = CLKCTL0->MAINCLKSELA;
    s_uartClockCtx.selB = CLKCTL0->MAINCLKSELB;
#if CONFIG_NCP_UART
    s_uartClockCtx.frgSel = CLKCTL1->FLEXCOMM[0].FRGCLKSEL;
    s_uartClockCtx.frgctl = CLKCTL1->FLEXCOMM[0].FRGCTL;
    s_uartClockCtx.osr    = USART0->OSR;
    s_uartClockCtx.brg    = USART0->BRG;
#endif
    /* Switch main_clk to LPOSC */
    CLKCTL0->MAINCLKSELA = 2;
    CLKCTL0->MAINCLKSELB = 0;
#if CONFIG_NCP_UART
    /* Change UART0 clock source to main_clk */
    CLKCTL1->FLEXCOMM[0].FRGCLKSEL = 0;
    /* bit[0:7] div, bit[8:15] mult.
     * freq(new) = freq(old)/(1 + mult/(div+1))
     * freq(new) is the frequency of LPOSC clock
     * freq(old) is the frequency of main_pll clock
     * Use the equation here to get div and mult.
     */
    CLKCTL1->FLEXCOMM[0].FRGCTL = 0x11C7;
    USART0->OSR                 = 7;
    USART0->BRG                 = 0;
#endif
    /* Update system core clock */
    SystemCoreClock = freq / ((CLKCTL0->SYSCPUAHBCLKDIV & CLKCTL0_SYSCPUAHBCLKDIV_DIV_MASK) + 1U);
}

AT_QUICKACCESS_SECTION_CODE(void host_sleep_post_hook(uint32_t mode, void *param))
{
    /* Recover main_clk clock source after wakeup.
     Use register access directly to avoid possible flash access in function call. */
#if CONFIG_NCP_UART
    USART0->OSR                    = s_uartClockCtx.osr;
    USART0->BRG                    = s_uartClockCtx.brg;
    CLKCTL1->FLEXCOMM[0].FRGCLKSEL = s_uartClockCtx.frgSel;
    CLKCTL1->FLEXCOMM[0].FRGCTL    = s_uartClockCtx.frgctl;
#endif
    CLKCTL0->MAINCLKSELA = s_uartClockCtx.selA;
    CLKCTL0->MAINCLKSELB = s_uartClockCtx.selB;
#if !(CONFIG_NCP_SDIO)
    SystemCoreClockUpdate();
#endif
}

int host_sleep_pre_cfg(int mode)
{
    /* when outband mode is selected, set GPIO as wake up source */
    if (ncp_wake_up_mode == 1)
    {
        POWER_ConfigWakeupPin(kPOWER_WakeupPin1, kPOWER_WakeupEdgeLow);
        NVIC_ClearPendingIRQ(PIN1_INT_IRQn);
        EnableIRQ(PIN1_INT_IRQn);
        POWER_ClearWakeupStatus(PIN1_INT_IRQn);
        POWER_EnableWakeup(PIN1_INT_IRQn);
    }

#if CONFIG_NCP_USB
    /* For usb PM2 trigger by remote wake up.
     * Maybe some unknow error if any data transfer on usb interface after remote wakeup.
     */
    if (mode != 2)
    {
#endif
        /* Notify host about entering sleep mode */
        /* Wait until receiving NCP_CMD_WLAN_POWERMGMT_MCU_SLEEP_CFM from host */
        if (suspend_notify_flag == 0)
        {
#if (CONFIG_NCP_USB) || (CONFIG_NCP_SDIO)
            if (lpm_getNcpInterfaceReinitState() == NCP_INTERFACE_REINIT_ONGOING)
            {
                return kStatus_PMPowerStateNotAllowed;
            }
#endif

            suspend_notify_flag |= APP_NOTIFY_SUSPEND_EVT;
            lpm_setHandshakeState(NCP_LMP_HANDSHAKE_IN_PROCESS);

            /* For PM mode, get wakelock to wait for NCP_CMD_WLAN_POWERMGMT_MCU_SLEEP_CFM from host */
            if (OSA_SemaphoreWait((osa_semaphore_handle_t)hs_cfm, osaWaitNone_c) != 0)
            {
                return kStatus_PMPowerStateNotAllowed;
            }
            /*** need to send MCU SLEEP EVENT ENTER to host ***/
            app_notify_event(APP_EVT_MCU_SLEEP_ENTER, APP_EVT_REASON_SUCCESS, NULL, 0);
            return kStatus_PMPowerStateNotAllowed;
        }
#if CONFIG_NCP_USB
    }
#endif

    /* PM2, enable UART3 as wakeup source */
    if (mode == 2U)
    {
#if CONFIG_NCP_USB
        POWER_ClearWakeupStatus(USB_IRQn);
        POWER_EnableWakeup(USB_IRQn);
#elif CONFIG_NCP_UART
        /* Enable RX interrupt. */
        USART_EnableInterrupts(USART0, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
        POWER_ClearWakeupStatus(FLEXCOMM0_IRQn);
        POWER_EnableWakeup(FLEXCOMM0_IRQn);
#elif CONFIG_NCP_SPI
        SYSCTL0->HWWAKE = 0x10;
        NVIC_ClearPendingIRQ(WKDEEPSLEEP_IRQn);
        POWER_ClearWakeupStatus(WKDEEPSLEEP_IRQn);
        POWER_EnableWakeup(WKDEEPSLEEP_IRQn);
#endif
        /* Delay UART clock switch after resume from PM2 to avoid UART FIFO read error*/
        POWER_SetPowerSwitchCallback((power_switch_callback_t)host_sleep_pre_hook, NULL,
#if CONFIG_NCP_SDIO
                                     (power_switch_callback_t)host_sleep_post_hook,
#else
                                     NULL,
#endif
                                     NULL);
    }

    return kStatus_PMSuccess;
}

void host_sleep_post_cfg(int mode)
{
    uint32_t irq_mask;

    /* Disable wakeup source of PIN1 interrupt after waking up */
    irq_mask = DisableGlobalIRQ();
    POWER_ConfigWakeupPin(kPOWER_WakeupPin1, kPOWER_WakeupEdgeHigh);
    NVIC_ClearPendingIRQ(PIN1_INT_IRQn);
    DisableIRQ(PIN1_INT_IRQn);
    POWER_ClearWakeupStatus(PIN1_INT_IRQn);
    POWER_DisableWakeup(PIN1_INT_IRQn);
    EnableGlobalIRQ(irq_mask);

    if (mode == 2U)
    {
#if CONFIG_NCP_USB
        POWER_ClearWakeupStatus(USB_IRQn);
        POWER_DisableWakeup(USB_IRQn);
#elif CONFIG_NCP_UART
        POWER_ClearWakeupStatus(FLEXCOMM0_IRQn);
        POWER_DisableWakeup(FLEXCOMM0_IRQn);
#elif CONFIG_NCP_SPI
        POWER_ClearWakeupStatus(WKDEEPSLEEP_IRQn);
        POWER_DisableWakeup(WKDEEPSLEEP_IRQn);
        SYSCTL0->HWWAKE = 0x0;
#elif CONFIG_NCP_SDIO
        POWER_ClearWakeupStatus(SDU_IRQn);
#endif
        POWER_SetPowerSwitchCallback(NULL, NULL, NULL, NULL);
#if !(CONFIG_NCP_SDIO)
        host_sleep_post_hook(2, NULL);
#endif
    }

#if CONFIG_NCP_USB
    /* For usb PM2 trigger by remote wake up.
     * Maybe some unknow error if any data transfer on usb interface after remote wakeup.
     */
    if (2 != mode)
#endif
    {
        if (suspend_notify_flag)
        {
            lpm_setHandshakeState(NCP_LMP_HANDSHAKE_NOT_START);
            app_notify_event(APP_EVT_MCU_SLEEP_EXIT, APP_EVT_REASON_SUCCESS, NULL, 0);
        }
    }

#if CONFIG_NCP_USB
    if (mode == 2)
    {
        // start usb wakeup process
        lpm_config_next_lp_mode(0);
        USB_DevicePmStartResume();
    }
#endif

    suspend_notify_flag = 0;
}

#if CONFIG_NCP_USB
int ncp_config_suspend_mode(int mode)
{
    if (!usb_allow_pm2_lowpower)
    {
        return -1;
    }

    lpm_config_next_lp_mode(2);

    return 0;
}
#endif

void ncp_gpio_init()
{
    /* Define the init structure for the input switch pin */
    gpio_pin_config_t gpio_in_config = {
        kGPIO_DigitalInput,
        0,
    };
    gpio_pin_config_t gpio_out_config = {
        kGPIO_DigitalOutput,
        0,
    };

#if CONFIG_NCP_UART || CONFIG_NCP_USB
    GPIO_PortInit(GPIO, 0);
#endif
    GPIO_PortInit(GPIO, 1);
    GPIO_PinInit(GPIO, BOARD_SW4_GPIO_PORT, BOARD_SW4_GPIO_PIN, &gpio_in_config);
    /* Init output GPIO50 */
    GPIO_PinInit(GPIO, 1, 18, &gpio_out_config);
}

int ncp_sleep_init(void)
{
    osa_status_t status = KOSA_StatusSuccess;

    ncp_pm_init();

    status = OSA_SemaphoreCreateBinary((osa_semaphore_handle_t)hs_cfm);
    if (status != KOSA_StatusSuccess)
    {
        return -1;
    }
    OSA_SemaphorePost((osa_semaphore_handle_t)hs_cfm);

    ncp_gpio_init();
    return 1;
}

void ncp_notify_host_gpio_init(void)
{
    /* Define the init structure for the output switch pin */
    gpio_pin_config_t notify_output_pin = {kGPIO_DigitalOutput, 1};

    IO_MUX_SetPinMux(IO_MUX_GPIO27);
    GPIO_PortInit(GPIO, 0);
    /* Init output GPIO. Default level is high */
    /* After wakeup from PM3, usb device use GPIO 27 to notify usb host */
    GPIO_PinInit(GPIO, 0, NCP_NOTIFY_HOST_GPIO, &notify_output_pin);
}

void ncp_notify_host_gpio_output(void)
{
    /* Toggle GPIO to notify usb host that device is ready for re-enumeration */
    GPIO_PortToggle(GPIO, 0, NCP_NOTIFY_HOST_GPIO_MASK);
}
