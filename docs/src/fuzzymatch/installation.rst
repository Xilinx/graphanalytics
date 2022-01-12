Xilinx Fuzzy Match Alveo Product Installation
=============================================

Follow the steps below to install the Fuzzy Match Alveo Alveo Product.

.. include:: ../install-alveo.rst

Installing Fuzzy Match Alveo from a Pre-built Package
------------------------------------------------------------------
* Get the installation package xilinx-tigergraph-install-x.y.z.tar.gz from the
  `Database Analytics POC Secure Site <https://www.xilinx.com/member/dba_poc.html>`_.  
  This package contains the Fuzzy Match as well as its dependencies: 
  XRT, XRM, the Alveo U50 Platform, and the Fuzzy Match Alveo Product.

* Install Xilinx Fuzzy Match Alveo Products and dependencies (XRT, XRM, and Alveo firmware packages)

.. code-block:: bash

   tar xzf xilinx-tigergraph-install-1.4.tar.gz
   cd xilinx-tigergraph-install && ./install.sh -p fuzzymatch


Setting up the Alveo Accelerator Card
-------------------------------------

The Fuzzy Match Alveo Alveo Product requires the following Xilix FPGA cards and 
their correponding firmware versions:  

* U50: xilinx_u50_gen3x16_xdma_201920_3 
* AWS F1: xilinx_aws-vu9p-f1_shell-v04261818_201920_2
 
Check and install the firmware by following the steps below:

* Run ``xbutil scan`` command to check the status of all Alveo cards on the server.

.. code-block:: bash

    /opt/xilinx/xrt/bin/xbutil scan

* Look at the final rows of the output to see what firmware is installed on each 
  card.  The example below shows the end of the output for a server with both 
  Alveo U50 and U55C cards, all containing the correct shell.

.. code-block::

    [0] 0000:81:00.1 xilinx_u50_gen3x16_xdma_201920_3 user(inst=131)
    [1] 0000:04:00.1 xilinx_u55c_gen3x16_xdma_base_2 user(inst=130)

* If all cards to use contain the right shell, skip the remainder of this section.

* Issue the following command to flash the cards with required firmware version:

.. code-block:: bash

    U50
    sudo /opt/xilinx/xrt/bin/xbmgmt flash --update --shell xilinx_u50_gen3x16_xdma_201920_3

* Cold reboot the server after flashing is done.

.. include:: install-collaborate.rst
