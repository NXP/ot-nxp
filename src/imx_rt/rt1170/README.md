# OpenThread on NXP RT1170 (host) + transceiver (rcp) example

This directory contains example platform drivers for the [NXP RT1170][rt1170] platform.

The example platform drivers are intended to present the minimal code necessary to support OpenThread. As a result, the example platform drivers do not necessarily highlight the platform's full capabilities.

[rt1170]: https://www.nxp.com/products/processors-and-microcontrollers/arm-microcontrollers/i-mx-rt-crossover-mcus/i-mx-rt1170-crossover-mcu-family-first-ghz-mcu-with-arm-cortex-m7-and-cortex-m4-cores:i.MX-RT1170?cid=ad_PRG4692582_TAC476846_EETECH_IMXRT1170&gclid=EAIaIQobChMIvr3xrYzT8QIVTgKLCh3GGQ80EAAYAiAAEgLnYvD_BwE

## Configuration(s) supported

Here are listed configurations that allow to support Openthread on RT1170:

- RT1170 + IWX12

## Prerequisites

Before you start building the examples, you must download and install the toolchain and the tools required for flashing and debugging.

## Toolchain

OpenThread environment is suited to be run on a Linux-based OS.

In a Bash terminal (found, for example, in Ubuntu OS), follow these instructions to install the GNU toolchain and other dependencies.

```bash
$ cd <path-to-ot-nxp>
$ ./script/bootstrap
```

## Tools

- Download and install the [MCUXpresso IDE][mcuxpresso ide].

[mcuxpresso ide]: https://www.nxp.com/support/developer-resources/software-development-tools/mcuxpresso-software-and-tools/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE

