
## Integration with TigerGraph 
If you have gone thru *Targeting Alveo in HPE server* section, you have installed all the necessary software to start integration of Xilinx Graph Library IP with Tigergraph software. This section explains how L3 Vitis Graph Library is ported into TigerGraph SW framework, and finally running the application on Alveo by calling GSQL query from TigerGraph SW.

<p align="center">
<img src="images2/fig_TG_top.jpg", width "300">
</p>

### TigerGraph [User-Defined Functions](https://docs.tigergraph.com/dev/gsql-ref/querying/operators-functions-and-expressions#user-defined-functions)(UDF)

Users can define their own custom functions in C++ in *<tigergraph.root.dir>/dev/gdk/gsql/src/QueryUdf/ExprFunctions.hpp*. It is this UDF mechanism that enables porting of Graph Library IP into TigerGraph SW. All the files necessary to make this happen is in the Graph Library, as shown below in *graph/plugin* directory.  Some of the important files/directories are:

- *install.sh*: is the script to automate the porting. It updates the *ExprFunctions.hpp* with the top level functions of Graph Library defined in *tigergraph/QueryUdf/xilinxUdf.hpp*
- *tigergraph/QueryUdf*: Once all the files in this directory is updated, it replaces the *QueryUdf* directory in TigerGraph install directory (*~tigergraph/dev/gdk/gsql/src/QueryUdf*)
- *tigergraph/tests/cosine_nbor_ss_dense_int*: has all the GSQL scripts that you can use to test the newely created UDFs


<p align="center">
<img src="images2/fig_plugin.jpg", width "200">
</p>


### UDF Example
This section shows how *udf_loadgraph_cosinesim_ss_fpga()* GSQL function is created. The table below should help you navigate the source files used in creating this UDF.


| function name | note |
| ------------------------------- | ------------------------------- |
| udf_loadgrpah_cosinesim_ss_fpga() | this UDF is called in *base.gsql* script (plugin/tigergraph/tests/cosine_nbor_ss_dense_int) |
| udf_loadgrpah_cosinesim_ss_fpga() | defined in xilinxUdf.hpp (plugin/tigergraph/QueryUdf) and copied to ExprFunctions.hpp in TigerGraph install directory by install.sh. It calls the function below |
| loadgrpah_cosinesim_ss_dense_fpga() | defined in core.cpp and its prototype is in loader.hpp (plugin/tigergraph/QueryUdf). This function opens libgraphL3wrap.so library, which is built from L3_wrapp.cpp (in plugin/tigergraph), and loads *loadgraph_cosine_ss_dense_fpga()* |
| config_cosinesim_ss_dense_fpga.json | *loadgraph_cosinesim_ss_dense_fpga* in L3_wrapp.cpp calls this json file which specifies the location/name of xclbin file, among other settings. It also calls *xf::graph::L3::Handle* defined in xf_graph_L3_handle.hpp (in graph/L3/include) used for XRM  |


### Running *install.sh*
Follow the steps below to transform the Graph Library functions into custom GSQL functions running on Alveo card in TigerGraph environment:


```
1) Integrate Xilinx graph library to TigerGraph framework as UDF 
su - tigergraph
cd <vitis lib dir>/graph/plugin  
./install.sh

2) check to make sure Alveo U50 is alive and working
xbutil query

3) Install xclbin file from TBD. You must be a tigergraph user. xclbin file programs FPGA with Graph Library
./install-xclbin.sh

4) start XRM (Xilinx Resource Manager) daemon in a separate terminal  
sudo /opt/xilinx/xrm/tools/restart_xrmd.sh

5) run cosine similarity poc  
cd <vitis lib dir>/graph/plugin/tigergraph/tests/cosine_nbor_ss_dense_int
./run.sh
```

### Benchmark Results
Update this section with the latest results.

