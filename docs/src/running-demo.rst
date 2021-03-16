===========================================
Running Cosine Similarity Acceleration Demo
===========================================

This page describes the necessary steps to run the cosine similarity acceleration demo.
In a nutshell you will be installing TigerGraph software,
necessary Xilinx software, port the Xilinx graph library to Tigergraph
environment, and then run cosine similarity computation running on Alveo U50
card integated into the HPE server. 

For more detail information on Xilinx environment and how Xilinx
functions are ported to Tigergraph environment, check [page](targeting_alveo.md).

Install XRT/XRM/U50 shells
--------------------------------------------------

#. Plug in U50
#. Install `TigerGraph Enterprise version 3.1 <https://info.tigergraph.com/enterprise-free>`_ 
   with all the default settings like user name, passwords, directories.
#. Get the installation package from `Database Analytics POC Secure Site 
   <https://www.xilinx.com/member/dba_poc.html>`_ 
#. tar xzf xilinx-tigergraph-install.tar.gz
#. `cd xilinx-tigergraph-install && ./install.sh`
#. flash the card and cold-reboot (as shown in the log message of the script output)
#. Now you should have XRT/XRM/U50 shell installed and xclbin copied to
   TigerGraph install directory


Porting cosine similarity function to TigerGraph framework
-----------------------------------------------------------

#. Open a shell as the user created during Tigergraph installation. e.g. `su - tigergraph`
#. Run the commands below to install the plugin and run the demo

.. code-block:: bash

   git clone https://github.com/Xilinx/graphanalytics.git
   cd graphanalytics/plugin
   ./install-plugin.sh 

   cd graphanalytics/plugin/tigergraph/tests/cosine_nbor_ss_dense_int
   ./run.sh
