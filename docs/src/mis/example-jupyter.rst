Jupyter Notebook
================

Follow the setup process below before running the notebook:

.. toctree::
    :maxdepth: 1

    Setup <../common/setup-python-standalone.rst>

Run following commands in the Python virtual environment (from setup) to run the notebook:

.. code-block:: bash

    (fpga)$ cp -r /opt/xilinx/apps/graphanalytics/mis/%MIS_VERSION/examples mis-examples
    (fpga)$ cd mis-examples/python
    (fpga)$ ./run.sh jupyter notebook jupyter/misdemo_notebook.ipynb


The Jupyter Notebook demo is also available on
`Github <https://github.com/Xilinx/graphanalytics/blob/master/mis/examples/python/jupyter/misdemo_notebook.ipynb>`_
