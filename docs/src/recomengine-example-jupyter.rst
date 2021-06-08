===========================================
Jupyter Notebook with PyTigerGraph
===========================================

Install Jupyter Notebook in a conda environment
-----------------------------------------------

NOTE: The Jupyter Notebook can be installed on any machine provided the machine
can connect to TigerGraph server via HTTP connections.

* Install conda by following instructions on https://docs.conda.io/en/latest/miniconda.html

* Create and activate a new conda environment and install all required packages

.. code-block:: bash

  $conda env list
  $conda create -n fpga python=3.6
  $conda activate fpga
  (fpga)$git clone https://github.com/Xilinx/graphanalytics
  (fpga)$cd graphanalytics
  (fpga)$conda install -r requirements.txt


Start Jupyter Notebook
------------------------

Run the command below to start Jupyter Notebook

.. code-block:: bash

    (fpga)$cd graphanalytics/plugin/tigergraph/examples/synthea/jupyter-demo
    (fpga)$jupyter notebook

Follow the step-by-step instructions in the notebook once it is loaded in your browser.

The Jupter Notebook demo is also available on 
`Github <https://github.com/Xilinx/graphanalytics/blob/master/plugin/tigergraph/examples/synthea/jupyter-demo/TG_demo.ipynb>`_

Watch `Recommendation Engine Jupyter Notebook Demo Webinar 
<https://www.xilinx.com/video/application/recommendation-engine-accelerated-tigergraph-webinar.html>`_
to learn more about this demo.
