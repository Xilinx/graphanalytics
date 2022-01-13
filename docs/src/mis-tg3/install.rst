Xilinx Maximal Independent Set for TigerGraph 3.x Installation
==============================================================

Follow the instructions in each of the sections below to install the Maximal 
Independent Set on your server.

Install the Alveo Acclerator Card
---------------------------------

The Maximal Independent Set requires one or more Xilinx Alveo accelerator cards 
to be installed on your server. Follow the steps below to install the card(s).

* Power off the server
* Plug Xilinx Alveo cards into PCIe Gen3 x16 slots
* Power on the server

Install TigerGraph Enterprise 3.x
---------------------------------

Next, follow the steps below to install TigerGraph on your server if you have 
not already.  The latest version tested with the Maximal Independent Set is version 3.1.

* Install `TigerGraph Enterprise version 3.1 <https://info.tigergraph.com/enterprise-free>`_, 
  accepting the defaults for all settings that have them.  Be sure to leave the 
  database owner as user "tigergraph", as the Maximal Independent Set scripts 
  depend on that user name.  Make a note of the password for the user "tigergraph". 
  That password will be needed for later steps.

* Create an account in the TigerGraph installation for yourself in the role of 
  a TigerGraph application developer by issuing the commands below (replace 
  YOUR-LINUX-USERNAME with your actual Linux username). The Maximal Independent 
  Set examples will use this account to create graphs and install GSQL queries 
  on your behalf.

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


Install the Maximal Independent Set Software
--------------------------------------------

To install the Maximal Independent Set product, you can either install a DEB/RPM 
pre-built package, or you can build the Maximal Independent Set from sources if 
you would like to collaborate with Xilinx and contribute to the Maximal Independent Set
product. Follow the steps in *only one* of the two sections below.

Install the Maximal Independent Set from a Package
**************************************************

* Get the installation package xilinx-tigergraph-install-1.0.2.tar.gz from the
  `Database Analytics POC Secure Site <https://www.xilinx.com/member/dba_poc.html>`_.  
  This package contains the Maximal Independent Set as well as its dependencies: 
  XRT, XRM, the Alveo U50 Platform, and the Maximal Independent Set Alveo Product.

* Install the product and its dependencies by un-tarring the package and running
  the included install script.

.. code-block:: bash

   tar xzf xilinx-tigergraph-install-1.0.2.tar.gz
   cd xilinx-tigergraph-install && ./install.sh

* Install the Maximal Independent Set plug-in into the TigerGraph installation.  
  This step makes TigerGraph aware of the Maximal Independent Set features.

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.0/install.sh

Build the Maximal Independent Set from Sources
**********************************************

Follow the instructions below if you want to collaborate and contribute to the 
Xilinx Maximal Independent Set Alveo Product and plugin for TigerGraph 3.x.  
Both products reside in one GitHub repository.

* Install the Xilinx XRT and XRM Libraries

    - Download the 2020.2 version of XRT for the Alveo U50 accelerator card from
      `xilinx.com <https://www.xilinx.com/products/boards-and-kits/alveo/u50.html#gettingStarted>`_ and install it.

    - Follow the instructions from this
      `Xilinx developer page <https://developer.xilinx.com/en/articles/orchestrating-alveo-compute-workloads-with-xrm.html>`_
      to build and install XRM.

* Download the 2020.2 version of the shell for the Alveo U50 accelerator card from
  `xilinx.com <https://www.xilinx.com/products/boards-and-kits/alveo/u50.html#gettingStarted>`_ and install it.
  The package to download is called the "Deployment Target Platform."

* Clone the Xilinx Graph Analytics repository from GitHub.

.. code-block:: bash

   git clone https://github.com/Xilinx/graphanalytics.git

* Build and install the Maximum Independent Set package. The Ubuntu apt package manager is used as an example.

.. code-block:: bash

   cd mis
   make dist
   sudo apt install --reinstall ./package/xilinx-mis-1.0_18.04-x86_64.deb

* Build and install the Maximal Independent Set for TigerGraph 3.x package. (Replace the package
  installation command and name for your server's OS.)

.. code-block:: bash

  cd plugin/tigergraph/mis
  make dist
  sudo apt install --reinstall ./package/xilinx-mis-tigergraph-1.0_18.04-x86_64.deb

* Install the Maximal Independent Set plug-in into the TigerGraph installation.  This step makes TigerGraph aware
  of the Maximal Independent Set features.

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.0/install.sh

Flash the Alveo Cards
---------------------

The Maximal Independent Set requires the xilinx_u50_gen3x16_xdma_201920_3 shell to be installed on each
Alveo card to use.  Check and install the shell by following the steps below.

* Run the ``xbutil scan`` command to check the status of all Alveo cards on the server.

.. code-block:: bash

    /opt/xilinx/xrt/bin/xbutil scan

* Look at the final rows of the output to see what shell is installed on each card.  The example below shows the
  end of the output for a server with three Alveo U50 cards, all containing the correct shell.

.. code-block::

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     [0] 0000:81:00.1 xilinx_u50_gen3x16_xdma_201920_3 user(inst=130)
     [1] 0000:21:00.1 xilinx_u50_gen3x16_xdma_201920_3 user(inst=129)
     [2] 0000:01:00.1 xilinx_u50_gen3x16_xdma_201920_3 user(inst=128)

* If one or more cards is not already running with the correct shell, issue the following
  command to flash the cards.  Cold reboot the server after flashing is done.

.. code-block:: bash

    sudo /opt/xilinx/xrt/bin/xbmgmt flash --update --shell xilinx_u50_gen3x16_xdma_201920_3


Uninstalling the Maximal Independent Set
--------------------------------------

You can uninstall the Maximal Independent Set from TigerGraph by running the install script with the ``-u`` option:

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.0/install.sh -u

**TIP**: To avoid TigerGraph errors, uninstall any queries and UDFs that use the Maximal Independent Set,
such as the Synthea Demo, before uninstalling the Maximal Independent Set itself.
