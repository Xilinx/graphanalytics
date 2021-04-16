=========================================================================
Install Xilinx CosineSim and Recommendation Engine Alveo Products 
=========================================================================

* Power off the server. Plug Xilinx U50 Alveo card into a PCIe Gen3 x16 slot. Power on the server.
* Install `TigerGraph Enterprise version 3.1 <https://info.tigergraph.com/enterprise-free>`_ with all the 
  default settings like username, passwords, directories. Make a note of the password for the user 
  "tigergraph" as it will be needed for later steps.
* Get the installation package xilinx-tigergraph-install-1.0.tar.gz from 
  `Database Analytics POC Secure Site <https://www.xilinx.com/member/dba_poc.html>`_ 
* Install Xilinx CosineSim and Recommendation Engine Alveo Products and 
  dependencies (XRT, XRM, and U50 platform packages)

.. code-block:: bash

   tar xzf xilinx-tigergraph-install-1.0.tar.gz
   cd xilinx-tigergraph-install && ./install.sh
    
* Flash the card and cold-reboot
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

