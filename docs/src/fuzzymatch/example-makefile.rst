C++ Example of Fuzzy Match using Makefile
===========================================

* Open a terminal using your Linux credentials
* Execute following commands to run the example

  .. code-block:: bash
  
    # replace the ${VERSION} to the version you want to run 
    # so far VERSION list: 0.1, 0.2
    cp -r /opt/xilinx/apps/graphanalytics/fuzzymatch/${VERSION}/examples fuzzymatch-examples
    cd fuzzymatch-examples/cpp
    
    # Print makefile usage and options
    make help

    # Run the demo on U50 card
    make run

    # Run the demo on AWS F1
    make run deviceNames=xilinx_aws-vu9p-f1_shell-v04261818_201920_2
