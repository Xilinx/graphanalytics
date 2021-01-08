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

# License

Licensed using the [Apache 2.0 license](https://www.apache.org/licenses/LICENSE-2.0).

    Copyright 2020-2021 Xilinx, Inc.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
    Copyright 2020-2021 Xilinx, Inc.

