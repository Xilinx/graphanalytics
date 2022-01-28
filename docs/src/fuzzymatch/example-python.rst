Python API Example
==================

* Open a terminal using your Linux credentials


* Open python virtual environment and install pandas package.

  .. code-block:: bash

    $ python3 -m venv fpga
    $ . fpga/bin/activate
    (fpga)$ pip install -r /opt/xilinx/apps/graphanalytics/requirements.txt

* Execute following commands to run the example

  .. code-block:: bash
    
    (fpga)$ cp -r /opt/xilinx/apps/graphanalytics/fuzzymatch/%FUZZYMATCH_VERSION/examples fuzzymatch-examples
    (fpga)$ cd fuzzymatch-examples/python

    # Support AWS F1 and U50 device
    # Run on AWS F1:
    (fpga)$ ./run.sh

    # Run on U50
    (fpga)$ ./run.sh U50

    # Run on U55C
    (fpga)$ ./run.sh U55C

