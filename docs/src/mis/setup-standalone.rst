.. _mis-setup-standalone-label:

Setup Maximal Independent Set for Stand-Alone Runs
==============================================

..  note:: If running Python Example only, directly go to Method 2

The stand-alone MIS jupyter notebooks run on the same server that has the Alveo cards installed.
Consequently, there a few ways to run the notebooks:

Method 1: Connect to a locally running Jupyterhub server from a remote system
##################################################################
..  note:: Jupyterhub must be running with proper configuration, contact Xilinx Support for instructions

* Login to the **local** (Alveo) machine and copy examples to user accessible directory

.. code-block:: bash

    $ mkdir mis-example
    $ cd mis-example
    $ cp /opt/xilinx/apps/graphanalytics/mis/0.1/examples.zip .
    $ unzip examples.zip

* Open a web browser on your **remote** machine, enter the IP address and port of the Jupyterhub server
* Navigate to :code:`mis-example/examples/python/jupyter` and open the notebook(s)

Method 2: Run directly on the local server
#############

* To run the stand alone MIS jupyter notebooks, start by creating a python virtual environment and installing the required packages.

**Using Conda**

* Install conda by following instructions on https://docs.conda.io/en/latest/miniconda.html
* Create and activate a new virtual environment and install all required packages

.. code-block:: bash

    $ conda env list
    $ conda create -n fpga python=3.6
    $ conda activate fpga
    (fpga)$ conda install -r /opt/xilinx/apps/graphanalytics/mis/0.1/requirements.txt

**Using local python**

* At least Python version 3.6 is required, install or upgrade python using your package manager
* Create and activate a new virtual environment and install all required packages

.. code-block:: bash

    $ python3 -m venv fpga
    $ source fpga/bin/activate
    (fpga)$ pip install -r /opt/xilinx/apps/graphanalytics/mis/0.1/requirements.txt

* Now copy examples to user accessible directory

.. code-block:: bash

    (fpga)$ mkdir mis-example
    (fpga)$ cd mis-example
    (fpga)$ cp /opt/xilinx/apps/graphanalytics/mis/0.1/examples.zip .
    (fpga)$ unzip examples.zip
