=============================
Fuzzy Match C++ API Reference
=============================

**Overview**
---------------
  
FuzzyMatch provides FuzzyMatch class which has interfaces to enable fuzzy match in batch mode. 
Maximum length of blacklist/query string: 64

**Options**
---------------

FuzyMatch constructor takes Options which configure the xclbin and device.
    
.. code-block:: bash

  struct Options {
      XString xclbinPath;
      XString deviceNames;
  };

**startFuzzyMatch Interface**
------------------------------

Initialize and set up FuzzyMatch software and Alveo cards.
  
Return:
    status: 0: successful; 1: fail.

.. code-block:: bash

  int startFuzzyMatch()

**fuzzyMatchLoadVec Interface**
------------------------------------------------------------

Load and preprocess the input table. 

Parameters:
    patternVec : vector of  pattern strings (e.g. black list strings).
 
Return:
    status: 0: successful; 1: fail.

.. code-block:: bash

   int fuzzyMatchLoadVec(std::vector<std::string>& patternVec)

**executefuzzyMatch Interface**
--------------------------------------------

Run fuzzymatch in batch mode. Match each input string in input_patterns vector 
against the pattern strings loaded using fuzzyMatchLoadVec interface.

.. code-block:: bash

   std::vector<std::vector<std::pair<int,int>>> executefuzzyMatch(
       std::vector<std::string> input_patterns, 
       int similarity_level);

  
Parameters:
  input_patterns    :   vector of input strings. For each input string in input_patterns, run fuzzymatch against the pattern strings loaded using fuzzyMatchLoadVec interface.
  similarity_level  :   similarity threshold.  value range [0-100]. 100 means exact same.
  
Return:
    2D vectors of pairs. Each row represents array of matched result pairs {id,score}. 
    Maximum top 100 of pairs for each input string result.
   

