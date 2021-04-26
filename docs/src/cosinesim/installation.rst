Xilinx Cosine Similarity Alveo Product Installation
===================================================

Follow the steps below to install the Cosine Similarity Alveo Product.

Installing the Alveo U50 Accelerator Card
-----------------------------------------

* Power off the server.
* Plug the Xilinx U50 Alveo acclerator card into a PCIe Gen3 x16 slot.
* Power on the server.

Installing the Xilinx XRT and XRM Libraries
-------------------------------------------

* Download the 2020.2 version of XRM for the Alveo U50 accelerator card from
  `xilinx.com <https://www.xilinx.com/products/boards-and-kits/alveo/u50.html#vitis>`_ and install it.

* Follow the instructions from this
  `Xilinx developer page <https://developer.xilinx.com/en/articles/orchestrating-alveo-compute-workloads-with-xrm.html>`_
  to build and install XRM.

Installing the Cosine Similarity Library
----------------------------------------

You can install the product from a pre-built DEB or RPM package.  Alternatively, as the product is an open-source
project hosted on github, you can collaborate with Xilinx and contribute to the development of the product.

Installing a Pre-built Package
******************************

* Download the Cosine Similarity installation package for your operating system from the
  `Database Analytics POC Secure Site <https://www.xilinx.com/member/dba_poc.html>`_ and install it.

Collaborating on the Cosine Similarity Product
**********************************************

* Clone the ``graphanalytics`` repository using ``git``.

.. code-block:: bash

   git clone https://github.com/Xilinx/graphanalytics.git

* Build and install the Cosine Similarity package. The example below shows installation using the
  Ubuntu ``apt`` package manager.

.. code-block:: bash

   cd cosinesim
   make dist
   sudo apt install --reinstall ./package/xilinx-cosinesim-1.0_18.04-x86_64.deb

