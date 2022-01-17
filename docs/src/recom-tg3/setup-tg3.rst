.. _cosinesim-setup-tg3-label:

==================================================
Setup Recommendation Engine for TigerGraph 3.x run
==================================================

The Recommendation Engine consists of Cosine Similarity library integrated into 
TigerGraph 3.x via plugins and UDFs.

.. include:: ../common/install-conda-python.rst

* Now copy examples to user accessible directory

.. code-block:: bash

    (fpga)$ mkdir recomengine-example
    (fpga)$ cd recomengine-example
    (fpga)$ cp -r /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.4/examples .

* Install demo specific plugin by following the instructions below. Note 
  the demo plugin is only needed to be installed once.

  .. toctree::
    :maxdepth: 1

    ../setup-demo-plugin.rst

* Now follow instructions in the Demo use cases pages to run the corresponding Notebooks