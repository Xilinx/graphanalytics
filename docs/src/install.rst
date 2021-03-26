==============================================
Install XRT and XRM libraries and U50 platform 
==============================================

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

==============================================
Install Xilinx Alveo Plugin for TigerGraph 
==============================================
   

* Clone graphanalytics repository using your username

.. code-block:: bash

   git clone https://github.com/Xilinx/graphanalytics.git

* Run the commands below to install the plugin as the user "tigergraph" 

.. code-block:: bash

   su - tigergraph
   cd graphanalytics/plugin
   ./install-plugin.sh 
