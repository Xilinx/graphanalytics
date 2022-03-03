Xilinx Maximal Independent Set for TigerGraph 3.x Overview
==========================================================

An application built on TigerGraph consists of application software, TigerGraph GSQL
queries, and possibly also user-defined functions (UDFs).

The Xilinx Maximal Independent Set for TigerGraph 3.x adds to the application 
with a plug-in module of UDFs that interface with the :ref:`cosinesim-label`.  
The Maximal Independent Set Alveo Product, in turn, interfaces with the Alveo 
accelerator card to provide hardware acceleration of maximal independent set 
operations.

By writing queries that call the Maximal Independent Set UDFs, your application 
can run maximal Independent set operations using a Xilinx Alveo accelerator card.  
Please see :ref:`mis-overview-label` for an overview of what maximal independent 
set is and how you can use it on an Alveo accelerator card to find maximal 
independent set of your graph quickly.

The following sections walk you through the installation of the Maximal Independent 
Set MIS plugin, the operation and composition of a demonstration application, and 
how to write your own TigerGraph-based application that uses the Maximal Independent 
Set.
