Xilinx Maximal Independent Set Alveo Product Installation
=========================================================

Follow the steps below to install the Maximal Independent Set Alveo Alveo Product.

.. include:: ../install-alveo.rst

Installing Maximal Independent Set Alveo from a Pre-built Package
------------------------------------------------------------------
* Get the installation package `xilinx-tigergraph-install-1.4.tar.gz from 
  Xilinx website <https://www.xilinx.com/member/forms/download/design-license-xef.html?filename=xilinx-tigergraph-install-1.4.tar.gz>`_ 

* Install Xilinx MIS and MIS TigerGraph Plugin Alveo Products and dependencies 
  (XRT, XRM, and Alveo firmware packages)

.. code-block:: bash

   tar xzf xilinx-tigergraph-install-1.4.tar.gz
   cd xilinx-tigergraph-install && ./install.sh -p mis


Setting up the Alveo Accelerator Card
-------------------------------------

The Maximal Independent Set Alveo Alveo Product requires the following Alveo cards 
and their correponding firmware versions to be installed on each Alveo card to use.  
* U50: xilinx_u50_gen3x16_xdma_201920_3 
* U55C: xilinx_u55c_gen3x16_xdma_base_2
 
Check and install the firmware by following the steps below:

* Run ``xbutil scan`` command to check the status of all Alveo cards on the server.

.. code-block:: bash

    /opt/xilinx/xrt/bin/xbutil scan

* Look at the final rows of the output to see what firmware is installed on each card.  The example below shows the
  end of the output for a server with both Alveo U50 and U55C cards, all containing the correct shell.

.. code-block::

    [0] 0000:81:00.1 xilinx_u50_gen3x16_xdma_201920_3 user(inst=131)
    [1] 0000:04:00.1 xilinx_u55c_gen3x16_xdma_base_2 user(inst=130)

* If all cards to use contain the right shell, skip the remainder of this section.

* Issue the following command to flash the cards with required firmware version:

.. code-block:: bash

    U50
    sudo /opt/xilinx/xrt/bin/xbmgmt flash --update --shell xilinx_u50_gen3x16_xdma_201920_3

    U55C
    sudo /opt/xilinx/xrt/bin/xbmgmt flash --update --shell xilinx_u55c_gen3x16_xdma_base_2

* Cold reboot the server after flashing is done.

..  note:: 
    
    If you only need to build applications utilizing Xilinx GraphAnalytics 
    products, you can skip the section "Collaborating on Maximal Independent Set 
    Product" below.

Collaborating on Maximal Independent Set Product
----------------------------------------------
The Maximal Independent Set product is an open-source project hosted on github, you can 
collaborate with Xilinx and contribute to the development of the product.

* Clone the ``graphanalytics`` repository using ``git``

.. code-block:: bash

   git clone https://github.com/Xilinx/graphanalytics.git
   cd graphanalytics
   git submodule update --init --recursive

All commands below are executed from the root direcotry of the repository.

* Install required devlopment packages. 

.. code-block:: bash

   sudo scripts/devdeps.sh

* Build and install the Maximal Independent Set package. The example below shows 
  installation using the Ubuntu ``apt`` package manager on a Ubuntu 20.04 machine.

.. code-block:: bash

   cd mis
   make dist
   sudo apt install --reinstall ./package/xilinx-mis-0.1_20.04-x86_64.deb

