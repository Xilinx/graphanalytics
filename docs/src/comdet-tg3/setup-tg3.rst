.. _louvain-setup-tg3-label:

=============================================
Set up Community Detection for TigerGraph 3.x
=============================================

The Community Detection consists of Louvain Modularity library integrated into 
TigerGraph 3.x via plugins and UDFs.

.. include:: ../common/install-conda-python.rst


2. Copy examples to user accessible directory
---------------------------------------------

.. code-block:: bash

    (fpga)$ mkdir comdetect-example
    (fpga)$ cd comdetect-example
    (fpga)$ cp -r /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/comdetect/1.4/examples .


3. Install demo specific plugin
-------------------------------

..  note:: The demo plugin needs to be installed only once

.. toctree::
    :maxdepth: 1

    ../common/setup-demo-plugin.rst


4. Follow instructions in the Demo use cases pages to run the corresponding Notebooks
-------------------------------
