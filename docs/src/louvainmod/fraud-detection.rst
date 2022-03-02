.. _fraud-detection-label:

Fraud Detection
===============

An important step in fraud detection is to identify whether a group of people is 
part of a fraud ring which shows higher relationship density within the community.
Louvain Modularity is an effective algorithm for community detection, which 
provides important insight to detecting fraud. AMD fraud detection product based 
on accelerated Louvain Modularity on AMD Alveo accelerator cards provides 10x speedup
as well as reduces system memory requirement by 2/3 comparing to a CPU only solution.

The instructions below will help you get started quickly based on your persona:

.. toctree::
    :maxdepth: 1

    TigerGraph Users <../louvainmod/getting-started-tg.rst>
    C++/Python Users <../louvainmod/getting-started-standalone.rst>