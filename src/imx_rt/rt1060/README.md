# OpenThread on NXP RT1060 (host) + K32W061 (rcp) example

This directory contains example platform drivers for the [NXP RT1060][rt1060]
platform.

The example platform drivers are intended to present the minimal code necessary
to support OpenThread. As a result, the example platform drivers do not
necessarily highlight the platform's full capabilities.

[rt1060]: https://www.nxp.com/products/processors-and-microcontrollers/arm-microcontrollers/i-mx-rt-crossover-mcus/i-mx-rt1060-crossover-mcu-with-arm-cortex-m7-core:i.MX-RT1060

## Prerequisites

Before you start building the examples, you must download and install the
toolchain and the tools required for flashing and debugging.

## Toolchain

OpenThread environment is suited to be run on a Linux-based OS.

In a Bash terminal (found, for example, in Ubuntu OS), follow these instructions
to install the GNU toolchain and other dependencies.

```bash
$ cd <path-to-ot-nxp>
$ ./script/bootstrap
```

## Tools

- Download and install the [MCUXpresso IDE][mcuxpresso ide].

[mcuxpresso ide]: https://www.nxp.com/support/developer-resources/software-development-tools/mcuxpresso-software-and-tools/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE

- Download the NXP MCUXpresso git SDK 2.13.0
  and associated middleware from GitHub using the west tool.

```
bash
$ cd third_party/rt1060_sdk/repo
$ west init -l manifest --mf west.yml
$ west update
```

In case there are local modification to the already installed git NXP SDK. Use the west forall command instead of the west init to reset the west workspace before running the west update command. Warning: all local changes will be lost after running this command.

```
bash
$ cd third_party/rt1060_sdk/repo
$ west forall -c "git reset --hard && git clean -xdf" -a
```

## Building the examples

```bash
$ cd <path-to-ot-nxp>
$ ./script/build_rt1060
```

Note : If the EVK-MIMXRT1060 board is used instead of MIMXRT1060-EVKB, make sure
to specify it using the following build command :

```bash
$ ./script/build_rt1060 -DEVK_RT1060_BOARD="evkmimxrt1060"
```

After a successful build, the ot-cli-rt1060 FreeRTOS version could be found in
`build_rt1060` and include FTD (Full Thread Device).

Notes:

- The K32W0 RCP firmware should be built before building the RT1060 image.
- FreeRTOS is required to be able to build the IMXRT1060 platform files.

## Hardware requirements

Host part:

- 1 MIMXRT1060-EVKB

Transceiver part:

- 1 OM15076-3 Carrier Board (DK6 board)
- 1 K32W061 Module to be plugged on the Carrier Board

Note : The pin connections between the boards are slightly different according
to which version of the board is used (MIMXRT1060-EVKB or EVK-MIMXRT1060).The
different settings are described below.

## Board settings for MIMXRT1060-EVKB

The below table explains pin settings (UART settings) to connect the
evkbmimxrt1060 (host) to a k32w061 transceiver (rcp).

|    PIN NAME    | DK6 (K32W061) | I.MXRT1060-EVKB | I.MXRT1060-EVK | PIN NAME OF RT1060 | GPIO NAME OF RT1060 |
| :------------: | :-----------: | :-------------: | :------------: | :----------------: | :-----------------: |
|    UART_TXD    |  PIO, pin 8   |   J16, pin 1    |   J22, pin 1   |    LPUART3_RXD     |    GPIO_AD_B1_07    |
|    UART_RXD    |  PIO, pin 9   |   J16, pin 2    |   J22, pin 2   |    LPUART3_TXD     |    GPIO_AD_B1_06    |
|    UART_RTS    |  PIO, pin 6   |   J33, pin 3    |   J23, pin 3   |    LPUART3_CTS     |    GPIO_AD_B1_04    |
|    UART_CTS    |  PIO, pin 7   |   J33, pin 4    |   J23, pin 4   |    LPUART3_RTS     |    GPIO_AD_B1_05    |
|      GND       |   J3, pin 1   |   J32, pin 7    |   J25, pin 7   |         XX         |         XX          |
|     RESET      |     RSTN      |   J17, pin 2    |   J24, pin 2   |   GPIO_AD_B0_02    |    GPIO_AD_B0_02    |
| DIO5/ISP Entry |  PIO, pin 5   |   J33, pin 1    |   J23, pin 1   |   GPIO_AD_B1_10    |    GPIO_AD_B1_10    |

