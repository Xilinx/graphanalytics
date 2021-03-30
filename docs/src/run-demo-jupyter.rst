===========================================
Run Demo in Jupyter Notebook
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

    (alveo-demo)$cd graphanalytics/plugin/tigergraph/jupyter-demo
    (alveo-demo)$jupyter notebook

Below is a snapshot of the TG-demo notebook:

.. image:: /images/tg-demo-jupyter.png
   :alt: Xilinx Tigergraph Plugin
   :scale: 60%
   :align: center