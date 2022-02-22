.. mis-setup-tg3-label:

=============================================
Set up Maximal Independent Set for TigerGraph 3.x
=============================================

.. include:: ../common/install-conda-python.rst


2. Copy examples to user accessible directory
---------------------------------------------

.. code-block:: bash

    (fpga)$ mkdir mis-example
    (fpga)$ cd mis-example
    (fpga)$ cp -r /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/mis/%MIS_TG_VERSION/examples .


3. Install demo specific plugin
-------------------------------

..  note:: The demo plugin needs to be installed only once

.. toctree::
    :maxdepth: 1

    ../common/setup-demo-plugin.rst


4. Follow instructions in the Demo use cases pages to run the corresponding Notebooks
-------------------------------
