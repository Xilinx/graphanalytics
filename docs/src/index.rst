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

Vitis Graph Database Analytics Library
======================================
Vitis Graph Database Analytics Library is an open-sourced Vitis library written 
in C++ for accelerating database applications in a variety of use cases. The library 
includes stand-alone Alveo products as well as products integrated with third-party
software such as TigerGraph. 

The library is released on `GitHub <https://github.com/Xilinx/graphanalytics>`_ 
under Apache License, Version 2.0


.. toctree::
   :caption: Stand-Alone Alveo Products
   :maxdepth: 1

   Cosine Similarity <cosinesim/index.rst>
   Louvain Modularity <louvainmod/index.rst>
   Maximum Independent Set <mis/index.rst>
   Fuzzy Match <fuzzymatch/index.rst>


.. toctree::
   :caption: Software Stack Integration Products
   :maxdepth: 1

    Recommendation Engine for TigerGraph 3.x <recom-tg3/index.rst>
    Community Detection for TigerGraph 3.x <comdet-tg3/index.rst>
    Maximal Independent Set for TigerGraph 3.x <mis-tg3/index.rst>
    Fuzzy Match for TigerGraph 3.x <fuzzymatch-tg3/index.rst>

.. toctree::
   :caption: Use Cases
   :maxdepth: 1

    Cosine Similarity Use Cases <cosinesim/usecases.rst>
    Louvain Modularity Use Cases <louvainmod/usecases.rst>

.. toctree::
    :caption: Additional Info
    :maxdepth: 1

    tg3-plugins.rst
    vitis-dev-flow.rst

.. toctree::
   :caption: Notice
   :maxdepth: 1
   :hidden:

   notice.rst

   