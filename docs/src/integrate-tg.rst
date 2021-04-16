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
 Graph Library, as shown below in *graph/plugin* directory.  Important files/directories are:

* install.sh*: is a script to automate the porting. It updates the *ExprFunctions.hpp* 
  with the top level functions of Graph Library defined in *tigergraph/QueryUdf/xilinxUdf.hpp*
* tigergraph/QueryUdf*: Once all the files in this directory is updated, it replaces 
  the *QueryUdf* directory in TigerGraph install directory 
  (*~tigergraph/dev/gdk/gsql/src/QueryUdf*)
* tigergraph/tests/cosine_nbor_ss_dense_int*: has all the GSQL scripts that you 
  can use to test the newely created UDFs
* tigergraph/tests/cosine_nbor_ss_dense_int/1K_patiensts*: has all the patient
  data set used for Cosine Similarity query testing

  .. image:: /images/fig_plugin.jpg
   :scale: 60%
   :align: center


UDF Example
-----------------------------------------------------------
This section shows how a custom GSQL function is created using 
*udf_loadgraph_cosinesim_ss_fpga()* as an example. The table below should help 
you navigate the source files used in creating this UDF. They are listed in the 
order of abstraction level - high to low level.

* udf_loadgrpah_cosinesim_ss_fpga() : this UDF is called in *base.gsql* script (in plugin/tigergraph/tests/cosine_nbor_ss_dense_int)
* udf_loadgrpah_cosinesim_ss_fpga() : defined in xilinxUdf.hpp (in plugin/tigergraph/QueryUdf) 
  and copied to ExprFunctions.hpp in TigerGraph install directory by install.sh. 
  It calls the function below
* loadgrpah_cosinesim_ss_dense_fpga() : defined in core.cpp and its prototype 
  is in loader.hpp (plugin/tigergraph/QueryUdf). This function opens 
  libgraphL3wrap.so library, which is built from L3_wrapp.cpp (in plugin/tigergraph), 
  and loads *loadgraph_cosine_ss_dense_fpga()*
* config_cosinesim_ss_dense_fpga.json : *loadgraph_cosinesim_ss_dense_fpga* in 
  L3_wrapp.cpp calls this json file which specifies the location/name of xclbin 
  file, among other settings. It also calls *xf::graph::L3::Handle* defined in 
  xf_graph_L3_handle.hpp (in graph/L3/include) used for XRM


Next Step
-----------------------------------------------------------

If you want to create your own accelerator function to run on Alveo, please see
the the next section.

