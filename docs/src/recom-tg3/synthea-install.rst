Installing and Uninstalling the Synthea Demo
============================================

As with your own application based on TigerGraph and the Recommendation Engine, the Synthea demo
requires separate installation and uninstallation steps.

Installing the Demo
-------------------

Because the client applications of the demo call the setup scripts themselves, the demo is self-installing.

However, for the sake of understanding how the installation process works, you can examine the scripts
``init_graph.sh``, which creates the patient graph, and ``install_query.sh``, which installs UDFs and queries.
Both scripts are located at the top level of the Synthea demo directory.

Uninstalling the Demo
---------------------

Because the demo applications do not uninstall the demo, the process for uninstalling is manual.
Follow the steps below to uninstall the demo.

Delete the Data
***************

At a minimum, all queries need to be deleted before uninstalling the demo's UDFs.  If you are replacing the demo
with your own application, it would be best to delete all graph data, as demonstrated below.

.. code-block:: bash

   $su - tigergraph
   Password:
   $gsql
   Welcome to TigerGraph.
   GSQL > drop all

Uninstall the UDFs
******************

To uninstall the demo's UDFs from TigerGraph, use the install script located in the ``bin`` subdirectory under
the Synthea demo directory.  The commands below assume you are running from the demo directory.

.. code-block:: bash

   $su - tigergraph
   Password:
   $bin/install_udf.sh -u

Uninstalling the demo UDFs leaves the Recommendation Engine UDFs in place.
