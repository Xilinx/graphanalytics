Xilinx Community Detection for TigerGraph 3.x Installation
============================================================

The instructions below assume that you have completed :ref:`louvain-install-label`.

Follow the instructions in each of the sections below to install the Community Detection on your server.

Install TigerGraph Enterprise 3.x
---------------------------------

Next, follow the steps below to install TigerGraph on your server if you have not 
already.  The latest version tested with the Community Detection is version 3.1.

* Install `TigerGraph Enterprise version 3.1 <https://info.tigergraph.com/enterprise-free>`_, 
  accepting the defaults for all settings. Be sure to leave the database owner 
  as user "tigergraph", as the Community Engine install scripts depend on that 
  user name.  Make a note of the password for the user "tigergraph". That password 
  will be needed for later steps.

* Create an account in the TigerGraph installation for yourself in the role of a 
  TigerGraph application developer by issuing the commands below (replace 
  YOUR-LINUX-USERNAME with your actual Linux username). The Community Detection 
  examples will use this account to create graphs and install GSQL queries on your behalf.

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


Install the Community Detection Software
------------------------------------------

Install the Community Detection plug-in into the TigerGraph installation. This step makes TigerGraph aware
of the Community Detection features.

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/comdetect/0.5/install.sh

..  note:: 
    
    If you only need to build applications utilizing Xilinx GraphAnalytics 
    products, you can skip the section "Collaborating on Community Detection 
    Plugin" below.

Collaboring on Community Detection Plugin 
*****************************************

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
   sudo apt install --reinstall ./package/xilinx-louvainmod-0.5_18.04-x86_64.deb

* Build and install the Community Detection package. (Replace the package 
  installation command and name for your server's OS.)

.. code-block:: bash

  cd plugin/tigergraph/comdetect
  make dist
  sudo apt install --reinstall ./package/xilinx-comdetect-tigergraph-0.5_18.04-x86_64.deb

* Install the Community Detection plug-in into the TigerGraph installation.  This step makes TigerGraph aware
  of the Community Detection features.

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/comdetect/0.5/install.sh

Uninstalling the Community Detection
--------------------------------------

You can uninstall the Community Detection from TigerGraph by running the install script with the ``-u`` option:

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/comdetect/0.5/install.sh -u

**TIP**: To avoid TigerGraph errors, uninstall any queries and UDFs that use the Community Detection,
before uninstalling the Community Detection itself.
