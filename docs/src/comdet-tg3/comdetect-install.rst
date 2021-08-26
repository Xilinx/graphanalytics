Installing and Uninstalling the Community Detection Demo
========================================================

Installing the Demo UDF 
-----------------------

Run the command below to install UDFs specific to the community detection demo:

.. code-block:: bash

    /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/0.1/examples/comdetect/bin/install-udf.sh


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

Run the command below to uninstall the UDFs for this demo:

.. code-block:: bash

   /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/0.1/examples/comdetect/bin/install-udf.sh -u

Uninstalling the demo UDFs leaves the Recommendation Engine UDFs in place.
