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
$ git submodule update --init
$ ./script/bootstrap
```

## Tools

- Download and install the [MCUXpresso IDE][mcuxpresso ide].

[mcuxpresso ide]: https://www.nxp.com/support/developer-resources/software-development-tools/mcuxpresso-software-and-tools/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE

- Download [the SDK from the link.](https://mcuxpresso.nxp.com/). Creating an nxp.com account is required before being able to download the SDK. S Select the **MIMXRT1170-EVKB** board. Select the SDK version 2.13.3 .
  In the SDK Builder UI selection you should select at least
  the **FreeRTOS component**, the **BT/BLE component** and the **ARM GCC Toolchain**.

## Building examples

```bash
$ cd <path-to-ot-nxp>
$ export NXP_RT1170_SDK_ROOT=/path/to/previously/downloaded/SDK
$ ./script/build_rt1170
```

After a successful build, the ot-cli-rt1170 FreeRTOS version could be found in `build_rt1170` and include FTD (Full Thread Device).

Note: FreeRTOS is required to be able to build the IMXRT1170 platform files. By default, if no argument is given when running the script `build_rt1170`, a freertos ot cli will be created to support the following configurations:

- RT1170 + IWX12 with spinel over SPI (binaries located in `build_rt1170/iwx12_spi`)

## Hardware requirements RT1170 + IWX12

Host part:

- 1 EVKB-MIMXRT1170
  ![](../../../doc/img/imxrt1170/IMX-RT1170-EVK-TOP.jpg)

Transceiver part:

Transceiver parts :

- 1 [2EL M2 A1 IW612 Secure Module](https://www.nxp.com/products/wireless/wi-fi-plus-bluetooth-plus-802-15-4/2-4-5-ghz-dual-band-1x1-wi-fi-6-802-11ax-plus-bluetooth-5-2-plus-802-15-4-tri-radio-solution:IW612)

![](../../../doc/img/imxrt1170/iwx612_2EL.jpg)

- 1 [Murata uSD to M2 adapter](https://www.murata.com/en-eu/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter)

![](../../../doc/img/imxrt1170/murata_usd-M2_adapter.jpg)

- TXS0108E level shifter module.

![](../../../doc/img/imxrt1170/level_shifter.jpg)

- Male to female Burg cables – 20 number’s

### Hardware rework for SPI support on EVKB-MIMXRT1170

To support SPI on the EVKB-MIMXRT1170 board, it is required to remove 0Ω resistors R404,R406.

### Hardware rework to connect SPI on 2EL M2 IW612 Module

- Solder burg wires (male to female) at JP1 for SPI interface.

![](../../../doc/img/imxrt1170/soldering_SPI_on_IW612-2EL.jpg)
![](../../../doc/img/imxrt1170/soldering_SPI_on_IW612-2EL_after.jpg)

- Solder 2X10 Berg Pins to TXS0108E connector.
- Solder 10 K ohm resistor between OE and GND.
- Connect OE and VA

![](../../../doc/img/imxrt1170/level_shifter_soldering.jpg)

### Board settings (Spinel over SPI)

- Murata uSD to M2 adapter connections description:

![](../../../doc/img/imxrt1170/murata_usd-m2_connections_1.jpg)

![](../../../doc/img/imxrt1170/murata_usd-m2_connections_2.jpg)

- SPI connection between RT1170 to TXS0108E level shifter

|  MIMXRT1170-EVKB  | TXS0108E |
| :---------------: | :------: |
| VDD_3V3 (J10_16)  |    VB    |
| SPI_MOSI (J10.8)  |    B1    |
| SPI_MISO (J10.10) |    B2    |
| SPI_CLK (J10.12)  |    B3    |
|  SPI_CS (J10.6)   |    B4    |
|  SPI_INT (J26.4)  |    B5    |
|   GND (J10.14)    |   GND    |

- SPI line connection between IWX612 2EL M2 Module to TXS0108E level shifter

|  IWX612 2EL M2   | TXS0108E |
| :--------------: | :------: |
| 1.8V_REF(J13.3)  |    VA    |
| SPI_MOSI (JP1.4) |    A1    |
| SPI_MISO (JP1.5) |    A2    |
| SPI_CLK (JP1.2)  |    A3    |
| SPI_SSEL (JP1.3) |    A4    |
| SPI_INT (JP1.8)  |    A5    |

- Reset line connection between RT1170 and uSD-M2 adapter

| MIMXRT1170-EVKB | Murata uSD-M2 |
| :-------------: | :-----------: |
|  RESET (J26.2)  |     J9.3      |
|   GND (J26.1)   |     J7.6      |

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
