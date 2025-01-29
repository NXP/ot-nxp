# OpenThread OT-CLI-FTD-BLE-SHELL

This example is a dual host application designed to enable and demonstrate BLE & 15.4 dynamic mode feature.
This application is supported for K32W1 and MCXW72.

## 1. Build

a. following environment variables need to be set before building application
ZEPHYR_BASE=<path to sdk>
ARMGCC_DIR=<path to compiler>

b. build for K32W1

```bash
$ cd <path-to-openthread>
$ sh script/build_k32w1 ot_cli_ftd_ble_shell --force
```

Application will be built in: <path-to-openthread>/build_k32w1/ot_cli_ftd_ble_shell/bin/ot-cli-ftd-ble-shell-k32w1.srec

c. build for MCXW72

```bash
$ cd <path-to-openthread>
$ sh script/build_mcxw72 ot_cli_ftd_ble_shell --force
```

Application will be built in: <path-to-openthread>/build_mcxw72/ot_cli_ftd_ble_shell/bin/ot-cli-ftd-ble-shell-mcxw72.srec

## 2. Setup and how to run application

One example of setup to demonstrate ot-cli-ftd-ble-shell dual application, using 3 devices, is presented bellow:

Device "A-DualApp": loaded with ot-cli-ftd-ble-shell dual host application
Device "B-OT-15.4": loaded with ot-cli-ftd application only
Device "C-BLE": loaded with ble-shell application only

The starting order of the 3 devices does not really matter, it should work in any starting order.
Bellow is shown one possible scenario of test, there could be more scenarios in terms of starting order and devices roles.

a. Start device "B-OT-15.4" and issue all needed openthread commands to start a network:

```
> factoryreset
> dataset init new
> dataset channel <channel>
> dataset networkkey <network key>
> dataset panid <pan>
> dataset commit active
> ifconfig up
> thread start
```

device "B-OT-15.4" will start the thread network and becames leader

b. Start device "A-DualApp" and issue openthread commands to join to thread network and ble commands to start ble adversting:
BLE and OT share the same serial console. BLE commands on serial console must be preceded by "ble:"

start OT:

```
> factoryreset
> dataset init new
> dataset channel <same channel>
> dataset networkkey <same network key>
> dataset panid <same pan>
> dataset commit active
> ifconfig up
> thread start
```

start BLE:

```
> ble: gap devicename HRS
> ble: gap advdata 1 6
> ble: gap advdata 8 HRS
> ble: gap advdata
> ble: gap advstart
> ble: gattdb addservice 0x180D
```

At this point this device should be joined on thread network and doing ble advertising.

c. Start device "C-BLE" and issue ble commands

```
> gap devicename Collector
> gap scanstart filter
> gap connect <X>          // comment: <X> here you should use the number for HRS device dicovered in the devices list
> gap pair 0
> gatt discover 0 -all
> gatt write 0 15 0x0001
```

At this point all 3 device are running and device "A-DualApp" is:

- joined with "B-OT-15.4" in Thread network
- paired with "C-BLE" in BLE

Form this point some additional commands could be used to test further:

On device "A-DualApp" to send ping packets to device "B-OT-15.4"

```
> ping <addr> <pckt size in bytes> <npckt> <interval_s>
```

While ping packets are running on BLE device "A-DualApp" could sent notifications to device "C-BLE"

```
> ble: gatt notify 0 14
```

Both ping packets and ble notify send from device "A-DualApp" should arrive with succes on destination devices "B-OT-15.4" and "C-BLE".
