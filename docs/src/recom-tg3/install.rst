AMD Recommendation Engine for TigerGraph Installation
=====================================================

Install Recommendation Engine Software
---------------------------------------

To install the Recommendation Engine product, you can either install a DEB/RPM pre-built package, or you can build the
Recommendation Engine from sources if you would like to collaborate with Xilinx and contribute to the Recommendation
Engine product.  Follow the steps in *only one* of the two sections below.

Install the Recommendation Engine from a Package
************************************************

* Get the installation package **amd-graphanalytics-install-%PACKAGE_VERSION.tar.gz** from the
  `Database Analytics POC Secure Site <%PACKAGE_LINK>`_.  This package contains
  the Recommendation Engine as well as its dependencies: XRT, XRM, the Alveo U50 Platform, and the Cosine Similarity
  Alveo Product.

* Install the Recommendation Engine product and its dependencies by un-tarring the package and running
  the included install script.

.. code-block:: bash

   tar xzf amd-graphanalytics-install-%PACKAGE_VERSION.tar.gz
   cd amd-graphanalytics-install && ./install.sh

* Install the Recommendation Engine plug-in into the TigerGraph installation.  This step makes TigerGraph aware
  of the Recommendation Engine features.

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/recomengine/%RECOMENGINE_TG_VERSION/install.sh

Build the Recommendation Engine from Sources
********************************************

Follow the instructions below if you want to collaborate and contribute to the Xilinx Cosine Similarity Alveo Product
and Recommendation Engine for TigerGraph 3.x.  Both products reside in one GitHub repository.

* Install the Xilinx XRT and XRM Libraries

    - Download the 2020.2 version of XRT for the Alveo U50 accelerator card from
      `xilinx.com <https://www.xilinx.com/products/boards-and-kits/alveo/u50.html#gettingStarted>`_ and install it.

    - Follow the instructions from this
      `Xilinx developer page <https://developer.xilinx.com/en/articles/orchestrating-alveo-compute-workloads-with-xrm.html>`_
      to build and install XRM.

* Download the 2020.2 version of the shell for the Alveo U50 accelerator card from
  `xilinx.com <https://www.xilinx.com/products/boards-and-kits/alveo/u50.html#gettingStarted>`_ and install it.
  The package to download is called the "Deployment Target Platform."

* Clone the Xilinx Graph Analytics repository from GitHub.

.. code-block:: bash

   git clone https://github.com/Xilinx/graphanalytics.git

* Build and install the Cosine Similarity package. The Ubuntu apt package manager is used as an example.

.. code-block:: bash

   cd cosinesim
   make dist
   sudo apt install --reinstall ./package/xilinx-cosinesim-%COSINESIM_VERSION_18.04-x86_64.deb

* Build and install the Recommendation Engine package. (Replace the package 
  installation command and name for your server's OS.)

.. code-block:: bash

  cd plugin/tigergraph/recomengine
  make dist
  sudo apt install --reinstall ./package/xilinx-recomengine-tigergraph-%RECOMENGINE_TG_VERSION_18.04-x86_64.deb

* Install the Recommendation Engine plug-in into the TigerGraph installation.  This step makes TigerGraph aware
  of the Recommendation Engine features.

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/recomengine/%RECOMENGINE_TG_VERSION/install.sh

Uninstalling the Recommendation Engine
--------------------------------------

You can uninstall the Recommendation Engine from TigerGraph by running the install script with the ``-u`` option:

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/recomengine/%RECOMENGINE_TG_VERSION/install.sh -u

**TIP**: To avoid TigerGraph errors, uninstall any queries and UDFs that use the Recommendation Engine,
such as the Synthea Demo, before uninstalling the Recommendation Engine itself.
