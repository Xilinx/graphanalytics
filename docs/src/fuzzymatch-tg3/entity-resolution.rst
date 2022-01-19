.. _fuzzymatch-usecases-entity-resolution-label:

Entity Resolution
=================

Entity resolution is an operational intelligence process of identifying records
corresponding to the same real-world entities across multiple data sources. 
The challenge in entity resolution is that records from different data sources 
are not exactly the same and they do not share a unique identifier. 

One popular technique used for entity resolution is to calcualte the Levenshtein
edit distance between two strings. This use case shows how a TigerGraph-based 
application can leverage Xilinx FuzzyMatch plugin to achieve order of magnitude 
speedup in entity resolution using Levenshtein edit distance technique.

.. toctree::
    :maxdepth: 1

    Overview <entity-resolution-overview.rst>
    Run Bash Script with GSQL <entity-resolution-gsql.rst>
