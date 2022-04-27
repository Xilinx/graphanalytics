Xilinx Community Detection for TigerGraph 3.x Overview
========================================================

Application built on TigerGraph consists of application software, TigerGraph GSQL
queries, and possibly also user-defined functions (UDFs).

The Xilinx Community Detection for TigerGraph 3.x adds to the application with 
a plug-in module of UDFs that interface with the :ref:`louvainmod-label`.  The 
Louvain Modularity Alveo Product, in turn, interfaces with the Alveo accelerator 
card to provide hardware acceleration of Louvain modularity computation,
as shown in the diagram below.

.. image:: /images/comdetect-stack.png
   :alt: Community Detection Stack
   :align: center

By writing queries that call the Community Detection UDFs, your application can 
accelerate Louvain modularity computation using a Xilinx Alveo accelerator card.  
Please see :ref:`louvainmod-overview-label` for an overview of
what Louvain modularity is and how you can use Louvain modularity on an Alveo 
accelerator card to extract communities from your graph quickly.

The following sections walk you through the installation of the Community Detection, 
the operation and composition of a demonstration application, and how to write your 
own TigerGraph-based application that uses the Community Detection product.
