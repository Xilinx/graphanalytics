Xilinx Fuzzy Match for TigerGraph 3.x Installation
==================================================

Follow the instructions in each of the sections below to install the Fuzzy Match  
products on your server.

.. include:: ../install-alveo.rst
.. include:: ../install-tigergraph.rst

Install the Fuzzy Match Software
--------------------------------------------

To install the Fuzzy Match product, you can either install a DEB/RPM 
pre-built package, or you can build the Fuzzy Match from sources if 
you would like to collaborate with Xilinx and contribute to the Fuzzy Match
product. Follow the steps in *only one* of the two sections below.

Install the Fuzzy Match from a Package
**************************************

* Get the installation package xilinx-tigergraph-install-x.y.z.tar.gz from the
  `Database Analytics POC Secure Site <https://www.xilinx.com/member/dba_poc.html>`_.  
  This package contains the Fuzzy Match as well as its dependencies: 
  XRT, XRM, the Alveo U50 Platform, and the Fuzzy Match Alveo Product.

* Install the product and its dependencies by un-tarring the package and running
  the included install script.

.. code-block:: bash

   tar xzf xilinx-tigergraph-install-x.y.z.tar.gz
   cd xilinx-tigergraph-install && ./install.sh -p fuzzymatch

* Install the Fuzzy Match plug-in into the TigerGraph installation.  
  This step makes TigerGraph aware of the Fuzzy Match features.

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.0/install.sh


.. include:: ../fuzzymatch/install-collaborate.rst

* Build and install the Fuzzy Match package. (Replace the package 
  installation command and name for your server's OS.)

.. code-block:: bash

  cd plugin/tigergraph/fuzzymatch
  make dist
  sudo apt install --reinstall ./package/xilinx-fuzzymatch-tigergraph-0.1_20.04-x86_64.deb

* Install the Fuzzy Match plug-in into the TigerGraph installation.  This step makes TigerGraph aware
  of the Fuzzy Match features.

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.0/install.sh

Flash the Alveo Cards
---------------------

The Fuzzy Match requires the xilinx_u50_gen3x16_xdma_201920_3 shell to be installed on each
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


Uninstalling the Fuzzy Match
--------------------------------------

You can uninstall the Fuzzy Match from TigerGraph by running the install script with the ``-u`` option:

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.0/install.sh -u

**TIP**: To avoid TigerGraph errors, uninstall any queries and UDFs that use the Fuzzy Match,
such as the Synthea Demo, before uninstalling the Fuzzy Match itself.
