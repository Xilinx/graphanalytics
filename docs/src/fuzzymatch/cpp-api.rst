Fuzzy Match C++ API Reference
===========================================

**Overview**
---------------
  
  | FuzzyMatch provides FuzzyMatch class which has interfaces to enable fuzzy match in batch mode. 
  | Maximum length of blacklist/query string: 64

**Options**
---------------

  | FuzyMatch constructor takes Options which configure the xclbin and device.
    
    .. code-block:: bash

     struct Options {
        XString xclbinPath;
        XString deviceNames;
     };

**startFuzzyMatch Interface**
------------------------------

  | Initialize and set up FuzzyMatch software and Alveo cards.
  | Return status: 0: successful; 1: fail.

   .. code-block:: bash

     int startFuzzyMatch()

**fuzzyMatchLoadVec Interface**
------------------------------------------------------------

  | Load and preprocess the input table. 
  | The argument is vector of  pattern strings (e.g. black list strings).
  | Return status; 0: successful; 1: fail.

   .. code-block:: bash

     int fuzzyMatchLoadVec(std::vector<std::string>& patternVec)

**executefuzzyMatch Interface**
--------------------------------------------

  | Run fuzzymatch in batch mode. 
  | The arguments take the vector of input strings. For each input string, run fuzzymatch against the pattern strings loaded using fuzzyMatchLoadVec interface.
  | Return vector of  matched patterns in pairs format {id,score} for each input string. 
    For each input string , return maximum top 100 of matched patterns.

   .. code-block:: bash

     std::vector<std::vector<std::pair<int,int>>> executefuzzyMatch(std::vector<std::string> input_patterns, int similarity_level);
