=========================================================
Xilinx Maximal Independent Set Alveo Product Installation
=========================================================

Follow the steps below to install the Maximal Independent Set Alveo Product. Skip the collaboration section if only deploying.

Installing Maximal Independent Set Alveo from a Pre-built Package
------------------------------------------------------------------
* Get the installation package **amd-graphanalytics-install-%PACKAGE_VERSION.tar.gz** from the
  `Database Analytics POC Secure Site <%PACKAGE_LINK>`_.
  This package contains the MIS as well as its dependencies.

* Install Xilinx MIS and MIS TigerGraph Plugin Alveo Products and dependencies 
  (XRT, XRM, and Alveo firmware packages)

.. code-block:: bash

   tar xzf amd-graphanalytics-install-%PACKAGE_VERSION.tar.gz
   cd amd-graphanalytics-install && ./install.sh -p mis


Setting up the Alveo Accelerator Card
-------------------------------------

The Maximal Independent Set Alveo Product requires the following Alveo cards
and their corresponding firmware versions to be installed on each Alveo card to use.

* U50: xilinx_u50_gen3x16_xdma_201920_3 
* U55C: xilinx_u55c_gen3x16_xdma_base_2

.. include:: ../common/flash-alveo.rst

* Cold reboot the server after flashing is done.

..  note:: 
    
    If you only need to build applications utilizing Xilinx GraphAnalytics 
    products, you can skip the section below.

Collaborating on Maximal Independent Set Product
----------------------------------------------
The Maximal Independent Set product is an open-source project hosted on github, you can 
collaborate with Xilinx and contribute to the development of the product.

.. include:: ../common/clone-repo.rst

* Build and install the Maximal Independent Set package. The example below shows 
  installation using the Ubuntu ``apt`` package manager on a Ubuntu 20.04 machine.

.. code-block:: bash

   cd mis
   make dist
   sudo apt install --reinstall ./package/xilinx-mis-%MIS_VERSION_20.04-x86_64.deb

