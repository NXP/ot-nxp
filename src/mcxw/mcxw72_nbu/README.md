# OpenThread on NXP MCXW72_NBU Example

This directory contains example platform drivers for the [NXP MCXW72][nxp_mcxw72] based on FRDM-MCXW72 hardware platform.

The example platform drivers are intended to present the minimal code necessary to support OpenThread. As a result, the example platform drivers do not necessarily highlight the platform's full capabilities.

[nxp_mcxw72]: https://www.nxp.com/products/processors-and-microcontrollers/arm-microcontrollers/general-purpose-mcus/mcx-arm-cortex-m/mcx-w-series-microcontrollers/mcx-w72x-secure-and-ultra-low-power-mcus-for-matter-thread-zigbee-and-bluetooth-le:MCX-W72X

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

Download the MCXW72_NBU SDK using the west tool.

```bash
$ cd <path-to-ot-nxp>
$ ./third_party/nxp_matter_support/scripts/update_nxp_sdk.py --platform common
```

In case there are local modification to the already installed git NXP SDK. Use the west forall command instead of the west init to reset the west workspace before running the west update command. Warning: all local changes will be lost after running this command.

```bash
$ cd third_party/nxp_matter_support/github_sdk/sdk_next/repo/mcuxsdk
$ west forall -c "git reset --hard && git clean -xdf" -a
```

## Building the examples

### Set the compilation parameters

Please set the ARMGCC_DIR and ZEPHYR_BASE parameters using the following commands:

- Linux

```bash
$ cd <path-to-ot-nxp>
$ export ARMGCC_DIR='your-own-armgcc-dir-path'
$ source ./third_party/nxp_matter_support/github_sdk/sdk_next/repo/mcuxsdk/mcux-env.sh
$ west mcuxsdk-export
```

- Windows

```cmd
> cd <path-to-ot-nxp>
> set ARMGCC_DIR=<path to ARMGCC>
> <SDK_path>mcuxsdk/mcux-env.cmd
```

Then run the build scripts:

- Linux

```bash
$ cd <path-to-ot-nxp>
$ ./script/build_mcxw72_nbu ot_without_rcp_image
$ west build -d build_ncp_basic -b mcxw72evk examples/helper/ncp_basic -Dcore_id=cm33_core0 --toolchain=armgcc
$ ./script/build_mcxw72_nbu ncp_ot_ble_ll
$ west build -d build_ncp_advanced -b mcxw72evk examples/helper/ncp_advanced -Dcore_id=cm33_core0 --toolchain=armgcc
```

- Windows

```cmd
> cd <path-to-ot-nxp>
> sh script/build_mcxw72_nbu ot_without_rcp_image
> west build -d build_ncp_basic -b mcxw72evk examples/helper/ncp_basic -Dcore_id=cm33_core0 --toolchain=armgcc
> sh script/build_mcxw72_nbu ncp_ot_ble_ll
> west build -d build_ncp_advanced -b mcxw72evk examples/helper/ncp_advanced -Dcore_id=cm33_core0 --toolchain=armgcc
```

After a successful build, the `elf` files are found in `build_mcxw72_nbu/bin` and include FTD (Full Thread Device) and MTD (Minimal Thread Device) variants of CLI applications.
In `build_ncp_basic` folder there is `ncp-basic_cm33_core0.elf`.
In `build_mcxw72_nbu/ncp_ot_ble_ll/bin` folder there is `ncp-ot-ble-ll-mcxw72_nbu.elf`.
In `build_ncp_advanced` folder there is `ncp-advanced_cm33_core0.elf`.

## Flashing

Two images must be written to the board: one for the host (CM33) (`ncp-basic_cm33_core0.elf`) and one for the NBU (CM33) (`ot-cli-ftd.elf`).
For NCP demo, you have to use `ncp-advanced_cm33_core0.elf` and `ncp-ot-ble-ll-mcxw72_nbu.elf`.

- Plug MCXW72 to the USB port

- copy the applications in the same folder that JLink executable is placed. Execute:

```bash
$  jlink -AutoConnect 1 -If SWD -Speed 4000 -NoGui 1 -Device KW47B42ZB7_CORE0 -SelectEmuBySN <SN>
$ reset
$ halt
$ erase
$ loadfile ncp-basic_cm33_core0.elf (or ncp-advanced_cm33_core0.elf)
$ r
$ g
$ q
```

- Unplug and plug back MCXW72 to the USB port

```bash
$  jlink -AutoConnect 1 -If SWD -Speed 4000 -NoGui 1 -Device KW47B42ZB7_CORE1 -SelectEmuBySN <SN>
$ reset
$ halt
$ loadfile ot-cli-ftd.elf (or ncp-ot-ble-ll-mcxw72_nbu.elf)
$ r
$ g
$ q
```

## Running the example

1. Prepare two boards with the flashed `CLI Example` (as shown above).
2. The CLI example uses UART connection. To view raw UART output, start a terminal emulator like PuTTY and connect to the used COM port with the following UART settings:

   - Baud rate: 115200
   - 8 data bits
   - 1 stop bit
   - No parity
   - No flow control

3. Open a terminal connection on the first board and start a new Thread network.

```bash
> factoryreset
Done
> dataset init new
Done
> dataset channel 17
Done
> dataset networkkey 00112233445566778899aabbccddeeff
Done
> dataset panid 0xabcd
Done
> dataset commit active
Done
> ifconfig up
Done
> thread start
Done
```

4. After a couple of seconds the node will become a Leader of the network.

```bash
> state
Leader
```

5. Open a terminal connection on the second board and attach a node to the network.

```bash
> factoryreset
Done
> dataset channel 17
Done
> dataset networkkey 00112233445566778899aabbccddeeff
Done
> dataset panid 0xabcd
Done
> dataset commit active
Done
> ifconfig up
Done
> thread start
Done
```

6. After a couple of seconds the second node will attach and become a Child.

```bash
> state
Child
```

7. List all IPv6 addresses of the first board.

```bash
> ipaddr
fdde:ad00:beef:0:0:ff:fe00:fc00
fdde:ad00:beef:0:0:ff:fe00:9c00
fdde:ad00:beef:0:4bcb:73a5:7c28:318e
fe80:0:0:0:5c91:c61:b67c:271c
```

8. Choose one of them and send an ICMPv6 ping from the second board.

```bash
> ping fdde:ad00:beef:0:0:ff:fe00:fc00
16 bytes from fdde:ad00:beef:0:0:ff:fe00:fc00: icmp_seq=1 hlim=64 time=8ms
```

For a list of all available commands, visit [OpenThread CLI Reference README.md][cli].

[cli]: https://github.com/openthread/openthread/blob/main/src/cli/README.md

## NCP demo

Same as above for the OpenThread.
You can also connect to the board over BLE from the IoT Toolbox phone app using Wireless UART option.
By doing so, you can type commands for the OpenThread CLI.
