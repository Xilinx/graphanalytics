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

.. role:: bash(code)
   :language: bash

To run the demo on a larger dataset (included under :bash:`/data` directory) that shows speedup over CPU only query, run
with full path to data files as follows (assuming working under home directory).

.. code-block:: bash

    # Run travelplan speedup demo
    ./bin/run.sh tp2tr=$HOME/mis-examples/travelplan/data/travelplan2trucks2000.csv tp2wo=$HOME/mis-examples/travelplan/data/travelplan2workorders2000.csv runnMode=3
