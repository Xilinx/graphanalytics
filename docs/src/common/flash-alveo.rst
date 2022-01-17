Check and install the firmware by following the steps below:

* Run ``xbutil scan`` command to check the status of all Alveo cards on the server.

.. code-block:: bash

    /opt/xilinx/xrt/bin/xbutil scan

* Look at the final rows of the output to see what firmware is installed on each card.  The example below shows the
  end of the output for a server with both Alveo U50 and U55C cards, all containing the correct shell.

.. code-block::

    [0] 0000:81:00.1 xilinx_u50_gen3x16_xdma_201920_3 user(inst=131)
    [1] 0000:04:00.1 xilinx_u55c_gen3x16_xdma_base_2 user(inst=130)

* If all cards to use contain the right shell, skip the remainder of this section.

* Issue the following command to flash the cards with required firmware version:

.. code-block:: bash

    U50
    sudo /opt/xilinx/xrt/bin/xbmgmt flash --update --shell xilinx_u50_gen3x16_xdma_201920_3

    U55C
    sudo /opt/xilinx/xrt/bin/xbmgmt flash --update --shell xilinx_u55c_gen3x16_xdma_base_2