- Download [IMXRT1170 SDK 2.13.0_firecrest_EAR4.6](https://mcuxpresso.nxp.com/).
  For internal SDK delivery, the SDK can be downloaded using the [KEX STAGE](https://kex-stage.nxp.com/en/welcome)
  Creating an nxp.com account is required before being able to download the
  SDK. Once the account is created, login and follow the steps for downloading
  SDK_2.13.0_EVK-MIMXRT1170. In the SDK Builder UI selection you should select
  the **FreeRTOS component**, the **BT/BLE component** and the **ARM GCC Toolchain**.
  SDK could also be retrieved with cloning [Internal SDK repository](https://bitbucket.sw.nxp.com/projects/MCUCORE/repos/mcu-sdk-2.0/browse)
  and setting branch develop/2.13.0_firecrest_v16 .

## Building examples

```bash
$ cd <path-to-ot-nxp>
$ export NXP_RT1170_SDK_ROOT=/path/to/previously/downloaded/SDK
$ ./script/build_rt1170
```
Note : If the EVK-MIMXRT1170 board is used instead of MIMXRT1170-EVK**B**, make sure
to specify it using the following build command :

```bash
$ export NXP_RT1170_SDK_ROOT=/path/to/previously/downloaded/SDK
$ ./script/build_rt1170 -DEVK_RT1170_BOARD="evkmimxrt1170"
```

After a successful build, the ot-cli-rt1170 FreeRTOS version could be found in `build_rt1170` and include FTD (Full Thread Device).

Note: FreeRTOS is required to be able to build the IMXRT1170 platform files. By default, if no argument is given when running the script `build_rt1170`, a freertos ot cli will be created to suport the following configurations:

- RT1170 + IWX12 with spinel over SPI (binaries located in `build_rt1170/iwx12_spi`)

## Hardware requirements RT1170 + IWX12

Host part:

- 1 EVKB-MIMXRT1170

Transceiver part:

- 1 [Murata WIFI 2EL M.2 module](https://www.embeddedartists.com/products/2el-m-2-module/)

Optional uSD-M2 Adapter:

- 1 [Murata uSD-M2 Adapter](https://www.murata.com/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter)

## Hardware rework for SPI support
If you connect IW612 module to **sdcard slot** using adaptor **no rework** is needed. Following reworks are necessary
to use IW612 connected directly to onboard m.2 slot.

- EVK-MIMXRT1170 **(Not tested)**
  - Remove 0Ω resistors R187, R195, R208, R764, (R78 maybe).
  - Populate 0Ω resistor R404.
  - Connect jumpers J63, J64, J65, J66.
- EVK**B**-MIMXRT1170
  - Remove 0Ω resistors R187, R195, R208, R1923.
  - Populate 0Ω resistor R404.
  - Make sure J113 is removed.
  - Connect jumpers J63, J64, J65, J66. 

## Board settings

### RT1170 + IW612 (Spinel over SPI)

The below table explains pin settings (SPI settings) to connect the evkmimxrt1170 and evkbmimxrt1170
(host) to a IW612 transceiver (Murata 2EL module).

All signals below must be routed through level shifter because 1170 uses 3.3V logic and IW612 module 
uses 1.8V logic. Recommended level shifter is 74LVC245.

#### IW612 connected directly to on board m.2 slot:
| PIN NAME OF IWX12 | IWX12 JP1 | I.MXRT1170  | PIN NAME OF RT1170 | GPIO NAME OF RT1170 |
|:-----------------:|:---------:|:-----------:|:------------------:|:-------------------:|
|        CLK        |     2     | J26, pin 9  |     LPSPI6_SCK     |    GPIO_LPSR_10     |
|       SSEL        |     3     | J26, pin 11 |    LPSPI6_PCS0     |    GPIO_LPSR_09     |
|       MOSI        |     4     | J26, pin 7  |     LPSPI6_SDO     |    GPIO_LPSR_11     |
|       MISO        |     5     | J26, pin 5  |     LPSPI6_SDI     |    GPIO_LPSR_12     |
|        INT        |     8     | J26, pin 4  |     GPIO_AD_11     |     GPIO_AD_11      |

#### IW612 connected to sd card slot using adaptor:
| PIN NAME OF IWX12 | IWX12 JP1 | I.MXRT1170  | PIN NAME OF RT1170 | GPIO NAME OF RT1170 |
|:-----------------:|:---------:|:-----------:|:------------------:|:-------------------:|
|        CLK        |     2     | J10, pin 12 |     LPSPI1_SCK     |     GPIO_AD_28      |
|       SSEL        |     3     | J10, pin 6  |    LPSPI1_PCS0     |     GPIO_AD_29      |
|       MOSI        |     4     | J10, pin 8  |     LPSPI1_SDO     |     GPIO_AD_30      |
|       MISO        |     5     | J10, pin 10 |     LPSPI1_SDI     |     GPIO_AD_31      |
|        INT        |     8     | J26, pin 4  |     GPIO_AD_11     |     GPIO_AD_11      |

Connect signals from IW612 module to Ax signals on level shifter and then corresponding Bx signals 
to arduino header on 1170 board. Level shifter then needs following additional connections:

| Lvl shifter pin |  I.MXRT1170  |
|:---------------:|:------------:|
|       VA        | J53 (jumper) |
|       VB        | J10, pin 16  |
|       GND       |  J26, pin 1  |
 
Make sure the OE pin on level shifter is connected to VA or VB (for example 
[Adafruit module TXB0108](https://www.adafruit.com/product/395) has already done that).

### RT1170 + K32W061 (Spinel over UART) - Warning: No longer maintain

The below table explains pin settings (UART settings) to connect the evkmimxrt1170 (host) to a k32w061 transceiver (rcp).

| PIN NAME OF K32W061 | DK6 (K32W061) | I.MXRT1170  | PIN NAME OF RT1170 | GPIO NAME OF RT1170 |
| :-----------------: | :-----------: | :---------: | :----------------: | :-----------------: |
|      UART_TXD       |     PIO 8     | J25, pin 13 |    LPUART7_RXD     |     GPIO_AD_01      |
|      UART_RXD       |     PIO 9     | J25, pin 15 |    LPUART7_TXD     |     GPIO_AD_00      |
|      UART_RTS       |     PIO 6     | J25, pin 11 |    LPUART7_CTS     |     GPIO_AD_02      |
|      UART_CTS       |     PIO 7     | J25, pin 9  |    LPUART7_RTS     |     GPIO_AD_03      |
|         GND         |   J3, pin 1   | J10, pin 14 |         XX         |         XX          |
|        RESET        |     RSTN      | J26, pin 2  |     GPIO_AD_10     |     GPIO_AD_10      |

### RT1170 + K32W061 (Spinel over SPI) - Warning: No longer maintain

The below table explains pin settings (SPI settings) to connect the evkmimxrt1170 (host) to a k32w061 transceiver (rcp).

| PIN NAME OF K32W061 | DK6 (K32W061) | I.MXRT1170  | PIN NAME OF RT1170 | GPIO NAME OF RT1170 |
| :-----------------: | :-----------: | :---------: | :----------------: | :-----------------: |
|         SIN         |    PIO 17     | J10, pin 8  |    LPSPI1_SOUT     |     GPIO_AD_30      |
|        SOUT         |    PIO 18     | J10, pin 10 |     LPSPI1_SIN     |     GPIO_AD_31      |
|        PCS0         |    PIO 16     | J10, pin 6  |    LPSPI1_PCS0     |     GPIO_AD_29      |
|         SCK         |    PIO 15     | J10, pin 12 |     LPSPI1_SCK     |     GPIO_AD_28      |
|         GND         |   J3, pin 1   | J10, pin 14 |         XX         |         XX          |
|        RESET        |     RSTN      | J26, pin 2  |     GPIO_AD_10     |     GPIO_AD_10      |
|       SPI_INT       |    PIO 19     | J26, pin 4  |     GPIO_AD_11     |     GPIO_AD_11      |

## Flash Binaries

### Flashing the IWX12 transceiver firmware

At each boot the RT1170 will automatically download the IWX12 firmware on the board via SDIO.

### Flashing the IMXRT ot-cli-rt1170 host image using MCUXpresso IDE

In order to flash the application for debugging we recommend using [MCUXpresso IDE (version >= 11.3.1)](https://www.nxp.com/design/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE?tab=Design_Tools_Tab).

- Import the previously downloaded NXP SDK into MCUXpresso IDE. This can be done by drag-and-dropping the SDK archive into MCUXpresso IDE.
- Follow the same procedure as described in [OpenThread on RT1060 examples][rt1060-page] in section "Flashing the IMXRT ot-cli-rt1060 host image using MCUXpresso IDE". Instead of selecting the RT1060 MCU, the RT1170 MCU should be chosen.

[rt1060-page]: ../rt1060/README.md

## Running the example

1. The CLI example uses UART connection. To view raw UART output, start a terminal emulator like PuTTY and connect to the used COM port with the following UART settings (on the IMXRT1170):

   - Baud rate: 115200
   - 8 data bits
   - 1 stop bit
   - No parity
   - No flow control

2. Follow the process describe in [Interact with the OT CLI][validate_port].

[validate_port]: https://openthread.io/guides/porting/validate-the-port#interact-with-the-cli

For a list of all available commands, visit [OpenThread CLI Reference README.md][cli].

[cli]: https://github.com/openthread/openthread/blob/master/src/cli/README.md

## Known issues

- Factory reset issue when the board is attacted to MCUXpresso debugguer: before running the factory reset command the debugguer needs to be detached.
