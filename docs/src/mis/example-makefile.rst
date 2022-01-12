===========================================
C++ Example using Makefile
===========================================

* Open a terminal using your Linux credentials
* Execute following commands to run the example

    .. code-block:: bash

        cp -r /opt/xilinx/apps/graphanalytics/mis/0.1/examples mis-examples
        cd mis-examples/cpp
    
        # Print makefile usage and options
        make help
    
        # Run the demo on U50 Alveo card
        make run
    
        # Run the demo on U55C Alveo card
        make run deviceNames=xilinx_u55c_gen3x16_xdma_base_2