=========================
Run Bash Script with GSQL
=========================

* Open a terminal using your Linux credentials
* Set up a directory that is readable and writable by the user "tigergraph"
* Execute the command below to run the example

.. code-block:: bash

   # Copy examples from the installation to your own working directory
   cp -r /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/recomengine/%RECOMENGINE_TG_VERSION/examples recomengine-examples

   cd recomengine-examples/synthea

   # Install demo specific UDFs. This is needed to run **ONLY ONCE**.
   ./bin/install-udf.sh

   # Print synthea demo help messages
   ./bin/run.sh -h

   # Run synthea demo with default settings. Settings can be changed via command
   # line options as shown above
   ./bin/run.sh -u <username> -p <password>
