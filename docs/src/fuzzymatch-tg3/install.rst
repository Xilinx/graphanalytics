=====================================
AMD Fuzzy Match Plugin for TigerGraph
=====================================

* Install the Fuzzy Match plugin into the TigerGraph installation.  

  .. code-block:: bash

    /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/fuzzymatch/%FUZZYMATCH_TG_VERSION/install.sh -d xilinx_u50_gen3x16_xdma_201920_3

  Supported Alveo devices:

  * U50: xilinx_u50_gen3x16_xdma_201920_3 
  * AWS F1: xilinx_aws-vu9p-f1_shell-v04261818_201920_2 


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

Uninstalling the Fuzzy Match
--------------------------------------

You can uninstall the Fuzzy Match from TigerGraph by running the install script with the ``-u`` option:

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/fuzzymatch/%FUZZYMATCH_TG_VERSION/install.sh -u

**TIP**: To avoid TigerGraph errors, uninstall any queries and UDFs that use the Fuzzy Match before
uninstalling the Fuzzy Match itself.
