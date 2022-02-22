=====================================================
Xilinx Fuzzy Match Plugin for TigerGraph Installation
=====================================================

Follow the instructions in each of the sections below to install the Fuzzy Match  
products on your server.

.. include:: ../common/install-alveo.rst
.. include:: ../common/install-tigergraph.rst

Install Fuzzy Match Software
----------------------------

To install the Fuzzy Match product, you can either install a DEB/RPM pre-built 
package, or you can build the Fuzzy Match from sources if you would like to 
collaborate with Xilinx and contribute to the Fuzzy Match product. Follow 
the steps in *only one* of the two sections below.

Install Fuzzy Match from a Package
**********************************

* Get the installation package **xilinx-tigergraph-install-%PACKAGE_VERSION.tar.gz** from the
  `Database Analytics POC Secure Site <%PACKAGE_LINK>`_.
  This package contains the Fuzzy Match as well as its dependencies: 
  XRT, XRM, the Alveo U50 Platform, and the Fuzzy Match Alveo Product.

* Install the product and its dependencies by un-tarring the package and running
  the included install script.

  .. code-block:: bash

    tar xzf xilinx-tigergraph-install-%PACKAGE_VERSION.tar.gz
    cd xilinx-tigergraph-install && ./install.sh -p fuzzymatch

* Install the Fuzzy Match plugin into the TigerGraph installation.  

  .. code-block:: bash

    /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/fuzzymatch/%FUZZYMATCH_TG_VERSION/install.sh -d xilinx_u50_gen3x16_xdma_201920_3

  Supported Alveo devices:

  * U50: xilinx_u50_gen3x16_xdma_201920_3 
  * AWS F1: xilinx_aws-vu9p-f1_shell-v04261818_201920_2 


.. include:: ../fuzzymatch/install-collaborate.rst

* Build and install Fuzzy Match plugin package. (Replace the package installation 
  command and name for your server's OS.)

  .. code-block:: bash

    cd plugin/tigergraph/fuzzymatch
    make dist
    sudo apt install --reinstall ./package/xilinx-fuzzymatch-tigergraph-%FUZZYMATCH_TG_VERSION_20.04-x86_64.deb

* Install the Fuzzy Match plugin into the TigerGraph installation. 

  .. code-block:: bash

    /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/fuzzymatch/%FUZZYMATCH_TG_VERSION/install.sh -d xilinx_u50_gen3x16_xdma_201920_3

  Supported Alveo devices:

  * U50: xilinx_u50_gen3x16_xdma_201920_3 
  * AWS F1: xilinx_aws-vu9p-f1_shell-v04261818_201920_2   

Flash Alveo Cards
-----------------

The Fuzzy Match requires supported firmware to be installed on each Alveo card.  

.. include:: ../common/flash-alveo.rst

Uninstalling the Fuzzy Match
--------------------------------------

You can uninstall the Fuzzy Match from TigerGraph by running the install script with the ``-u`` option:

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/fuzzymatch/%FUZZYMATCH_TG_VERSION/install.sh -u

**TIP**: To avoid TigerGraph errors, uninstall any queries and UDFs that use the Fuzzy Match before
uninstalling the Fuzzy Match itself.
