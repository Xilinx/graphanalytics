# Getting started with Alveo - accelerating TigerGraph

The goal of this page is to describe the process of integrating graphics analytic functions running on Alveo FPGA accelerator card to TigerGraph software environment;thus, accelerating the overall execution of TigerGraph software. There are 4 main sections:


- Targeting Alveo in HPE server: 
Describes the steps to bring up Alveo U50 card in on-premise environment like HPE server.  The target application is cosine similarity computation from Xilinx Vitis Graph Library.  

- Targeting Azure NP VM (later): 
Describes the steps to run applications on Azure NP virtual machine

- Integration with TigerGraph:
Explains how Xilinx Vitis graph library IP is constructed and ported into TigerGraph SW framework

- Vitis development flow: 
Describes the development flow to create/modify accelerator IP to run on FPGA, which can be exported to TigerGraph SW

