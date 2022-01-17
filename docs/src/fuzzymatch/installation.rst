Xilinx Fuzzy Match Alveo Product Installation
=============================================

Follow the steps below to install the Fuzzy Match Alveo Alveo Product.

.. include:: ../common/install-alveo.rst

Installing Fuzzy Match Alveo from a Pre-built Package
------------------------------------------------------------------
* Get the installation package xilinx-tigergraph-install-x.y.z.tar.gz from the
  `Database Analytics POC Secure Site <https://www.xilinx.com/member/dba_poc.html>`_.  
  This package contains the Fuzzy Match as well as all dependencies (XRT, XRM, Alveo 
  firmware, etc)

* Install Xilinx Fuzzy Match Alveo Products and dependencies: 

  .. code-block:: bash

    tar xzf xilinx-tigergraph-install-x.y.z.tar.gz
    cd xilinx-tigergraph-install && ./install.sh -p fuzzymatch


Setting up the Alveo Accelerator Card
-------------------------------------

The Fuzzy Match Alveo Alveo Product requires the following Xilix FPGA cards and 
their correponding firmware versions:  

* U50: xilinx_u50_gen3x16_xdma_201920_3 
* AWS F1: xilinx_aws-vu9p-f1_shell-v04261818_201920_2
 
.. include:: ../common/flash-alveo.rst

.. include:: install-collaborate.rst
