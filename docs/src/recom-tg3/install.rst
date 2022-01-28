Xilinx Recommendation Engine for TigerGraph 3.x Installation
============================================================

Follow the instructions in each of the sections below to install the Recommendation Engine on your server.

Install the Alveo Acclerator Card
---------------------------------

The Recommendation Engine requires one or more Xilinx Alveo accelerator cards to be installed on your server.
At this time, the only Alveo card that the Recommendation Engine supports is the Alveo U50.  Follow the steps
below to install the card(s).

* Power off the server.
* Plug Xilinx U50 Alveo card into a PCIe Gen3 x16 slot.
* Power on the server.

Install TigerGraph Enterprise 3.x
---------------------------------

Next, follow the steps below to install TigerGraph on your server if you have not already.  The latest version tested
with the Recommendation Engine is version 3.1.

* Install `TigerGraph Enterprise version 3.1 <https://info.tigergraph.com/enterprise-free>`_, accepting the defaults
  for all settings that have them.  Be sure to leave the database owner as user "tigergraph", as the Recommendation
  Engine install scripts depend on that user name.  Make a note of the password for the user 
  "tigergraph". That password will be needed for later steps.

* Create an account in the TigerGraph installation for yourself in the role of a TigerGraph application developer
  by issuing the commands below (replace YOUR-LINUX-USERNAME with your actual Linux username). 
  The Recommendation Engine examples will use this account to create graphs and install GSQL queries on your behalf.

.. code-block:: bash

   $su - tigergraph
   Password:
   $gsql
   Welcome to TigerGraph.
   GSQL > create user
   User Name : YOUR-LINUX-USERNAME
   New Password : *********
   Re-enter Password : *********
   The user "YOUR-LINUX_USERNAME" is created.
   GSQL > grant role globaldesigner to YOUR-LINUX-USERNAME
   Role "globaldesigner" is successfully granted to user(s): YOUR-LINUX-USERNAME


Install the Recommendation Engine Software
------------------------------------------

To install the Recommendation Engine product, you can either install a DEB/RPM pre-built package, or you can build the
Recommendation Engine from sources if you would like to collaborate with Xilinx and contribute to the Recommendation
Engine product.  Follow the steps in *only one* of the two sections below.

Install the Recommendation Engine from a Package
************************************************

* Get the installation package **xilinx-tigergraph-install-%PACKAGE_VERSION.tar.gz** from the
  `Database Analytics POC Secure Site <%PACKAGE_LINK>`_.  This package contains
  the Recommendation Engine as well as its dependencies: XRT, XRM, the Alveo U50 Platform, and the Cosine Similarity
  Alveo Product.

* Install the Recommendation Engine product and its dependencies by un-tarring the package and running
  the included install script.

.. code-block:: bash

   tar xzf xilinx-tigergraph-install-%PACKAGE_VERSION.tar.gz
   cd xilinx-tigergraph-install && ./install.sh

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

Flash the Alveo Cards
---------------------

The Recommendation Engine requires the xilinx_u50_gen3x16_xdma_201920_3 shell to be installed on each
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


Uninstalling the Recommendation Engine
--------------------------------------

You can uninstall the Recommendation Engine from TigerGraph by running the install script with the ``-u`` option:

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/recomengine/%RECOMENGINE_TG_VERSION/install.sh -u

**TIP**: To avoid TigerGraph errors, uninstall any queries and UDFs that use the Recommendation Engine,
such as the Synthea Demo, before uninstalling the Recommendation Engine itself.
