==============================================================
Xilinx Maximal Independent Set for TigerGraph 3.x Installation
==============================================================

Follow the instructions in each of the sections below to install the Maximal 
Independent Set on your server.

.. include:: ../common/install-alveo.rst

.. include:: ../common/install-tigergraph.rst

Install Maximal Independent Set Software
----------------------------------------

To install the Maximal Independent Set product, you can either install a DEB/RPM 
pre-built package, or you can build the Maximal Independent Set from sources if 
you would like to collaborate with Xilinx and contribute to the Maximal Independent Set
product. Follow the steps in *only one* of the two sections below.

Install the Maximal Independent Set from a Package
**************************************************

* Get the installation package **xilinx-tigergraph-install-%PACKAGE_VERSION.tar.gz** from the
  `Database Analytics POC Secure Site <%PACKAGE_LINK>`_.
  This package contains the Maximal Independent Set as well as its dependencies.

* Install the product and its dependencies by un-tarring the package and running
  the included install script.

  .. code-block:: bash

    tar xzf xilinx-tigergraph-install-%PACKAGE_VERSION.tar.gz
    cd xilinx-tigergraph-install && ./install.sh -p mis

* Install the Maximal Independent Set plugin into the TigerGraph installation 
  targeting a supported Alveo device. Below is an example targeting Alveo U50"

  .. code-block:: bash

    /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/mis/%MIS_TG_VERSION/install.sh -d xilinx_u50_gen3x16_xdma_201920_3

  Supported Alveo devices:

  * U50: xilinx_u50_gen3x16_xdma_201920_3 
  * U55C: xilinx_u55c_gen3x16_xdma_base_2   

..  note:: 
    
    If you only need to build applications utilizing Xilinx GraphAnalytics 
    products, you can skip the section below.

Build the Maximal Independent Set from Sources
**********************************************

Follow the instructions below if you want to collaborate and contribute to the 
Xilinx Maximal Independent Set Alveo Product and plugin for TigerGraph 3.x.  
Both products reside in one GitHub repository.

* Clone the Xilinx Graph Analytics repository from GitHub.

  .. code-block:: bash

    git clone https://github.com/Xilinx/graphanalytics.git

* Build and install the Maximum Independent Set package. The Ubuntu apt package manager is used as an example.

  .. code-block:: bash

    cd mis
    make dist
    sudo apt install --reinstall ./package/xilinx-mis-%MIS_VERSION_20.04-x86_64.deb

* Build and install the Maximal Independent Set for TigerGraph 3.x package. (Replace the package
  installation command and name for your server's OS.)

  .. code-block:: bash

    cd plugin/tigergraph/mis
    make dist
    sudo apt install --reinstall ./package/xilinx-mis-tigergraph-%MIS_TG_VERSION_20.04-x86_64.deb

    /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/mis/%MIS_TG_VERSION/install.sh -d xilinx_u50_gen3x16_xdma_201920_3

  Supported Alveo devices:

  * U50: xilinx_u50_gen3x16_xdma_201920_3 
  * U55C: xilinx_u55c_gen3x16_xdma_base_2   


Flash the Alveo Cards
---------------------

The Maximal Independent Set requires the xilinx_u50_gen3x16_xdma_201920_3 shell to be installed on each
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


Uninstalling the Maximal Independent Set
--------------------------------------

You can uninstall the Maximal Independent Set from TigerGraph by running the install script with the ``-u`` option:

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/mis/%MIS_TG_VERSION/install.sh -u

**TIP**: To avoid TigerGraph errors, uninstall any queries and UDFs that use the Maximal Independent Set before
uninstalling the Maximal Independent Set itself.
