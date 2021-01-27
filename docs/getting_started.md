# Getting started with Alveo - accelerating TigerGraph

The goal of this page is to describe the process of integrating graphics analytic functions running on Alveo FPGA accelerator card to TigerGraph software environment;thus, accelerating the overall execution of TigerGraph software. There are 4 main sections:


- [Targeting Alveo in HPE server](targeting_alveo.md): 
Describes the steps to bring up Alveo U50 card in on-premise environment like HPE server.  The target application is cosine similarity computation from Xilinx Vitis Graph Library.  

- Targeting Azure NP VM (later): 
Describes the steps to run applications on Azure NP virtual machine

- [Integration with TigerGraph](integrate_TG.md):
Explains how Xilinx Vitis graph library IP is constructed and ported into TigerGraph SW framework

- [Vitis development flow](vitis_dev_flow.md): 
Describes the development flow to create/modify accelerator IP to run on FPGA, which can be exported to TigerGraph SW

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
