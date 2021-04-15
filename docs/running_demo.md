

# Running Cosine Similarity Acceleration Demo

This page describes the necessary steps to run the cosine similarity acceleration demo.
In a nutshell you will be installing TigerGraph software,
necessary Xilinx software, port the Xilinx graph library to Tigergraph
environment, and then run cosine similarity computation running on Alveo U50
card integated into the HPE server. 

For more detail information on Xilinx environment and how Xilinx
functions are ported to Tigergraph environment, check [page](targeting_alveo.md).


## Install XRT/XRM/U50 shells

1. Plug in U50
1. Install [TigerGraph Enterprise version 3.1](https://info.tigergraph.com/enterprise-free) with all the default settings like user name, passwords, directories.
1. Download xilinx-tigergraph-install.tar.gz from [Design Files tab on Xilinx Database Analytics Secure Site](https://www.xilinx.com/member/dba_poc.html#designFiles). Registration of the secure site is required. This has XRT/XRM install files with static boost library, U50 deployment shell, and xclbin file used for this PoC.
1. tar xzf xilinx-tigergraph-install.tar.gz
1. `cd xilinx-tigergraph-install && ./install.sh`
1. flash the card and cold-reboot (as shown in the log message of the script output). Check [Alveo U50 Data Center
Accelerator Card Installation
Guide](https://www.xilinx.com/support/documentation/boards_and_kits/accelerator-cards/1_7/ug1370-u50-installation.pdf) for more details.
1. Now you should have XRT/XRM/U50 shell installed and xclbin copied to
   TigerGraph install directory


## Porting cosine similarity function to TigerGraph framework

1. Open a shell as the user created during Tigergraph installation. e.g. `su - tigergraph`
1. Git clone [graph analytic library](https://gitenterprise.xilinx.com/FaaSApps/graphanalytics). (Use this the internal version before we push it out the public Github page).  
`git clone https://gitenterprise.xilinx.com/FaaSApps/graphanalytics`
1. cd \<graph analytic directory>\/plugin
1. ./install-plugin.sh 
1. cd \<graph analytic directory>\/plugin/tigergraph/tests/cosine_nbor_ss_dense_int
1. ./run.sh
1. If this script has been run sucessfully you should see the output similar to
   the one below

<p align="center">
<img src="images/fig_fpgaRun.jpg" >
</p>



