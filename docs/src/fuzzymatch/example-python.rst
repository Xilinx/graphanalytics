Python Example with Fuzzy Match
===============================

* Open a terminal using your Linux credentials


* Open python virtual environment and install pandas package.

  .. code-block:: bash

    python3 -m venv venv
    . venv/bin/activate
    pip install pandas

* Execute following commands to run the example

  .. code-block:: bash

    cp -r /opt/xilinx/apps/graphanalytics/fuzzymatch/0.1/examples fuzzymatch-examples
    cd fuzzymatch-examples/python

    # Support AWS F1 and U50 device
    # Run on AWS F1:
    ./run.sh

    # Run on U50
    ./run.sh U50

