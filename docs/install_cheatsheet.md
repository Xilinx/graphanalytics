

# Install steps

## Install XRT/XRM/U50 shells

1. Plug in U50
1. Install [TigerGraph Enterprise version 3.1](https://info.tigergraph.com/enterprise-free) with all the default settings like user name, passwords, directories.
1. Get the installation package from [here](https://xilinx-intranet--simpplr.visualforce.com/apex/FileDetail?siteId=a114T000000haTvQAI&fileId=0694T0000049gBUQAY). (This will eventually put into the Xilinx lounge). This has XRT/XRM install files with static boost library, U50 deployment shell, and xclbin file used for this PoC.
1. tar xzf xilinx-tigergraph-install.tar.gz
1. `cd xilinx-tigergraph-install && ./install.sh`
1. flash the card and cold-reboot
1. Now you should have XRT/XRM/U50 shell installed and xclbin copied to
   TigerGraph install directory


## Porting cosine similarity IP to TigerGraph framework

1. Open a shell as the user created during Tigergraph installation. e.g. `su - tigergraph`
1. Git clone [graph analytic library](https://gitenterprise.xilinx.com/FaaSApps/graphanalytics). Use this the internal version before we push it out the public Github.  
`git clone https://gitenterprise.xilinx.com/FaaSApps/graphanalytics`
1. cd \<graph analytic directory>\/plugin
1. ./install-plugin.sh 
1. cd \<graph analytic directory>\/plugin/tigergraph/tests/cosine_nbor_ss_dense_int
1. ./run.sh

<p align="center">
<img src="images/fig_fpgaRun.jpg" >
</p>



