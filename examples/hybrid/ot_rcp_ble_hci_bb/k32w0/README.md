# Dual mode app - OpenThread RCP, BLE HCI BB example on K32W061

This directory contains the code of the dual mode application Openthread RCP + BLE HCI BB for K32W061

## Toolchain & Tool

Follow the same procedure as described in the [README.md][k32w061_readme], describing what need to be install to run OpenThread on NXP K32W061 platform.

[k32w061_readme]: ../../../../src/k32w0/k32w061/README.md

## Building the examples

To build the example, follow what is described in [README.md 'Building the examples section'][k32w061_readme_build_example].

[k32w061_readme_build_example]: ../../../../src/k32w0/k32w061/README.md#Building-the-examples

Instead of running (which would build all k32w061 examples)

```bash
$ ./script/build_k32w061
```

it is possible to build only ot_rcp_ble_hci_bb examples.

The below ot_rcp_ble_hci_bb targets can be generated:

- `ot_rcp_uart_ble_hci_bb_uart_fc` with spinel running on one UART and HCI on another UART with flow control support.
- `ot_rcp_spi_ble_hci_bb_uart_fc` with spinel running on SPI and HCI on one UART with flow control support.

The following command can be used to build only a specific target:

```bash
$ ./script/build_k32w061 <target_name>
```

After a successful build, the application binary will be generated in `build_k32w061/<target_name>/ot-rcp-ble-hci-bb-k32w061`.

## Flash Binaries

To flash the binary, follow the same procedure as described in the "Flash Binaries" section in [README.md][k32w061_readme_flash_binaries].

[k32w061_readme_flash_binaries]: ../../../../src/k32w0/k32w061/README.md#Flash-Binaries
