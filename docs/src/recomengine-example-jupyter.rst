===========================================
Jupyter Notebook with PyTigerGraph
===========================================

Install Jupyter Notebook in a conda environment
-----------------------------------------------

NOTE: The Jupyter Notebook can be installed on any machine provided the machine
can connect to TigerGraph server via HTTP connections.

* Install conda by following instructions on 
  https://conda.io/projects/conda/en/latest/user-guide/install/index.html

* Create and activate a new conda environment 

.. code-block:: bash

  $conda env list
  $conda create -n alveo-demo python=3.6
  $conda activate alveo-demo

* Install Jupyter Notebook and PyTigerGraph 

.. code-block:: bash

    (alveo-demo)$conda install -c conda-forge notebook jupyter_contrib_nbextensions
    (alveo-demo)$pip install PyTigerGraph


Start Jupyter Notebook
------------------------

Run the command below to start Jupyter Notebook

.. code-block:: bash

    (alveo-demo)$cd /opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.0/examples/synthea/jupyter-demo
    (alveo-demo)$jupyter notebook

Follow the step-by-step instructions in the notebook once it is loaded in your browser.

The Jupter Notebook demo is also available on 
`Github <https://github.com/Xilinx/graphanalytics/blob/master/plugin/tigergraph/examples/synthea/jupyter-demo/TG_demo.ipynb>`_

