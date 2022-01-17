.. _fuzzymatch-overview-label:

Xilinx Fuzzy Match Alveo Product Overview
=========================================

Levenshtein Distances
---------------------

Levenshtein edit distance between two strings is the minimum number of single-character 
edits (insertions, deletions or substitutions) required to change one word into the other. 
Below is the mathematically definition:

.. image:: /images/fuzzymatch-levenshtein-formula.svg
   :alt: Levenshtein Edit Distance Formula

| where
* tail(x) is a string of all but the first character of x, and x[n] is the nth 
  character of the string x, starting with character 0.
* lev(tail(a), b) is deletion (from a to b).
* lev(a, tail(b) is insertion.
* lev(tail(a), tail(b)) replacement.

For example, the Levenshtein distance between "kitten" and "sitting" is 3, since 
the following three edits change one into the other, and there is no way to do it 
with fewer than three edits:

#. kitten → sitten (substitution of "s" for "k")
#. sitten → sittin (substitution of "i" for "e")
#. sittin → sitting (insertion of "g" at the end).

Similarity of Two Strings
-------------------------
The similarity of two strings is defined as below, where lev() is the Levenshtein 
edit distance of the strings:

.. image:: /images/fuzzymatch-similarity-formula.png
   :alt: Fuzzy Match Similarity Formula
   :width: 400

Two strings are considered as a match if the similarity score is greater than a 
user programmable threshold. 


