# Dual mode app - OpenThread RCP & ZB ZC

This directory contains the code of the dual mode application Openthread RCP + ZB coordinator for K32W1

## Toolchain & Tool

Follow the same procedure as described in the [README.md][mcxw71_readme], describing what need to be install to run OpenThread on NXP MCXW71 platform.

[mcxw71_readme]: ../../../../src/mcxw71/README.md

## Building the examples

To build the example, follow what is described in [README.md 'Building the examples section'][mcxw71_readme_build_example].

[mcxw71_readme_build_example]: ../../../../src/mcxw71/README.md#Building-the-examples

Instead of running (which would build all mcxw71 examples)

```bash
$ ./script/build_mcxw71
```

it is possible to build only ot_rcp_uart_dma_zb_zc examples.

The below ot_rcp_uart_dma_zb_zc targets can be generated:

- `ot_rcp_uart_dma_zb_zc` with RCP app running on LPUART1 with DMA and ZB app running on LPUART0.

The following command can be used to build only a specific target:

```bash
$ ./script/build_mcxw71 <target_name>
```

After a successful build, the application binary will be generated in `build_mcxw71/<target_name>/ot-rcp-uart-dma-zb-zc-mcxw71`.

## Flash Binaries

To flash the binary, follow the same procedure as described in the "Flash Binaries" section in [README.md][mcxw71_readme_flash_binaries].

[mcxw71_readme_flash_binaries]: ../../../../src/mcxw71/README.md#Flash-Binaries
