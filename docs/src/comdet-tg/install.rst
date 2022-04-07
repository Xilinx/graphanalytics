Xilinx Community Detection for TigerGraph Installation
======================================================

---------------------------------------

Install the Community Detection Software
------------------------------------------

Follow the steps in *only one* of the two sections below.

Install the Community Detection from a Package
************************************************

* Get the installation package **amd-graphanalytics-install-%PACKAGE_VERSION.tar.gz** from the
  `Database Analytics POC Secure Site <%PACKAGE_LINK>`_.  This package contains
  the Community Detection as well as its dependencies: XRT, XRM, the Alveo U50 Platform, and the Louvain Modularity
  Alveo Product.

* Install the Community Detection product and its dependencies by un-tarring the package and running
  the included install script.

.. code-block:: bash

   tar xzf amd-graphanalytics-install-%PACKAGE_VERSION.tar.gz
   cd amd-graphanalytics-install && ./install.sh

* Install the Community Detection plug-in into the TigerGraph installation. This step makes TigerGraph aware
  of the Community Detection features.

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/comdetect/%COMDETECT_TG_VERSION/install.sh

..  note:: 
    
    If you only need to build applications utilizing Xilinx GraphAnalytics 
    products, you can skip the section "Collaborating on Community Detection 
    Plugin" below.

Collaborating on Community Detection Plugin
*******************************************

Follow the instructions below if you want to collaborate and contribute to the 
Xilinx Louvain Modularity Alveo Product and Community Detection for TigerGraph 3.x.
Both products reside in one GitHub repository.

* Clone the Xilinx Graph Analytics repository from GitHub.

.. code-block:: bash

   git clone https://github.com/Xilinx/graphanalytics.git

* Build and install the Louvain Modularity package. The Ubuntu apt package manager is used as an example.

.. code-block:: bash

   cd louvainmod
   make dist
   sudo apt install --reinstall ./package/xilinx-louvainmod-%LOUVAIN_VERSION_18.04-x86_64.deb

* Build and install the Community Detection package. (Replace the package 
  installation command and name for your server's OS.)

.. code-block:: bash

  cd plugin/tigergraph/comdetect
  make dist
  sudo apt install --reinstall ./package/xilinx-comdetect-tigergraph-%COMDETECT_TG_VERSION_18.04-x86_64.deb

* Install the Community Detection plug-in into the TigerGraph installation.  This step makes TigerGraph aware
  of the Community Detection features.

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/comdetect/%COMDETECT_TG_VERSION/install.sh

---------------------------------------

Uninstalling the Community Detection
--------------------------------------

You can uninstall the Community Detection from TigerGraph by running the install script with the ``-u`` option:

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/comdetect/%COMDETECT_TG_VERSION/install.sh -u

**TIP**: To avoid TigerGraph errors, uninstall any queries and UDFs that use the Community Detection,
before uninstalling the Community Detection itself.
