## Targeting Alveo in HPE server

Alveo U50 is a PCIe card with a FPGA that can be programmed with compute intensive applications like graphics analytics. This has 16 channels of DDR like memory called High Bandwidth Memory (HBM) that offers +200GB/s memory bandwidth; and close to 6K Multiply-Accumulate like engines to speed up the computation. 
Other components of the FPGA are: many SRAM blocks and logic elements that are used to build a custom
computation engine.

The server where U50 is plugged in is HPE DL385 with 2x AMD EPYC processor. This is where we will install TigerGraph software and Xilinx Vitis tool environment which will allow deployment of cosine similarity application.


<p align="center">
<img src="images2/fig_hpe.jpg">
</p>


### Vitis Tool

Vitis is a unified software environment that includes:

- Tools to deploy and run applications on Alveo cards, which includes XRT (Xilinx Runtime), XRM (Xilinx Resouce Manager)

- Tools to develop applications, like: High Level Synthesis tool (which "maps" C/C++/OpenCL C to FPGA), Vivado FPGA implementation and simulation/debugging/profiling environment. This would be covered in the last section - *Vitis development flow*

- Libraries, like graph analytics library

<p align="center">
<img src="images2/fig_vitis.jpg" width="300">
</p>

In order to deploy/run an application like Cosine Similarity on Alveo cards, you need to install the following in order:

1) U50 card to a server 
2) Xilinx Runtime (XRT)
3) Xilinx Resource Manager (XRM)
4) Alveo Deployment shell
5) Install TigerGratph Enterprise version 3.1 (do this before installing Vitis Libraries)
6) Vitis libraries

<p align="center">
<img src="images2/fig_cpu_fpga.jpg" width="500">
</p>


### 1. Alveo U50 Install

Check out:
- [Landing page for U50](https://www.xilinx.com/products/boards-and-kits/alveo/u50.html#overview)
- [UG1301](https://www.xilinx.com/support/documentation/boards_and_kits/accelerator-cards/2019_1/ug1301-getting-started-guide-alveo-accelerator-cards.pdf) 


### 2. XRT (Xilinx Runtime)

XRT is a key component in running application in Alveo. It handles all the data movement between host (x86) and FPGA, board management, FPGA programming and orchestrating tasks running on FPGA


<p align="center">
<img src="images2/fig_xrt.jpg" width="600">
</p>


### Building/Installing XRT

Typically XRT can be installed simply from .deb/.rpm package, but for TigerGraph integration requires you to have a static version of boost library, so you must build XRT from sources. These are the steps in Ubuntu OS:

1) Build XRT
```
git clone -b master https://github.com/Xilinx/XRT 
sudo -i
cd <XRT install directory>
PATH_XRT=$PWD
source $PATH_XRT/src/runtime_src/tools/scripts/xrtdeps.sh
source $PATH_XRT/src/runtime_src/tools/scripts/boost.sh -prefix $PATH_XRT/boost
source $PATH_XRT/build/build.sh -clean
sudo -i
cd <XRT install directory>
PATH_XRT=$PWD
env XRT_BOOST_INSTALL=$PATH_XRT/boost/xrt $PATH_XRT/build/build.sh
cd $PATH_XRT/build/Release
make package
```

2) Install XRT
```
cd $PATH_XRT/build/Release
apt install ./xrt_202110.2.9.0_16.04-amd64-xrt.deb
```


### 3. XRM (Xilinx Resource Manager)

