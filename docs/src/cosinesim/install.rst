Xilinx Cosine Similarity Alveo Product Installation
===================================================

Follow the steps below to install the Cosine Similarity Alveo Product.

Installing the Alveo Accelerator Cards
-----------------------------------------

* Power off the server
* Plug the Xilinx U50 or U55C Alveo Data Center Accelerator card into a PCIe Gen3 x16 slot
* Power on the server

Installing Cosine Similarity Library from a Pre-built Package
-------------------------------------------------------------
* Get the installation package **amd-graphanalytics-install-%PACKAGE_VERSION.tar.gz** from the
  `Database Analytics POC Secure Site <%PACKAGE_LINK>`_.

* Install Xilinx CosineSim and Recommendation Engine Alveo Products and dependencies 
  (XRT, XRM, and Alveo firmware packages)

.. code-block:: bash

   tar xzf amd-graphanalytics-install-%PACKAGE_VERSION.tar.gz
   cd amd-graphanalytics-install && ./install.sh -p cosinesim


Setting up the Alveo Accelerator Card
-------------------------------------

The Cosine Similarity Alveo Product requires the following Alveo cards and their 
corresponding firmware versions to be installed on each Alveo card to use.

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
    products, you can skip the section "Collaborating on Cosine Similarity 
    Product" below.

Collaborating on Cosine Similarity Product
----------------------------------------------
The Cosine Similarity product is an open-source project hosted on github, you can 
collaborate with Xilinx and contribute to the development of the product.

* Clone the ``graphanalytics`` repository using ``git``

.. code-block:: bash

   git clone https://github.com/Xilinx/graphanalytics.git
   cd graphanalytics
   git submodule update --init --recursive

All commands below are executed from the root directory of the repository.

* Install required development packages.

.. code-block:: bash

   sudo scripts/devdeps.sh

* Build and install the Cosine Similarity package. The example below shows installation using the
  Ubuntu ``apt`` package manager on a Ubuntu 18.04 machine.

.. code-block:: bash

   cd cosinesim
   make dist
   sudo apt install --reinstall ./package/xilinx-cosinesim-%COSINESIM_VERSION_18.04-x86_64.deb

