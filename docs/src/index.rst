.. 
   Copyright 2021 Xilinx, Inc.
  
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
  
       http://www.apache.org/licenses/LICENSE-2.0
  
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

.. meta::
   :keywords: Vitis, Graph Database, Alveo
   :description: Vitis Graph Database Analytics Library is an open-sourced 
                 Vitis library written in C++ for accelerating database applications in a 
                 variety of use cases.
   :xlnxdocumentclass: Document
   :xlnxdocumenttype: Tutorials

Alveo-Accelerated GraphAnalytics
================================

We offer ready-to-use Alveo-Accelerated GraphAnalytics products for variety of business applications. You can use them on-prem or in cloud (AWS, Azure etc.). 
The accelerated products can be instaleld on top of your existing Tigergraph installaiton or can be used stand alone. Using the Avlveo-accelerated 
products can reduce your time-to-insight by as much as 1000x compared to your existing CPU only solutions. Additionally, they can lower requirements
for system memeory for your servers.


The products are released on `GitHub <https://github.com/Xilinx/graphanalytics>`_ 
under Apache License, Version 2.0


.. toctree::
   :caption: Tigergraph Applications
   :maxdepth: 1

    Recommnedation System <cosinesim/usecases.rst>
    Fraud Detection <louvainmod/usecases.rst>
    Supply Chain Optimization <mis/usecases.rst>
    Entity Resolution <fuzzymatch/usecases.rst>


.. toctree::
   :caption: Stand-Alone Applications
   :maxdepth: 1

   Cosine Similarity <cosinesim/index.rst>
   Louvain Modularity <louvainmod/index.rst>
   Maximal Independent Set <mis/index.rst>
   Fuzzy Match <fuzzymatch/index.rst>


.. toctree::
   :caption: Product Installation (TigerGraph 3.x)
   :maxdepth: 1

    Recommendation System  <recom-tg3/index.rst>
    Fraud Detection  <comdet-tg3/index.rst>
    Supply Chain Optimization  <mis-tg3/index.rst>
    Entity Resolution <fuzzymatch-tg3/index.rst>


.. toctree::
    :caption: Additional Info
    :maxdepth: 1

    vitis-dev-flow.rst
    ..tg3-plugins.rst
    

.. toctree::
   :caption: Notice
   :maxdepth: 1
   :hidden:

   notice.rst

   