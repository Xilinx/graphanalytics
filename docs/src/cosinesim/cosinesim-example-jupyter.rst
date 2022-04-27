===========================================
Jupyter Notebook
===========================================

Follow the setup process below before running the notebook:

.. toctree::
    :maxdepth: 1

    Setup <../common/setup-python-standalone.rst>

Run following commands in the Python virtual environment (from setup) to run the Notebook:

.. code-block:: bash

    (fpga)$ cp -r /opt/xilinx/apps/graphanalytics/cosinesim/%COSINESIM_VERSION/examples cosinesim-examples
    (fpga)$ cd cosinesim-examples/python
    (fpga)$ ./run.sh jupyter notebook jupyter/jupyter_demo.ipynb


The Jupyter Notebook demo is also available on
`Github <https://github.com/Xilinx/graphanalytics/blob/master/cosinesim/examples/python/jupyter/jupyter_demo.ipynb>`_

Watch `Standalone Cosine Similarity Direct Python Webinar 
<https://www.xilinx.com/video/application/standalone-cosine-similarity-direct-python-webinar.html>`_
to learn more about this demo.
