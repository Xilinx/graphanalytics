=====================================
Integration with TigerGraph 
=====================================

This section explains how L3 Vitis Graph Library function is 
ported into TigerGraph SW framework, and finally running the application on Alveo by 
calling GSQL query from TigerGraph SW.

.. image:: /images/fig_TG_top.jpg
   :scale: 60%
   :align: center


TigerGraph User-Defined Functions (UDF)
---------------------------------------

You can create your own custom GSQL function as `UDF <https://docs.tigergraph.com/dev/gsql-ref/querying/operators-functions-and-expressions#user-defined-functions>`_ 
in TigerGraph using C++. You can define a top level function in 
*<tigergraph.root.dir>/dev/gdk/gsql/src/QueryUdf/ExprFunctions.hpp*. 
It is this UDF mechanism that enables porting of Xilinx Graph Library IP into 
TigerGraph SW. All the files necessary to make this porting is included in the
Graph Library, as shown below in *plugin/tigergraph* directory. Important 
files/directories are:

* *install.sh*: is a script to automate the porting. It updates the ExprFunctions.hpp
  with the top level functions of Graph Library defined in *udf/xilinxRecomEngine.hpp.hpp*
* *plugin/tigergraph/examples/synthea*: has all supporting files (shell scripts, 
  GSQL scripts, UDF) that you can use to test the newely created UDFs
* *plugin/tigergraph/examples/synthea/1000_patients/csv*: has all the patient data 
  set used for Cosine Similarity query testing


Next Step
-----------------------------------------------------------

If you want to create your own accelerator function to run on Alveo, please see
the the next section.

