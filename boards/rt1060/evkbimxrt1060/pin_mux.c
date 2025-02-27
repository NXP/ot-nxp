/*
 * Copyright 2018-2022, 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v11.0
processor: MIMXRT1062xxxxA
package_id: MIMXRT1062DVL6A
mcu_data: ksdk2_0
processor_version: 11.0.1
board: MIMXRT1060-EVKB
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

#include "pin_mux.h"
#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
    BOARD_InitDEBUG_UARTPins();
#if !defined(CONFIG_NCP) || defined(CONFIG_NCP_UART)
    BOARD_InitArduinoUARTPins();
#endif
#if defined(SDIO_ENABLED)
    BOARD_InitUSDHCPins();
#endif
}

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: L11, peripheral: LPUART2, signal: TX, pin_signal: GPIO_AD_B1_02}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPins(void)
{
    CLOCK_EnableClock(kCLOCK_Iomuxc);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_02_LPUART2_TX, 0U);
    IOMUXC_GPR->GPR26 = ((IOMUXC_GPR->GPR26 & (~(BOARD_INITPINS_IOMUXC_GPR_GPR26_GPIO_MUX1_GPIO_SEL_MASK))) |
                         IOMUXC_GPR_GPR26_GPIO_MUX1_GPIO_SEL(0x04U));
}

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitDEBUG_UARTPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: K14, peripheral: LPUART1, signal: TX, pin_signal: GPIO_AD_B0_12, software_input_on: Disable,
hysteresis_enable: Disable, pull_up_down_config: Pull_Down_100K_Ohm, pull_keeper_select: Keeper, pull_keeper_enable:
Enable, open_drain: Disable, speed: MHZ_100, drive_strength: R0_6, slew_rate: Slow}
  - {pin_num: L14, peripheral: LPUART1, signal: RX, pin_signal: GPIO_AD_B0_13, software_input_on: Disable,
hysteresis_enable: Disable, pull_up_down_config: Pull_Down_100K_Ohm, pull_keeper_select: Keeper, pull_keeper_enable:
Enable, open_drain: Disable, speed: MHZ_100, drive_strength: R0_6, slew_rate: Slow}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitDEBUG_UARTPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitDEBUG_UARTPins(void)
{
    CLOCK_EnableClock(kCLOCK_Iomuxc);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_12_LPUART1_TX, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_13_LPUART1_RX, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_12_LPUART1_TX, 0x10B0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_13_LPUART1_RX, 0x10B0U);
}

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitSDRAMPins:
- options: {callFromInitBoot: 'false', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: C2, peripheral: SEMC, signal: 'ADDR, 00', pin_signal: GPIO_EMC_09}
  - {pin_num: G1, peripheral: SEMC, signal: 'ADDR, 01', pin_signal: GPIO_EMC_10}
  - {pin_num: G3, peripheral: SEMC, signal: 'ADDR, 02', pin_signal: GPIO_EMC_11}
  - {pin_num: H1, peripheral: SEMC, signal: 'ADDR, 03', pin_signal: GPIO_EMC_12}
  - {pin_num: A6, peripheral: SEMC, signal: 'ADDR, 04', pin_signal: GPIO_EMC_13}
  - {pin_num: B6, peripheral: SEMC, signal: 'ADDR, 05', pin_signal: GPIO_EMC_14}
  - {pin_num: B1, peripheral: SEMC, signal: 'ADDR, 06', pin_signal: GPIO_EMC_15}
  - {pin_num: A5, peripheral: SEMC, signal: 'ADDR, 07', pin_signal: GPIO_EMC_16}
  - {pin_num: A4, peripheral: SEMC, signal: 'ADDR, 08', pin_signal: GPIO_EMC_17}
  - {pin_num: B2, peripheral: SEMC, signal: 'ADDR, 09', pin_signal: GPIO_EMC_18}
  - {pin_num: G2, peripheral: SEMC, signal: 'ADDR, 10', pin_signal: GPIO_EMC_23}
  - {pin_num: B4, peripheral: SEMC, signal: 'ADDR, 11', pin_signal: GPIO_EMC_19}
  - {pin_num: A3, peripheral: SEMC, signal: 'ADDR, 12', pin_signal: GPIO_EMC_20}
  - {pin_num: C1, peripheral: SEMC, signal: 'BA, 0', pin_signal: GPIO_EMC_21}
  - {pin_num: F1, peripheral: SEMC, signal: 'BA, 1', pin_signal: GPIO_EMC_22}
  - {pin_num: D3, peripheral: SEMC, signal: semc_cas, pin_signal: GPIO_EMC_24}
  - {pin_num: A2, peripheral: SEMC, signal: semc_cke, pin_signal: GPIO_EMC_27}
  - {pin_num: B3, peripheral: SEMC, signal: semc_clk, pin_signal: GPIO_EMC_26}
  - {pin_num: E3, peripheral: SEMC, signal: 'DATA, 00', pin_signal: GPIO_EMC_00}
  - {pin_num: F3, peripheral: SEMC, signal: 'DATA, 01', pin_signal: GPIO_EMC_01}
  - {pin_num: F4, peripheral: SEMC, signal: 'DATA, 02', pin_signal: GPIO_EMC_02}
  - {pin_num: G4, peripheral: SEMC, signal: 'DATA, 03', pin_signal: GPIO_EMC_03}
  - {pin_num: F2, peripheral: SEMC, signal: 'DATA, 04', pin_signal: GPIO_EMC_04}
  - {pin_num: G5, peripheral: SEMC, signal: 'DATA, 05', pin_signal: GPIO_EMC_05}
  - {pin_num: H5, peripheral: SEMC, signal: 'DATA, 06', pin_signal: GPIO_EMC_06}
  - {pin_num: H4, peripheral: SEMC, signal: 'DATA, 07', pin_signal: GPIO_EMC_07}
  - {pin_num: C6, peripheral: SEMC, signal: 'DATA, 08', pin_signal: GPIO_EMC_30}
  - {pin_num: C5, peripheral: SEMC, signal: 'DATA, 09', pin_signal: GPIO_EMC_31}
  - {pin_num: D5, peripheral: SEMC, signal: 'DATA, 10', pin_signal: GPIO_EMC_32}
  - {pin_num: C4, peripheral: SEMC, signal: 'DATA, 11', pin_signal: GPIO_EMC_33}
  - {pin_num: D4, peripheral: SEMC, signal: 'DATA, 12', pin_signal: GPIO_EMC_34}
  - {pin_num: E5, peripheral: SEMC, signal: 'DATA, 13', pin_signal: GPIO_EMC_35}
  - {pin_num: C3, peripheral: SEMC, signal: 'DATA, 14', pin_signal: GPIO_EMC_36}
  - {pin_num: E4, peripheral: SEMC, signal: 'DATA, 15', pin_signal: GPIO_EMC_37}
  - {pin_num: H3, peripheral: SEMC, signal: 'DM, 0', pin_signal: GPIO_EMC_08}
  - {pin_num: D6, peripheral: SEMC, signal: 'DM, 1', pin_signal: GPIO_EMC_38}
  - {pin_num: D2, peripheral: SEMC, signal: semc_ras, pin_signal: GPIO_EMC_25}
  - {pin_num: D1, peripheral: SEMC, signal: semc_we, pin_signal: GPIO_EMC_28}
  - {pin_num: E1, peripheral: SEMC, signal: 'CS, 0', pin_signal: GPIO_EMC_29}
  - {pin_num: B7, peripheral: SEMC, signal: semc_dqs, pin_signal: GPIO_EMC_39}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitSDRAMPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitSDRAMPins(void)
{
    CLOCK_EnableClock(kCLOCK_Iomuxc);

    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_00_SEMC_DATA00, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_01_SEMC_DATA01, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_02_SEMC_DATA02, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_03_SEMC_DATA03, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_04_SEMC_DATA04, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_05_SEMC_DATA05, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_06_SEMC_DATA06, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_07_SEMC_DATA07, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_08_SEMC_DM00, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_09_SEMC_ADDR00, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_10_SEMC_ADDR01, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_11_SEMC_ADDR02, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_12_SEMC_ADDR03, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_13_SEMC_ADDR04, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_14_SEMC_ADDR05, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_15_SEMC_ADDR06, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_16_SEMC_ADDR07, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_17_SEMC_ADDR08, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_18_SEMC_ADDR09, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_19_SEMC_ADDR11, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_20_SEMC_ADDR12, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_21_SEMC_BA0, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_22_SEMC_BA1, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_23_SEMC_ADDR10, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_24_SEMC_CAS, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_25_SEMC_RAS, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_26_SEMC_CLK, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_27_SEMC_CKE, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_28_SEMC_WE, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_29_SEMC_CS0, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_30_SEMC_DATA08, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_31_SEMC_DATA09, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_32_SEMC_DATA10, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_33_SEMC_DATA11, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_34_SEMC_DATA12, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_35_SEMC_DATA13, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_36_SEMC_DATA14, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_37_SEMC_DATA15, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_38_SEMC_DM01, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_39_SEMC_DQS, 0U);
}

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitENETPins:
- options: {callFromInitBoot: 'false', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: A7, peripheral: ENET, signal: enet_mdc, pin_signal: GPIO_EMC_40, software_input_on: Disable,
hysteresis_enable: Disable, pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable,
open_drain: Disable, speed: MHZ_200, drive_strength: R0_5, slew_rate: Fast}
  - {pin_num: C7, peripheral: ENET, signal: enet_mdio, pin_signal: GPIO_EMC_41, software_input_on: Disable,
hysteresis_enable: Disable, pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable,
open_drain: Enable, speed: MHZ_50, drive_strength: R0_5, slew_rate: Fast}
  - {pin_num: B13, peripheral: ENET, signal: enet_ref_clk, pin_signal: GPIO_B1_10, software_input_on: Enable,
hysteresis_enable: Disable, pull_up_down_config: Pull_Down_100K_Ohm, pull_keeper_select: Keeper, pull_keeper_enable:
Disable, open_drain: Disable, speed: MHZ_50, drive_strength: R0_6, slew_rate: Fast}
  - {pin_num: E12, peripheral: ENET, signal: 'enet_rx_data, 0', pin_signal: GPIO_B1_04, software_input_on: Disable,
hysteresis_enable: Disable, pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable,
open_drain: Disable, speed: MHZ_200, drive_strength: R0_5, slew_rate: Fast}
  - {pin_num: D12, peripheral: ENET, signal: 'enet_rx_data, 1', pin_signal: GPIO_B1_05, software_input_on: Disable,
hysteresis_enable: Disable, pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable,
open_drain: Disable, speed: MHZ_200, drive_strength: R0_5, slew_rate: Fast}
  - {pin_num: C12, peripheral: ENET, signal: enet_rx_en, pin_signal: GPIO_B1_06, software_input_on: Disable,
hysteresis_enable: Disable, pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable,
open_drain: Disable, speed: MHZ_200, drive_strength: R0_5, slew_rate: Fast}
  - {pin_num: C13, peripheral: ENET, signal: enet_rx_er, pin_signal: GPIO_B1_11, software_input_on: Disable,
hysteresis_enable: Disable, pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable,
open_drain: Disable, speed: MHZ_200, drive_strength: R0_5, slew_rate: Fast}
  - {pin_num: B12, peripheral: ENET, signal: 'enet_tx_data, 0', pin_signal: GPIO_B1_07, software_input_on: Disable,
hysteresis_enable: Disable, pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable,
open_drain: Disable, speed: MHZ_200, drive_strength: R0_5, slew_rate: Fast}
  - {pin_num: A12, peripheral: ENET, signal: 'enet_tx_data, 1', pin_signal: GPIO_B1_08, software_input_on: Disable,
hysteresis_enable: Disable, pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable,
open_drain: Disable, speed: MHZ_200, drive_strength: R0_5, slew_rate: Fast}
  - {pin_num: A13, peripheral: ENET, signal: enet_tx_en, pin_signal: GPIO_B1_09, software_input_on: Disable,
hysteresis_enable: Disable, pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable,
open_drain: Disable, speed: MHZ_200, drive_strength: R0_5, slew_rate: Fast}
  - {pin_num: F14, peripheral: GPIO1, signal: 'gpio_io, 09', pin_signal: GPIO_AD_B0_09, software_input_on: Disable,
hysteresis_enable: Disable, pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable,
open_drain: Disable, speed: MHZ_100, drive_strength: R0_5, slew_rate: Fast}
  - {pin_num: G13, peripheral: GPIO1, signal: 'gpio_io, 10', pin_signal: GPIO_AD_B0_10, software_input_on: Disable,
hysteresis_enable: Disable, pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable,
open_drain: Disable, speed: MHZ_100, drive_strength: R0_5, slew_rate: Fast}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitENETPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitENETPins(void)
{
    CLOCK_EnableClock(kCLOCK_Iomuxc);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_09_GPIO1_IO09, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_10_GPIO1_IO10, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_04_ENET_RX_DATA00, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_05_ENET_RX_DATA01, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_06_ENET_RX_EN, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_07_ENET_TX_DATA00, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_08_ENET_TX_DATA01, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_09_ENET_TX_EN, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_10_ENET_REF_CLK, 1U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_11_ENET_RX_ER, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_40_ENET_MDC, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_41_ENET_MDIO, 0U);
    IOMUXC_GPR->GPR26 = ((IOMUXC_GPR->GPR26 & (~(BOARD_INITENETPINS_IOMUXC_GPR_GPR26_GPIO_MUX1_GPIO_SEL_MASK))) |
                         IOMUXC_GPR_GPR26_GPIO_MUX1_GPIO_SEL(0x00U));
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_09_GPIO1_IO09, 0xB0A9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_10_GPIO1_IO10, 0xB0A9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_04_ENET_RX_DATA00, 0xB0E9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_05_ENET_RX_DATA01, 0xB0E9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_06_ENET_RX_EN, 0xB0E9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_07_ENET_TX_DATA00, 0xB0E9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_08_ENET_TX_DATA01, 0xB0E9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_09_ENET_TX_EN, 0xB0E9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_10_ENET_REF_CLK, 0x31U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_11_ENET_RX_ER, 0xB0E9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_EMC_40_ENET_MDC, 0xB0E9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_EMC_41_ENET_MDIO, 0xB829U);
}

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitUSDHCPins:
- options: {callFromInitBoot: 'false', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: J2, peripheral: USDHC1, signal: 'usdhc_data, 3', pin_signal: GPIO_SD_B0_05, hysteresis_enable: Disable,
pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable, open_drain: Disable, speed:
MHZ_100, drive_strength: R0_7, slew_rate: Fast}
  - {pin_num: H2, peripheral: USDHC1, signal: 'usdhc_data, 2', pin_signal: GPIO_SD_B0_04, hysteresis_enable: Disable,
pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable, open_drain: Disable, speed:
MHZ_100, drive_strength: R0_7, slew_rate: Fast}
  - {pin_num: K1, peripheral: USDHC1, signal: 'usdhc_data, 1', pin_signal: GPIO_SD_B0_03, hysteresis_enable: Disable,
pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable, open_drain: Disable, speed:
MHZ_100, drive_strength: R0_7, slew_rate: Fast}
  - {pin_num: J1, peripheral: USDHC1, signal: 'usdhc_data, 0', pin_signal: GPIO_SD_B0_02, hysteresis_enable: Disable,
pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable, open_drain: Disable, speed:
MHZ_100, drive_strength: R0_7, slew_rate: Fast}
  - {pin_num: J4, peripheral: USDHC1, signal: usdhc_cmd, pin_signal: GPIO_SD_B0_00, hysteresis_enable: Disable,
pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, pull_keeper_enable: Enable, open_drain: Disable, speed:
MHZ_100, drive_strength: R0_7, slew_rate: Fast}
  - {pin_num: J3, peripheral: USDHC1, signal: usdhc_clk, pin_signal: GPIO_SD_B0_01, hysteresis_enable: Disable,
pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Keeper, pull_keeper_enable: Disable, open_drain: Disable,
speed: MHZ_100, drive_strength: R0_7, slew_rate: Fast}
  - {pin_num: C14, peripheral: USDHC1, signal: usdhc_vselect, pin_signal: GPIO_B1_14, hysteresis_enable: Disable,
pull_up_down_config: Pull_Up_100K_Ohm, pull_keeper_select: Pull, drive_strength: R0_7, slew_rate: Fast}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitUSDHCPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitUSDHCPins(void)
{
    CLOCK_EnableClock(kCLOCK_Iomuxc);

    IOMUXC_SetPinMux(IOMUXC_GPIO_B1_14_USDHC1_VSELECT, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_00_USDHC1_CMD, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_01_USDHC1_CLK, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_02_USDHC1_DATA0, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_03_USDHC1_DATA1, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_04_USDHC1_DATA2, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_05_USDHC1_DATA3, 0U);
#if !defined(SDIO_ENABLED)
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_14_USDHC1_VSELECT, 0xB0B9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_00_USDHC1_CMD, 0xB0B9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_01_USDHC1_CLK, 0x80B9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_02_USDHC1_DATA0, 0xB0B9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_03_USDHC1_DATA1, 0xB0B9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_04_USDHC1_DATA2, 0xB0B9U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_05_USDHC1_DATA3, 0xB0B9U);
#else
    IOMUXC_SetPinConfig(IOMUXC_GPIO_B1_14_USDHC1_VSELECT, 0x0170A1U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_00_USDHC1_CMD, 0x0170A1U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_01_USDHC1_CLK, 0x0140A1U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_02_USDHC1_DATA0, 0x0170A1U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_03_USDHC1_DATA1, 0x0170A1U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_04_USDHC1_DATA2, 0x0170A1U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_05_USDHC1_DATA3, 0x0170A1U);
#endif
}

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitQSPIPins:
- options: {callFromInitBoot: 'false', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: P3, peripheral: FLEXSPI, signal: FLEXSPI_A_DATA0, pin_signal: GPIO_SD_B1_08}
  - {pin_num: N4, peripheral: FLEXSPI, signal: FLEXSPI_A_DATA1, pin_signal: GPIO_SD_B1_09}
  - {pin_num: P4, peripheral: FLEXSPI, signal: FLEXSPI_A_DATA2, pin_signal: GPIO_SD_B1_10}
  - {pin_num: P5, peripheral: FLEXSPI, signal: FLEXSPI_A_DATA3, pin_signal: GPIO_SD_B1_11}
  - {pin_num: L4, peripheral: FLEXSPI, signal: FLEXSPI_A_SCLK, pin_signal: GPIO_SD_B1_07}
  - {pin_num: L3, peripheral: FLEXSPI, signal: FLEXSPI_A_SS0_B, pin_signal: GPIO_SD_B1_06}
  - {pin_num: N3, peripheral: FLEXSPI, signal: FLEXSPI_A_DQS, pin_signal: GPIO_SD_B1_05}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitQSPIPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitQSPIPins(void)
{
    CLOCK_EnableClock(kCLOCK_Iomuxc);

    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_05_FLEXSPIA_DQS, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_06_FLEXSPIA_SS0_B, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_07_FLEXSPIA_SCLK, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_08_FLEXSPIA_DATA00, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_09_FLEXSPIA_DATA01, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_10_FLEXSPIA_DATA02, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_11_FLEXSPIA_DATA03, 0U);
}

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitOTWPins:
- options: {callFromInitBoot: 'false', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: L13, peripheral: GPIO1, signal: 'gpio_io, 26', pin_signal: GPIO_AD_B1_10, direction: OUTPUT,
gpio_init_state: 'true'}
  - {pin_num: M11, peripheral: GPIO1, signal: 'gpio_io, 27', pin_signal: GPIO_AD_B1_11, direction: OUTPUT,
gpio_init_state: 'true'}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitOTWPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitOTWPins(void)
{
    CLOCK_EnableClock(kCLOCK_Iomuxc);

    /* This is GPIO pin used for K32W0x1 RCP ISP Entry/DIO5 pin */
    /* GPIO configuration on GPIO_AD_B1_10 (pin L13) */
    gpio_pin_config_t gpio1_pinL13_config = {
        .direction = kGPIO_DigitalOutput, .outputLogic = 1U, .interruptMode = kGPIO_NoIntmode};
    /* Initialize GPIO functionality on GPIO_AD_B1_10 (pin L13) */
    GPIO_PinInit(GPIO1, 26U, &gpio1_pinL13_config);

    /* This is GPIO pin used for K32W0x1 RCP Reset */
    /* GPIO configuration on GPIO_AD_B1_11 (pin J13) */
    gpio_pin_config_t gpio1_pinJ13_config = {
        .direction = kGPIO_DigitalOutput, .outputLogic = 1U, .interruptMode = kGPIO_NoIntmode};
    /* Initialize GPIO functionality on GPIO_AD_B1_11 (pin J13) */
    GPIO_PinInit(GPIO1, 27U, &gpio1_pinJ13_config);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_10_GPIO1_IO26, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_11_GPIO1_IO27, 0U);
    IOMUXC_GPR->GPR26 = ((IOMUXC_GPR->GPR26 & (~(BOARD_INITPINS_IOMUXC_GPR_GPR26_GPIO_MUX1_GPIO_SEL_MASK))) |
                         IOMUXC_GPR_GPR26_GPIO_MUX1_GPIO_SEL(0x00U));
}

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitArduinoUARTPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: J12, peripheral: LPUART3, signal: TX, pin_signal: GPIO_AD_B1_06, software_input_on: no_init,
hysteresis_enable: no_init, pull_up_down_config: Pull_Down_100K_Ohm, pull_keeper_select: Keeper, pull_keeper_enable:
Enable, open_drain: Disable, speed: MHZ_100, drive_strength: R0_3}
  - {pin_num: K12, peripheral: LPUART3, signal: RTS_B, pin_signal: GPIO_AD_B1_05, hysteresis_enable: Disable,
pull_up_down_config: Pull_Down_100K_Ohm, pull_keeper_select: Keeper, pull_keeper_enable: Enable, open_drain: Disable,
speed: MHZ_100}
  - {pin_num: L12, peripheral: LPUART3, signal: CTS_B, pin_signal: GPIO_AD_B1_04, hysteresis_enable: Disable,
pull_up_down_config: Pull_Down_100K_Ohm, pull_keeper_select: Keeper, pull_keeper_enable: Enable, open_drain: Disable,
speed: MHZ_100}
  - {pin_num: K10, peripheral: LPUART3, signal: RX, pin_signal: GPIO_AD_B1_07, pull_up_down_config: Pull_Down_100K_Ohm,
pull_keeper_select: Keeper, pull_keeper_enable: Enable, open_drain: Disable, speed: MHZ_100}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitArduinoUARTPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitArduinoUARTPins(void)
{
    CLOCK_EnableClock(kCLOCK_Iomuxc);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_04_LPUART3_CTS_B, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_05_LPUART3_RTS_B, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_06_LPUART3_TX, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_07_LPUART3_RX, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_04_LPUART3_CTS_B, 0x10B0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_05_LPUART3_RTS_B, 0x10B0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_06_LPUART3_TX, 0x1098U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_07_LPUART3_RX, 0x10B0U);
}

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitMurataModulePins:
- options: {callFromInitBoot: 'false', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: G10, peripheral: GPIO1, signal: 'gpio_io, 11', pin_signal: GPIO_AD_B0_11, direction: OUTPUT}
  - {pin_num: H13, peripheral: GPIO1, signal: 'gpio_io, 24', pin_signal: GPIO_AD_B1_08, direction: OUTPUT}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitMurataModulePins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitMurataModulePins(void)
{
    CLOCK_EnableClock(kCLOCK_Iomuxc);

    /* GPIO configuration of INT2_COMBO on GPIO_AD_B0_11 (pin G10) */
    gpio_pin_config_t INT2_COMBO_config = {
        .direction = kGPIO_DigitalOutput, .outputLogic = 0U, .interruptMode = kGPIO_NoIntmode};
    /* Initialize GPIO functionality on GPIO_AD_B0_11 (pin G10) */
    GPIO_PinInit(GPIO1, 11U, &INT2_COMBO_config);

    /* GPIO configuration of CSI_D9 on GPIO_AD_B1_08 (pin H13) */
    gpio_pin_config_t CSI_D9_config = {
        .direction = kGPIO_DigitalOutput, .outputLogic = 0U, .interruptMode = kGPIO_NoIntmode};
    /* Initialize GPIO functionality on GPIO_AD_B1_08 (pin H13) */
    GPIO_PinInit(GPIO1, 24U, &CSI_D9_config);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_11_GPIO1_IO11, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_08_GPIO1_IO24, 0U);
    IOMUXC_GPR->GPR26 =
        ((IOMUXC_GPR->GPR26 & (~(BOARD_INITMURATAMODULEPINS_IOMUXC_GPR_GPR26_GPIO_MUX1_GPIO_SEL_MASK))) |
         IOMUXC_GPR_GPR26_GPIO_MUX1_GPIO_SEL(0x00U));
}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
