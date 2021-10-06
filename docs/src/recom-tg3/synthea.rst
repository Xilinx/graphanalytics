.. _recomengine-synthea-label:

Recommendation Engine Synthea Demo
==================================

The Synthea Demo is a demonstration medical application to show how a TigerGraph-based application might leverage
Xilinx Recommendation Engine for TigerGraph 3.x.

Health and medical professionals are required to make predictions for diagnosis and prognosis for various medical
problems. Individualized predictive analytics based on patient similarity is becoming popular in this field to
identify similar patients to an index patient, derive insights from the records of the similar patients,
and provide personalized predictions. Studies have shown that cosine similarity metrics can outperform
conventional predictive modeling in which all available patient data are analyzed. The improved prediction
performance is achieved at the cost of increased computational burden. 

Xilinx Recommendation Engine for TigerGraph 3.x, using
`Alveo Adaptable Accelerator Cards <https://www.xilinx.com/products/boards-and-kits/alveo.html>`_ ,
has been demonstrated on `Synthea <https://synthetichealth.github.io/synthea/>`_ generated 
patient data to greatly reduce that computational burden and provide predication results 
for patients more rapidly. Synthea is an open-source, synthetic patient 
generator that models the medical history of synthetic patients and provides high-quality, 
synthetic, realistic but not real, patient data and associated health records covering 
every aspect of healthcare. 


.. toctree::
    :maxdepth: 1

    Overview <synthea-overview.rst>
    recomengine-example-script.rst
    Demo Query Reference <synthea-query.rst>
..    Installing and Uninstalling the Demo <synthea-install.rst>
..    recomengine-example-jupyter.rst


Run Jupyter Notebook
------------------------

* Install the Synthea Demo Plugin

.. code-block:: bash

    (fpga)$ cd synthea
    (fpga)$ su - tigergraph
    Password:
    (fpga)$ bin/install_udf.sh


* Run the command below to start Jupyter Notebook

.. code-block:: bash

    (fpga)$ cd jupyter-demo
    (fpga)$ jupyter notebook TG_demo.ipynb

* Follow the step-by-step instructions in the notebook once it is loaded in your browser.

The Jupter Notebook demo is also available on
`Github <https://github.com/Xilinx/graphanalytics/blob/master/plugin/tigergraph/examples/synthea/jupyter-demo/TG_demo.ipynb>`_

Watch `Recommendation Engine Jupyter Notebook Demo Webinar
<https://www.xilinx.com/video/application/recommendation-engine-accelerated-tigergraph-webinar.html>`_
to learn more about this demo.