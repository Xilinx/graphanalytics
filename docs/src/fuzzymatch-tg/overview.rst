Xilinx Fuzzy Match for TigerGraph 3.x Overview
==========================================================

An application built on TigerGraph consists of application software, TigerGraph GSQL
queries, and possibly also user-defined functions (UDFs).

The Xilinx Fuzzy Match for TigerGraph 3.x adds to the application 
with a plug-in module of UDFs that interface with the :ref:`fuzzymatch-label`.  
The Fuzzy Match Alveo Product, in turn, interfaces with the Alveo 
accelerator card to provide hardware acceleration of fuzzy match operations.

By writing queries that call the Fuzzy Match UDFs, your application 
can run fuzzy match operations using a Xilinx Alveo accelerator card.  
Please see :ref:`fuzzymatch-overview-label` and learn what fuzzy match 
is and how you can use it on an Alveo accelerator card to find maximal 
independent set of your graph quickly.

The following sections walk you through the installation of the Fuzzy Match 
plugin, the operation and composition of a demonstration application, and 
how to write your own TigerGraph-based application that uses the Fuzzy Match.
