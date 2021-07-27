Xilinx Recommendation Engine for TigerGraph 3.x Overview
========================================================

As shown in the diagram below, an application built on TigerGraph consists of application software, TigerGraph GSQL
queries, and possibly also user-defined functions (UDFs).

<insert application schematic>

The Xilinx Recommendation Engine for TigerGraph 3.x adds to the application with a plug-in module of UDFs that
interface with the :ref:`cosinesim-label`.  The Cosine Similarity Alveo Product, in turn, interfaces with
the Alveo accelerator card to provide hardware acceleration of cosine similarity operations,
as shown in the diagram below.

<insert application schematic with Recom Engine>

By writing queries that call the Recommendation Engine UDFs, your application can accelerate cosine similarity
operations using a Xilinx Alveo accelerator card.  Please see :ref:`cosinesim-overview-label` for an overview of
what cosine similarity is and how you can use cosine similarity on an Alveo accelerator card to find patterns
in your data quickly.

The following sections walk you through the installation of the Recommendation Engine, the operation and composition
of a demonstration application, and how to write your own TigerGraph-based application that uses the Recommendation
Engine.
