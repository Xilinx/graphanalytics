Wikipedia Search Engine
=======================

The Wikipedia Search Engine takes the given heyword/phrase and finds top matching 
Wikipedia pages using Cosine Similarity.

Instead of finding similarity with the direct one-hot word representation, 
we used GloVe Word Embeddings, which maps words into more meaningful space.

In General, finding Cosine Similarity on large dataset will take a huge amount 
of time on CPU. With the Xilinx Cosine Similarity Acceleration, it will speedup
the process by > ~80x.

This use case can be downloaded from `Wikipedia Search Engine Jupyter notebook on GitHub 
<https://github.com/Xilinx/graphanalytics/blob/master/cosinesim/examples/python/jupyter/wikipedia_demo.ipynb>`_.

Follow the setup process below before running the notebook:

.. toctree::
    :maxdepth: 1

    Setup <setup-standalone.rst>

Run following commands in the Python virtual environment (from setup) to run the Notebook:

.. code-block:: bash

    (fpga)$ cd cosinesim-examples/python
    (fpga)$ ./run.sh jupyter notebook jupyter/wikipedia_demo.ipynb
