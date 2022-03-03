Run Bash Script with GSQL
=============

* Open a terminal using your Linux credentials
* Set up a directory that is readable and writable by the user "tigergraph"
* Execute the commands below to run the demo

.. code-block:: bash

   # Copy examples from the installation to your own working directory mis-examples
   cp -r /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/mis/%MIS_TG_VERSION/examples mis-examples

   cd mis-examples/travelplan

   # Install demo specific UDFs. This is needed to run **ONLY ONCE**.
   ./bin/install-udf.sh

   # Print travelplan demo help messages
   ./bin/run.sh -h

   # Run travelplan demo with default settings. Settings can be changed via command
   # line options as shown above
   ./bin/run.sh