..  note:: 
    
    If you only need to build applications utilizing Xilinx GraphAnalytics products, 
    you can skip the section below.

Collaborating on Fuzzy Match Product
------------------------------------
The Fuzzy Match product is an open-source project hosted on github, you can 
collaborate with Xilinx and contribute to the development of the product.

.. include:: ../common/clone-repo.rst

* Build and install the Fuzzy Match package. The example below shows 
  installation using the Ubuntu ``apt`` package manager on a Ubuntu 20.04 machine.

  .. code-block:: bash

    cd fuzzymatch
    make dist
    sudo apt install --reinstall ./package/xilinx-fuzzymatch-0.1_20.04-x86_64.deb
