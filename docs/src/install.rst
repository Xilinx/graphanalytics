=========================================================================
Install Xilinx CosineSim and Recommendation Engine Alveo Products 
=========================================================================

* Power off the server. Plug Xilinx U50 Alveo card into a PCIe Gen3 x16 slot. Power on the server.

* Install `TigerGraph Enterprise version 3.1 <https://info.tigergraph.com/enterprise-free>`_ with all the 
  default settings like username, passwords, directories. Make a note of the password for the user 
  "tigergraph". It will be needed for later steps. Create an account on TigerGraph installation for 
  yourself by using the commands below (replace YOUR-LINUX-USERNAME with your actual Linux username)

.. code-block:: bash

   $su - tigergraph
   Password:
   $gsql
   Welcome to TigerGraph.
   GSQL > create user
   User Name : YOUR-LINUX-USERNAME
   New Password : *********
   Re-enter Password : *********
   The user "YOUR-LINUX_USERNAME" is created.
   GSQL > grant role globaldesigner to YOUR-LINUX-USERNAME
   Role "globaldesigner" is successfully granted to user(s): YOUR-LINUX-USERNAME

* Get the installation package xilinx-tigergraph-install-1.0.tar.gz from 
  `Database Analytics POC Secure Site <https://www.xilinx.com/member/dba_poc.html>`_ 

* Install Xilinx CosineSim and Recommendation Engine Alveo Products and 
  dependencies (XRT, XRM, and U50 platform packages)

.. code-block:: bash

   tar xzf xilinx-tigergraph-install-1.0.tar.gz
   cd xilinx-tigergraph-install && ./install.sh
    
* Flash the Alveo U50 card if it is not already running with the shell 
  xilinx_u50_gen3x16_xdma_201920_3. Cold reboot the machine after flashing is done

* Install Recommendation Engine plugin for TigerGraph

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.0/install.sh


==============================================================================
Collaborate on Xilinx CosineSim and Recommendation Engine Alveo Products 
==============================================================================

Follow the instructions below if you want to collaborate and contribute to Xilinx 
CosineSim and Recommendation Engine Alveo Products

* Clone graphanalytics repository

.. code-block:: bash

   git clone https://github.com/Xilinx/graphanalytics.git

* Build and install CosinSim package (Replace the package installation command 
  and name for your server's OS):

.. code-block:: bash

   cd cosinesim
   make dist
   sudo apt install ./package/xilinx-cosinesim-1.0_18.04-x86_64.deb

* Build and install Recommendation Engine package (Replace the package 
  installation command and name for your server's OS)

.. code-block:: bash

  cd plugin/tigergraph/
  make dist
  sudo apt install ./package/xilinx-recomengine-tigergraph-1.0_18.04-x86_64.deb

