# Level 2: Predefined Kernels

The level 2 of Vitis Graph Library contains the host-callable kernels. For more details information, please reference to [_L2 User Guide_](https://pages.gitenterprise.xilinx.com/FaaSApps/xf_graph/2020.1/index.html) in the document for usage and design information.

# Vitis Tests for Kernels

Simple tests are included for each of Graph kernels to discover simple regression errors.

To run a test for a specific kernel, execute the following command:

```
cd <KERNEL-NAME>
source <install path>/Vitis/2019.2/settings64.sh
source /opt/xilinx/xrt/setup.sh
export PLATFORM_REPO_PATHS=/opt/xilinx/platforms
make run TARGET=sw_emu DEVICE=xilinx_u50_gen3x16_xdma_201920_3
```

`TARGET` can also be `hw_emu` or `hw`.
