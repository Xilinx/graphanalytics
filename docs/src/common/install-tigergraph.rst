.. _common-install-tigergraph-label:

Install TigerGraph Enterprise Software
--------------------------------------

.. note:: 
    If you already have TigerGraph 3.4 installed on the machines with Alveo U50 or U55C 
    cards, this step can be skipped. 


Next, follow the steps below to install TigerGraph on your server.  The latest version tested with Xilinx Graph Analytics products is
version 3.4.

* Download `TigerGraph Enterprise version 3.4 <https://info.tigergraph.com/enterprise-free>`_ 
  and follow installation instructions received from TigerGraph. Accept all default settings
  during the installation. Be sure to leave the database owner as user "tigergraph". Make a note 
  of the password for the user "tigergraph". That password will be needed for later steps.

* Create an account in the TigerGraph installation for yourself in the role of 
  a TigerGraph application developer by issuing the commands below (replace 
  YOUR-LINUX-USERNAME with your actual Linux username). Xilinx Graph Analytics
  products examples will use this account to create graphs and install GSQL queries 
  on your behalf.

.. code-block:: bash

   $su - tigergraph
   Password:

   # Set up tigergraph environment if needed
   $source ~/.bashrc
   
   # Set the new password to Xilinx123 if you plan to run demo scripts without entering a password.
   # Otherwise make a note of the new password. It will be needed when running demo scripts.
   $gsql
   Welcome to TigerGraph.
   GSQL > create user
   User Name : YOUR-LINUX-USERNAME
   New Password : *********
   Re-enter Password : *********
   The user "YOUR-LINUX_USERNAME" is created.
   GSQL > grant role globaldesigner to YOUR-LINUX-USERNAME
   Role "globaldesigner" is successfully granted to user(s): YOUR-LINUX-USERNAME