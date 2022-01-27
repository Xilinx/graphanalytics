.. _cosinesim-setup-tg3-label:

==================================================
Setup Recommendation Engine for TigerGraph 3.x run
==================================================

The Recommendation Engine consists of Cosine Similarity library integrated into 
TigerGraph 3.x via plugins and UDFs.

.. include:: ../common/install-conda-python.rst


2. Copy examples to user accessible directory
---------------------------------------------

.. code-block:: bash

    (fpga)$ mkdir recomengine-example
    (fpga)$ cd recomengine-example
    (fpga)$ cp -r /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/recomengine/1.4/examples .


3. Install demo specific plugin
-------------------------------

..  note:: The demo plugin needs to be installed only once

.. toctree::
    :maxdepth: 1

    ../common/setup-demo-plugin.rst


4. Follow instructions in the Demo use cases pages to run the corresponding Notebooks
-------------------------------
