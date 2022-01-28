===========================================
Run Bash Script with GSQL
===========================================

* Open a terminal using your Linux credentials
* Set up a directory that is readable and writable by the user "tigergraph" from
  all nodes in the same tigergraph cluster. /tg-shared is used as an example in 
  this document.
* Execute the commands below to run the demo

.. code-block:: bash

   # Copy examples from the installation to your own working directory
   cp -r /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/comdetect/%COMDETECT_TG_VERSION/examples comdetect-examples
   
   cd comdetect-examples/comdetect
   # Print comdetect demo help messages
   ./bin/run.sh -h
   
   # Run comdetect demo with default settings. Settings can be changed via command 
   # line options as shown above
   ./bin/run.sh 




   
