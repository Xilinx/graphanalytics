Install Conda or Python Environment
===================================

.. note:: 
    Demo Jupyter Notebooks can be run from any machine that is on the same network 
    as the TigerGraph server i.e. can connect via HTTP connections. These Notebooks 
    use **pytigerGraph** module to connect to the TigerGraph server via REST APIs.

* To run the TigerGraph jupyter notebooks, start by creating a python virtual 
  environment and installing the required packages.

**Using Conda**

* Install conda by following instructions on https://docs.conda.io/en/latest/miniconda.html

* Create and activate a new virtual environment and install all required packages

.. code-block:: bash

    $ conda env list
    $ conda create -n fpga python=3.6
    $ conda activate fpga
    (fpga)$ conda install -r /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.4/requirements.txt

**Using local python**

* At least Python version 3.6 is required, install or upgrade python using your package manager

* Create and activate a new virtual environment and install all required packages

.. code-block:: bash

    $ python3 -m venv fpga
    $ source fpga/bin/activate
    (fpga)$ pip install -r /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.4/requirements.txt