Install Conda or Python Environment
===================================

.. note:: 
    **TigerGraph users ONLY**: Demo Jupyter Notebooks can be run from any machine that is on the same network
    as the TigerGraph server i.e. can connect via HTTP connections. These Notebooks 
    use **pyTigerGraph** module to connect to the TigerGraph server via REST APIs.

Create a virtual environment using *any* of the following methods and install dependencies.

Using Conda
-----------

* Install conda by following instructions on https://docs.conda.io/en/latest/miniconda.html

* Create and activate a new virtual environment and install all required packages

.. code-block:: bash

    $ conda env list
    $ conda create -n fpga python=3.6
    $ conda activate fpga
    (fpga)$ pip install -r /opt/xilinx/apps/graphanalytics/requirements.txt

Using local python
------------------

* At least Python version 3.6 is required, install or upgrade python using your package manager

* Create and activate a new virtual environment and install all required packages

.. code-block:: bash

    $ python3 -m venv fpga
    $ source fpga/bin/activate
    (fpga)$ pip install -r /opt/xilinx/apps/graphanalytics/requirements.txt