The below picture shows pins connections for the EVK-MIMXRT1060.

![rt1060_k32w061_pin_settings](../../../doc/img/imxrt1060/rt1060_k32w061_pin_settings.jpg)

## Flash Binaries

### K32W061 OT-RCP transceiver image

An ot-rcp image has to be built. For that, it is recommended to follow the [K32W061
Readme][k32w061-readme].

To avoid to have to rebuild all K32W061 application, it is recommended to build only the ot_rcp version using the below command:

```bash
$ ./script/build_k32w061 ot_rcp_only_uart_flow_control
```

After a successful build, application binaries will be generated in
`ot-nxp/build_k32w061/rcp_only_uart_flow_control/bin` and would contain the file called "rcp_name.bin.h" that would be used by the RT1060 to download the K32W061 RCP firmware. In fact, the Over The Wire (OTW) protocol (over UART) would be used to download the k32w0 transceiver image from the RT1060 to the its internal flash.

[k32w061-readme]: ../../k32w0/k32w061/README.md
[sdk_mcux]: https://mcuxpresso.nxp.com/en/welcome

### Flashing the IMXRT ot-cli-rt1060 host image using MCUXpresso IDE

In order to flash the application for debugging we recommend using
[MCUXpresso IDE (version >= 11.3.1)](https://www.nxp.com/design/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE?tab=Design_Tools_Tab).

- Import the previously downloaded NXP SDK into MCUXpresso IDE. This can be
  done by drag-and-dropping the SDK archive into MCUXpresso IDE.

- Import ot-nxp repo in MCUXpresso IDE as Makefile Project. Use _none_ as
  _Toolchain for Indexer Settings_:

```
File -> Import -> C/C++ -> Existing Code as Makefile Project
```

- Configure MCU Settings:

```
Right click on the Project -> Properties -> C/C++ Build -> MCU Settings -> Select MIMXRT1060 -> Apply & Close
```

![MCU_Sett](../../../doc/img/imxrt1060/mcu_settings.JPG)

- Configure the toolchain editor:

```
Right click on the Project -> C/C++ Build-> Tool Chain Editor -> NXP MCU Tools -> Apply & Close
```

![MCU_Sett](../../../doc/img/k32w/toolchain.JPG)

- Create a debug configuration:

To create a new debug configuration for our application, we will duplicate an
existing debug configaturation.

- Create a debug configuration for the hello word projet

1. Click on "Import SDK example(s)..." on the bottom left window of MCUXpresso.
2. Select the "evkbmimxrt1060" SDK, click on "Next"
3. Expand "demo_apps", select the "hello_word" example, click on next and then
   finish.
4. Build the imported "Hello word" application by right clicking on the project
   and select "Build Project".
5. Right click again on the project and select "Debug As" and click on
   "MCUXpresso IDE LinkServer" option. Doing this will flash the application on
   the board. Then click on the red "Terminate" button.

- Duplicate the hello word debug configaturation to create a new debug
  configuration for the ot_cli

1. Right click on the "Hello Word" project, select "Debug As" and then select
   "Debug Configurations".
2. Right click on the "Hello Word" debug configuration and click on "Duplicate".
3. Rename the Duplicated debug configuration "ot-cli".
4. In the "C/C++ Application", click on "Browse" and select the ot-cli-rt1060.elf
   app (should be located in "ot-nxp/build_rt1060/bin/ot-cli-rt1060.elf"). Then click on
   Apply and Save.
5. Click on "Organize Favorites".
   ![MCU_Sett](../../../doc/img/imxrt1060/organize_favorites.png)
6. Add the ot-cli debug configuration
7. Run the ot-cli debug configuration

[cmsis-dap]: https://os.mbed.com/handbook/CMSIS-DAP

## Running the example

1. The CLI example uses UART connection. To view raw UART output, start a
   terminal emulator like PuTTY and connect to the used COM port with the
   following UART settings (on the IMXRT1060):

   - Baud rate: 115200
   - 8 data bits
   - 1 stop bit
   - No parity
   - No flow control

2. Follow the process describe in [Interact with the OT CLI][validate_port].

[validate_port]: https://openthread.io/guides/porting/validate-the-port#interact-with-the-cli

For a list of all available commands, visit [OpenThread CLI Reference
README.md][cli].

[cli]: https://github.com/openthread/openthread/blob/master/src/cli/README.md
