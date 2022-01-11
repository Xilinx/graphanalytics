Maximal Independent Set C++ API Reference
===========================================

**Overview**
---------------
  
  | Maximal Independent Set provides MIS class which has interfaces to find MIS. 


**GraphCSR class**
----------------------
  | GraphCSR class constructor takes rowPtr vector and colIdx vector in CSR format. 
    User need to create GraphCSR graph and call  setGraph() before execution step.

    .. code-block:: bash

        // GraphCSR class constructor
        template <typename G>
        GraphCSR(G& rowPtr, G& colIdx)

**Options**
---------------

  | MIS constructor takes Options which configure the xclbin and device.
    
    .. code-block:: bash

     struct Options {
        XString xclbinPath;
        XString deviceNames;
     };

**startMis Interface**
------------------------------

  | Initialize and set up MIS software and Alveo cards.

   .. code-block:: bash

     void startMis();

**setGraph Interface**
------------------------------------------------------------

  | Set the graph and internal pre-process the graph
  | The argument is GraphCSR graph. 

   .. code-block:: bash

     void setGraph(GraphCSR<int>* graph);

**executeMIS Interface**
--------------------------------------------

  | Run Maximal Independent Set
  | Return vector of int

   .. code-block:: bash

     std::vector<int> executeMIS();

**count Interface**
--------------------------------------------

  | Return the number of MIS vertices 

  .. code-block:: bash

     size_t count() const;
     