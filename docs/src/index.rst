.. 
   Copyright 2021-2022 Xilinx, Inc.
  
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

We offer ready-to-use Alveo-Accelerated GraphAnalytics products for variety of 
business applications. You can use them on-prem or in cloud (AWS, Azure etc.). 
The accelerated products can be installed on top of your existing Tigergraph 
installation or can be used standalone. Using the Alveo-accelerated 
products can reduce your time-to-insight by as much as 100x compared to your 
existing CPU only solutions. Additionally, they can lower requirements
for server system memory as much as 3x.


The binary installation package is available on `Database Analytics POC Secure Site <%PACKAGE_LINK>`_. 
Additionally for advanced users wanting to make code changes, the source code is available at 
`https://github.com/Xilinx/graphanalytics <https://github.com/Xilinx/graphanalytics>`_ under 
Apache License, Version 2.0.


.. toctree::
   :caption: Applications
   :maxdepth: 1

    Recommendation System <cosinesim/usecases.rst>
    Fraud Detection <louvainmod/fraud-detection.rst>
    Supply Chain Optimization <mis/supply-chain.rst>
    Entity Resolution <fuzzymatch/entity-resolution.rst>



.. toctree::
   :caption: Algorithms
   :maxdepth: 1

   Cosine Similarity <cosinesim/index.rst>
   Louvain Modularity <louvainmod/index.rst>
   Maximal Independent Set <mis/index.rst>
   Levenstein Edit Distance (Fuzzy Match) <fuzzymatch/index.rst>

.. toctree::
   :caption: Notice
   :maxdepth: 1
   :hidden:

   notice.rst

   