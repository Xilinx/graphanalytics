Fuzzy Match Python API Reference
===========================================
**Overview**
---------------
  
  | FuzzyMatch provides FuzzyMatch class which has interfaces to enable fuzzy match in batch mode. 
  | Maximum length of blacklist/query string: 64

**startFuzzyMatch Interface**
------------------------------

 | Initialize and set up FuzzyMatch software and Alveo cards.

 .. code-block:: bash

    startFuzzyMatch()

**fuzzyMatchLoadVec Interface**
------------------------------------------------------------

 | Load and preprocess the input table. 
 | Parameters:  
 
    | patternList :  list of strings
    | patternId   : list of Int.  the Id for each string in patternList. if null, pass empty list. if pass empty list, internally will be assigned sequential id as default;
 
 | Return: status: integer.  0: successful; 1: fail
   
   .. code-block:: bash
   
      fuzzyMatchLoadVec(patternList,patternId)

**executefuzzyMatch Interface**
--------------------------------------------

  | Run fuzzymatch in batch mode. Check each input string in inputStr list has fuzzymatch against the pattern strings loaded using fuzzyMatchLoadVec interface.
  
  | Parameters: 

    | inputStr        : list of input strings.
    | similarityLevel : similarity threshold.  value range [0-100]. 100 means exact same.  
 
  | Return: 2D array of pairs. Each row represents array of matched result pairs {id,score}. 
        Maximum top 100 of pairs for each input string result.

   .. code-block:: bash
      
      executefuzzyMatch(inputStr,similarityLevel)
