.. _cosinesim-setup-tg3-label:

==============================================
Setup Recommendation Engine for TigerGraph 3.x run
==============================================

The Recommendation Engine consists of Cosine Similarity library integrated into TigerGraph 3.x via plugins and UDFs.

.. note:: The Recommendation Engine Jupyter Notebooks can be run from any machine that is on the same network as the TigerGraph server i.e. can connect via HTTP connections. These Notebooks use **pytigerGraph** module to connect to the TigerGraph server via REST APIs.

* To run the TigerGraph jupyter notebooks, start by creating a python virtual environment and installing the required packages.

**using Conda**

    * Install conda by following instructions on https://docs.conda.io/en/latest/miniconda.html

    * Create and activate a new virtual environment and install all required packages

    .. code-block:: bash

        $ conda env list
        $ conda create -n fpga python=3.6
        $ conda activate fpga
        (fpga)$ conda install -r /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.4/requirements.txt

**using local python**

    * At least Python version 3.6 is required, install or upgrade python using your package manager

    * Create and activate a new virtual environment and install all required packages

    .. code-block:: bash

        $ python3 -m venv fpga
        $ source fpga/bin/activate
        (fpga)$ pip install -r /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.4/requirements.txt

* Now copy examples to user accessible directory

.. code-block:: bash

    (fpga)$ mkdir recomengine-example
    (fpga)$ cd recomengine-example
    (fpga)$ cp -r /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.4/examples .


* (For info purpose only) To understand the Demo specific plugin, go to the section below:

    .. toctree::
        :maxdepth: 1

        Demo Plugin Installation <setup-demo-plugin-install.rst>

* Now follow instructions in the Demo use cases pages to run the corresponding Notebooks