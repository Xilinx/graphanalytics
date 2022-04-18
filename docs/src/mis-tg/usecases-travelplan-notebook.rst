Run Jupyter Notebook
====================

* Install the TravelPlan Demo Plugin

.. code-block:: bash

    (fpga)$ cd travelplan
    (fpga)$ su - tigergraph
    Password:
    (fpga)$ bin/install-udf.sh


* Run the command below to start Jupyter Notebook

.. code-block:: bash

    (fpga)$ cd jupyter
    (fpga)$ jupyter notebook travelplan-jupyter-tg.ipynb

* Follow the step-by-step instructions in the notebook once it is loaded in your browser.

The Jupter Notebook demo is also available on
`Github <https://github.com/Xilinx/graphanalytics/blob/master/plugin/tigergraph/mis/examples/travelplan/jupyter/travelplan-jupyter-tg.ipynb>`_

To run the demo on a larger dataset (included under :bash:`/data` directory) that shows speedup over CPU only query,
change the filename variables in **Path Setup** cell in the Notebook as follows:

.. code-block:: python

    tp2woInfile = serverDataLocation / "travelplan2workorders2000.csv"
    tp2trInfile = serverDataLocation / "travelplan2trucks2000.csv"