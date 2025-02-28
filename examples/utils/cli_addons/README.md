# ot-cli addons

NXP's ot-cli example includes an addons mechanism which allow to easily register extra commands to ot-cli.

By default, no addons are built, but they can be enabled by settings CMake options.

## iperf-cli addon

[iPerf](https://github.com/esnet/iperf) is a tool used to measure throughput performances on IP networks.

The tool has been ported to ot-cli is order to run benchmarks and throughput scenarios over OpenThread networks.

**Note:** This addon is based on "lwiperf" which is a port of iPerf onto lwIP tcp/ip stack.

To build ot-cli with iperf addon, dedicated targets exist for each platform that supports this feature. For example,
for RW612:

`ot_cli_iperf`: Adds iperf_cli addon to the ot-cli application

Other platforms may have multiple iperf targets, but they can be easily identified by the `_iperf` extension.

Example of a typical command:

```bash
./script/build_<platform> ot_cli_iperf
```
