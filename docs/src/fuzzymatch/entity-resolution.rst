.. _entity-resolution-label:

=================
Entity Resolution
=================

Entity resolution is an operational intelligence process of identifying records
corresponding to the same real-world entities across multiple data sources. 
The challenge in entity resolution is that records from different data sources 
are not exactly the same and they do not share a unique identifier. 

Entity resolution generally involves several algorithms such as 
`Levenshtein edit distance <https://en.wikipedia.org/wiki/Levenshtein_distance>`_  and 
`Soundex <https://en.wikipedia.org/wiki/Soundex>`_ working together. AMD Entity Resolution product 
accelerates Levenshtein edit distance computation and achieves order of magnitude speed-up.

The instructions below will help you get started quickly based on your persona:

.. toctree::
    :maxdepth: 1

    TigerGraph Users <../fuzzymatch/getting-started-tg.rst>
    C++/Python Users <../fuzzymatch/getting-started-standalone.rst>