XRM is the software to manage all the FPGA resources on the system. 
- All the kernels (IP Kernel or Soft Kernel, you mean host code?) on FPGA board are abstracted as compute unit (CU) resources. 
- XRM provides interfaces to allocate and release CU. The smallest allocation unit is a channel, which is percentage of one CU / number of available CU?
- XRM provides command line tool (*xrmadm*) to download xclbin to FPGA devices and builds the resource database.
- XRM daemon (*xrmd*) is running on the background process to support resource reservation, relinquishing, allocation and release of CU. 
- Load distribution among CUs can be easily done utilizing XRM APIs
- The user APIs are defined in [*xrm.h*](https://github.com/Xilinx/XRM/blob/master/src/lib/xrm.h) header file.


<p align="center">
<img src="images2/fig_xrm.jpg" width="650">
</p>


### Building/Installing XRM
Build a staic boost version for stability (EDIT LATER)

```
git clone -b master https://github.com/Xilinx/XRM 
sudo -i
cd <install dir>/tools
./xrmdeps.sh
cd ..
./build.sh
cd <install dir>/Release
apt install ./xrm_202020.1.1.0_16.04-x86_64.deb

```

### 4. Alveo depolyment shell

There are two major partitions within the FPGA - one is static area we call "shell" and the other is dynamic region where we program our custom application. The shell is programmed by using the deployment shell which we need to install. This shell functions as a hardware infrastructure where data movement between host and FPGA/global memory and configuration/control of custom logic region are handled.  *xclbin* is the file that contains binary information which programs this custom logic region. 


<p align="center">
<img src="images2/fig_heter.jpg" width="600">
</p>


### Installing deployment shell

Here are the steps to install a deployment shell for U50 (EDIT LATER). First, download the deployment shell from [here](https://www.xilinx.com/products/boards-and-kits/alveo/u50.html#gettingStarted)
```
tar xvzf xxxx.deb.tar.gz
sudo apt install all 3 files 
Flash the card
Cold-reboot
```

Test to XRT/deployment shell are properly installed and U50 is working
```
source /opt/xilinx/xrt/setup.sh
xbutil query # for simple query to see if U50 is visible
xbutil validate # for more thorough testing of the board
```

### 5. TigerGrpah software

We need to install TigerGraph Enterprise version 3.1 to work properly with the rest of the install. You can start from [here](https://info.tigergraph.com/enterprise-free) 


### 6. Vitis Library

[Vitis Library](https://xilinx.github.io/Vitis_Libraries/index.html) includes several domain specific libraries which are open-sourced (Apache License, v2.0) and performance optimized.

<p align="center">
<img src="images2/fig_vitisLib.jpg" width="600">
</p>

You can install Vitis Library by:
```
su - tigergraph # to make sure all the files are owned by tigergraph user, which is necessary later
git clone -b master https://github.com/Xilinx/Vitis_Libraries
```

There are 3 levels of hierarchy in Vitis Library. 
- Level 1 (L1): this is the lowest level. It is primitive functions which are called by L2/L3 functions
- Level 2 (L2): this is a kernel level function, meaning that it has the proper interface (AXI-M/AXI-lite interface) and HLS pragmas embedded in the code to help HLS synthesis. These are the kind of functions you can call from the host code and it will run in the FPGA. At this level, you will most likely see host code as well as the kernel code
- Level 3 (L3): highest level functions, which may call L2/L1 functions. You can view this as a standalone application software, like many of graph analytic functions.


<p align="center">
<img src="images2/fig_L123.jpg" width="300">
</p>

### [Vitis Graph Library](https://github.com/Xilinx/Vitis_Libraries/tree/master/graph)

Since we will be using several IPs from Graph Library like Cosine Similarity, here is a bit more detail on Graph Library. It is written in C++ and has L1, L2 and L3 level functions like the rest of Vitis Library. L3 functions utilizes XRM to make it easier to manage FPGA resources. Included in this library is all the files necessary for TigerGraph integration.


Here is the overview of Graph Library 

- Similarity analysis: Cosine Similarity, Jaccard Similarity, k-nearest neighbor.
- Centrality analysis: PageRank.
- Pathfinding: Single Source Shortest Path (SSSP), Multi-Sources Shortest Path (MSSP).
- Connectivity analysis: Weakly Connected Components and Strongly Connected Components.
- Community Detection: Louvain Modularity, Label Propagation and Triangle Count.
- Search: Breadth First Search.
- Graph Format: Calculate Degree and Format Convert between CSR and CSC.
- [User's guide](https://xilinx.github.io/Vitis_Libraries/graph/2020.1/guide_L3/L3_internal/getting_started.html)


### Running L3 tests on Alveo
Before the TigerGraph integration, it would be good to understand the directory structure of L3 Graph Library and how to compile and run any of these applications standalone on Alveo card. 

- include: header files for L3 graph function prototypes
- src:  source files for functions declared in the header files
- lib: all the functions in *src* is packaged into libgraphL3.so, which can be built by *./build_so.sh*
- tests: contains 20+ test application to run on Alveo cards. <test>.cpp in each directory contains the host code that runs on x86. This code utilizes XRM API;thus, adding a little more complexity to the code but allows easier provisioning and maintenance of FPGA resources. One thing to note is that generating *xclbin* takes awhile - it may take several hours. In order to avoid this compiling time, these *xclbin* files can be downloaded from TBD.

<p align="center">
<img src="images2/fig_dir1.jpg", width "300">
</p>


In order to run any test, do the [following](https://github.com/Xilinx/Vitis_Libraries/tree/master/graph/L3/tests):
```
cd graph/L3; ./build_so.sh #build libgraphL3.so
source /opt/xilinx/xrt/setup.sh 
source /opt/xilinx/xrm/setup.sh 
cd tests/<testcase> 
make build TARGET=hw # this may take several hours to build xclbin, which is used in programming FPGA
change "PROJECTPATH" in config.json to the absolute path where .xclbin file is
make run TARGET=hw 
```
