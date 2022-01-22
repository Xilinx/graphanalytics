C++ Example of Fuzzy Match using Makefile
===========================================

* Open a terminal using your Linux credentials
* Execute following commands to run the example

  .. code-block:: bash
  
    cp -r /opt/xilinx/apps/graphanalytics/fuzzymatch/0.2/examples fuzzymatch-examples
    cd fuzzymatch-examples/cpp
    
    # Print makefile usage and options
    make help

    # Run the demo on U50 card
    make run

    # Run the demo on AWS F1
    make run deviceNames=xilinx_aws-vu9p-f1_shell-v04261818_201920_2

    # Run the demo on U55C
    make run deviceNames=xilinx_u55c_gen3x16_xdma_base_2    
