.. _cosinesim-setup-standalone-label:

Setup Cosine Similarity for Stand-Alone Runs
==============================================

1. Create a virtual environment and install dependencies
--------------------------------------------------------

**Using Conda**

* Install conda by following instructions on https://docs.conda.io/en/latest/miniconda.html
* Create and activate a new virtual environment and install all required packages

.. code-block:: bash

    $ conda env list
    $ conda create -n fpga python=3.6
    $ conda activate fpga
    (fpga)$ conda install -r /opt/xilinx/apps/graphanalytics/requirements.txt

**Using local python**

* At least Python version 3.6 is required, install or upgrade python using your package manager
* Create and activate a new virtual environment and install all required packages

.. code-block:: bash

    $ python3 -m venv fpga
    $ source fpga/bin/activate
    (fpga)$ pip install -r /opt/xilinx/apps/graphanalytics/requirements.txt

2. Copy examples to user accessible directory
--------------------------------------------------------

.. code-block:: bash

    (fpga)$ cp -r /opt/xilinx/apps/graphanalytics/cosinesim/%COSINESIM_VERSION/examples cosinesim-examples
    (fpga)$ cd cosinesim-examples/python

---------------------------

Setup for Jupyter Notebooks
---------------------------

..  note:: If running Jupyter Notebook(s), read this section and follow instructions in the
           corresponding Notebook Demo pages to run.

The stand-alone cosine similarity jupyter notebooks run on the same server (**local**) that has the Alveo cards installed,
but can be launched locally or remotely from a system **that is on the same network**. Consequently, there are
two ways to run the notebooks:

Method 1: Run on the local server
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Simply start the jupyter server as:

.. code-block:: bash

    (fpga)$ ./run.sh jupyter notebook

* Navigate to the Notebook under current directory that you want to run

Method 2: Run from a remote machine
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* JupyterHub is not part of the requirements.txt dependency list. Install JupyterHub on the **local**
  machine as shown `here <https://jupyterhub.readthedocs.io/en/stable/quickstart.html#installation>`_
* Start a JupyterHub server on the **local** machine as:

.. code-block:: bash

    (fpga)$ ./run.sh jupyterhub

* Open a web browser on your **remote** machine, enter the IP address and port of the Jupyterhub server
* Navigate to the Notebook under current directory that you want to run
