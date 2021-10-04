Xilinx Cosine Similarity Alveo Product Installation
===================================================

Follow the steps below to install the Cosine Similarity Alveo Product.

Installing the Alveo U50 Accelerator Card
-----------------------------------------

* Power off the server.
* Plug the Xilinx U50 Alveo acclerator card into a PCIe Gen3 x16 slot.
* Power on the server.

Installing Cosine Similarity Library from a Pre-built Package
-------------------------------------------------------------
* Get the installation package `xilinx-tigergraph-install-1.0.2.tar.gz from 
  Xilinx website <https://www.xilinx.com/member/forms/download/design-license-xef.html?filename=xilinx-tigergraph-install-1.0.2.tar.gz>`_ 

* Install Xilinx CosineSim and Recommendation Engine Alveo Products and 
  dependencies (XRT, XRM, and U50 platform packages)

.. code-block:: bash

   tar xzf xilinx-tigergraph-install-1.0.2.tar.gz
   cd xilinx-tigergraph-install && ./install.sh -p cosinesim


Setting up the Alveo Accelerator Card
-------------------------------------

The Cosine Similarity Alveo Product requires the xilinx_u50_gen3x16_xdma_201920_3 shell to be installed on each
Alveo card to use.  Check and install the shell by following the steps below.

* Run ``xbutil scan`` command to check the status of all Alveo cards on the server.

.. code-block:: bash

    /opt/xilinx/xrt/bin/xbutil scan

* Look at the final rows of the output to see what shell is installed on each card.  The example below shows the
  end of the output for a server with three Alveo U50 cards, all containing the correct shell.

.. code-block::

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     [0] 0000:81:00.1 xilinx_u50_gen3x16_xdma_201920_3 user(inst=130)
     [1] 0000:21:00.1 xilinx_u50_gen3x16_xdma_201920_3 user(inst=129)
     [2] 0000:01:00.1 xilinx_u50_gen3x16_xdma_201920_3 user(inst=128)

* If all cards to use contain the right shell, skip the remainder of this section.

* Issue the following command to flash the cards.

.. code-block:: bash

    sudo /opt/xilinx/xrt/bin/xbmgmt flash --update --shell xilinx_u50_gen3x16_xdma_201920_3

* Cold reboot the server after flashing is done.


Collaborating on the Cosine Similarity Product
----------------------------------------------
The Cosine Similarity product is an open-source project hosted on github, you can 
collaborate with Xilinx and contribute to the development of the product.

* Clone the ``graphanalytics`` repository using ``git``.

.. code-block:: bash

   git clone https://github.com/Xilinx/graphanalytics.git
   cd graphanalytics
   git submodule update --init --recursive

* Build and install the Cosine Similarity package. The example below shows installation using the
  Ubuntu ``apt`` package manager.

.. code-block:: bash

   cd cosinesim
   make dist
   sudo apt install --reinstall ./package/xilinx-cosinesim-1.0_18.04-x86_64.deb

