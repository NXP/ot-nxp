# OpenThread on NXP K32W1 Example

This directory contains example platform drivers for the [NXP K32W1] based on [K32W148-EVK] hardware platform.

The example platform drivers are intended to present the minimal code necessary to support OpenThread. As a result, the example platform drivers do not necessarily highlight the platform's full capabilities.

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

- Download [K32W1 SDK 2.10.0](https://mcuxpresso.nxp.com/).
  Creating an nxp.com account is required before being able to download the
  SDK. Once the account is created, login and follow the steps for downloading
  SDK_2.10.0_K32W148-EVK. The SDK Builder UI selection should be similar with
  the one from the image below.
  ![MCUXpresso SDK Download](../../../doc/img/k32w1/mcux-sdk-download.JPG)

## Building the examples

```bash
$ cd <path-to-ot-nxp>
$ export NXP_K32W1_SDK_ROOT=/path/to/previously/downloaded/SDK
$ ./script/build_K32W1
```

After a successful build, the `elf` files are found in `build_K32W1/openthread/examples/apps/cli` and include FTD (Full Thread Device), MTD (Minimal Thread Device) and variants of CLI appliations.

## Extracting the binaries from the elf files

You can convert the `elf` files to binary format using arm-none-eabi-objcopy:

```bash
$ arm-none-eabi-objcopy -O binary -j.text build_K32W1/openthread/examples/apps/cli/ot-cli-ftd build_K32W1/openthread/examples/apps/cliot-cli-ftd.bin
```

## Flash Binaries

Only flashing of the binaries is supported right now using the MCULink drag and drop feature of the K32W148-EVK board.

### Using MCULink

To Flash the binary using MCULink just drag and drop the binary file onto the MCULink drive in File Explorer.

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
> panid 0xabcd
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
> panid 0xabcd
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
