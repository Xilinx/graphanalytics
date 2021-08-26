// ***********************************************************************
//
//            Grappolo: A C++ library for graph clustering
//               Mahantesh Halappanavar (hala@pnnl.gov)
//               Pacific Northwest National Laboratory
//
// ***********************************************************************
//
//       Copyright (2014) Battelle Memorial Institute
//                      All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// ************************************************************************

#include "defs.h"
#include "xilinxlouvain.h"

int host_ParserParameters(int argc,
                          char** argv,
                          double& opts_C_thresh,   //; //Threshold with coloring on
                          long& opts_minGraphSize, //; //Min |V| to enable coloring
                          double& opts_threshold,  //; //Value of threshold
                          int& opts_ftype,         //; //File type
                          char opts_inFile[4096],  //;
                          bool& opts_coloring,     //
                          bool& opts_output,       //;
                          bool& opts_VF,           //;
                          char opts_xclbinPath[4096]) {
    // step1: parser parameters: xclbinPath, coloring, int fType ,
    // opts.minGraphSize, opts.threshold
    // step2: prepare graphNew: parser files and undirection
    // Parse Input parameters:
    clustering_parameters opts;
    if (!opts.parse(argc, argv)) {
        return -1;
    }
    opts_C_thresh = opts.C_thresh;
    opts_minGraphSize = opts.minGraphSize;
    opts_threshold = opts.threshold;
    opts_ftype = opts.ftype; // File type
    opts_output = opts.output;
    opts_coloring = opts.coloring;
    opts_VF = opts.VF;
    strcpy(opts_inFile, (char*)opts.inFile);
    strcpy(opts_xclbinPath, (char*)opts.xclbin);
    return 0;
}

graphNew* host_PrepareGraph(int opts_ftype, char opts_inFile[4096], bool opts_VF) {
    graphNew* G = (graphNew*)malloc(sizeof(graphNew));
    if (opts_ftype == 1)
        parse_MatrixMarket_Sym_AsGraph(G, opts_inFile);
    else if (opts_ftype == 2)
        parse_Dimacs9FormatDirectedNewD(G, opts_inFile);
    else if (opts_ftype == 3)
        parse_PajekFormat(G, opts_inFile);
    else if (opts_ftype == 4)
        parse_PajekFormatUndirected(G, opts_inFile);
    else if (opts_ftype == 5)
        loadMetisFileFormat(G, opts_inFile);
    else if (opts_ftype == 6)
        parse_DoulbedEdgeList(G, opts_inFile);
    else if (opts_ftype == 7)
        parse_EdgeListBinary(G, opts_inFile);
    else
        parse_SNAP(G, opts_inFile);
    //displayGraphCharacteristics(G);
    /* Vertex Following option */
    if (opts_VF) {
        printf("Vertex following is enabled.\n");
        long numVtxToFix = 0; // Default zero
        long* C = (long*)malloc(G->numVertices * sizeof(long));
        assert(C != 0);
        numVtxToFix = vertexFollowing(G, C); // Find vertices that follow other vertices
        if (numVtxToFix > 0) {               // Need to fix things: build a new graphNew
            printf("Graph will be modified -- %ld vertices need to be fixed.\n", numVtxToFix);
            graphNew* Gnew = (graphNew*)malloc(sizeof(graphNew));
            long numClusters = renumberClustersContiguously(C, G->numVertices);
            buildNewGraphVF(G, Gnew, C, numClusters);
            // Get rid of the old graphNew and store the new graphNew
            free(G->edgeListPtrs);
            free(G->edgeList);
            free(G);
            G = Gnew;
        }
        free(C); // Free up memory
        printf("Graph after modifications:\n");
        displayGraphCharacteristics(G);
    } // End of if( VF == 1 )
    return G;
}

