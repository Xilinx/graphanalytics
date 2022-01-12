.. _mis-overview-label:

=====================================================
Xilinx Maximal Independent Set Alveo Product Overview
=====================================================

Maximum and Maximal Independent Sets
------------------------------------
An independent set of a graph is a subset of vertices in which no two vertices 
are adjacent. Given a set of vertices, the maximum independent set problem (MIS) 
calls for finding the independent set of maximum cardinality. Finding such a Maximum set does
not have a polynomial time solution and is NP-Hard. For most practical purposes, large cardinality
maximal independent sets are sufficient to accomplish required tasks. This library uses a modified version of
`Luby's algorithm <http://www.cs.cmu.edu/afs/cs/academic/class/15750-s18/ScribeNotes/lecture32.pdf>`_
to find high quality Maximal Independent Sets using Xilinx Alveo accelerator cards.

There are many applications that require finding the MIS, including but not limited to the following:

* Massive data sets: military systems, telecommunications, medicine, finance, etc.
* Coding theory: formulating error correction codes
* Ad-hoc wireless networks: search and rescue operations, battlefield decisions, addressing
  battery and security issues, etc.

Algorithm Overview
------------------
Xilinx Maximal Independent Set Alveo Product is built on the random-permutation variation of Luby's MIS
algorithm. The specific variation used was proposed by MARTIN BURTSCHER et al. from the Texas State University.

The original paper is available `here <https://userweb.cs.txstate.edu/~mb92/papers/topc18.pdf>`_

The algorithm can be summarized as follows:

1. Let V be the set of all vertices
2. Initialize a set I as an empty set
3. Generate a random priority number p(v) for each vertex v in V
4. While V is not empty:

 4.1 If p(v) is larger than the priority numbers of all neighbors of v, then remove v from V and insert into I

 4.2 If any neighbors of v was inserted into I, then remove v from V

5. Return I

