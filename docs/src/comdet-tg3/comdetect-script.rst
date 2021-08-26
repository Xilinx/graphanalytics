===========================================
Bash Script with GSQL
===========================================

* Open a terminal using your Linux credentials
* Set up a directory that is readable and writable by the user "tigergraph" from
  all nodes in the same tigergraph cluster. /tg-shared is used as an example in 
  this document.
* Execute the commands below to run the demo

.. code-block:: bash

   # Copy examples from the installation to your own working directory
   cp -r /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/0.1/examples comdetec-examples
   
   cd comdetect-examples/comdetect  
   # Print comdetect demo help messages
   ./bin/run.sh -h
   
   # Run comdetect demo on a graph (.mtx) on a cluster with 3 nodes and 7 Alveo cards/nodes (21 partitions)
   ./bin/run.sh ./bin/run.sh -s /path/to/as-Skitter-wt.mtx -a /tg-share/as-Skitter-wt -m 3 -n 21




   
