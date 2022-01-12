.. _install-demo-plugin-label:

Installing and Uninstalling the Demo use cases
==============================================

As with your own application based on TigerGraph and Xilinx Graph Analytics 
products, the demo use cases require separate installation and uninstallation 
steps.

Installing the Demo
-------------------

The demo applications require that demo plugin be installed first manually.

Run the install script located in the ``bin`` subdirectory under the corresponding demo directory

.. code-block:: bash

    $ cd <Demo Name> # e.g. cd synthea
    $ su - tigergraph
    Password:
    $ bin/install_udf.sh

Uninstalling the Demo
---------------------

The demo applications do not uninstall the demo and the process for uninstalling is manual.
Follow the steps below to uninstall the demo.

Delete the Data
***************

At a minimum, all queries need to be deleted before uninstalling the demo's UDFs.  If you are replacing the demo
with your own application, it would be best to delete all graph data, as demonstrated below.

.. code-block:: bash

   $ su - tigergraph
   Password:
   $ gsql
   Welcome to TigerGraph.
   GSQL > drop all

Uninstall the UDFs
******************

To uninstall the demo's UDFs from TigerGraph, use the install script located in the ``bin`` subdirectory under
the Synthea demo directory.  The commands below assume you are running from the demo directory.

.. code-block:: bash

   $ su - tigergraph
   Password:
   $ bin/install_udf.sh -u

Uninstalling the demo UDFs leaves main plugin UDFs in place.
