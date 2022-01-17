.. _fuzzymatch-setup-tg3-label:

====================================
Set up Fuzzy Match TigerGraph Plugin
====================================

The Fuzzy Match plugin consists of Fuzzy Match library integrated into TigerGraph 
via UDFs.

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