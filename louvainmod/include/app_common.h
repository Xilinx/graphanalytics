/*
 * Copyright 2021 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _APP_COMMON_H_
#define _APP_COMMON_H_
struct ToolOptions {
    int argc;
    char** argv;  // strings not owned!
    
    double opts_C_thresh;   //; //Threshold with coloring on
    long opts_minGraphSize; //; //Min |V| to enable coloring
    double threshold;  //; //Value of threshold
    int opts_ftype;         //; //File type
    char opts_inFile[4096];  //;
    bool opts_coloring;     //
    bool opts_output;       //;
    std::string outputFile;
    bool opts_VF; //;
    std::string xclbinPath;
    std::string deviceNames;
    int numNodes;
    int nodeId;
    int numThreads;
    int numPars;
    int gh_par;  // same as par_prune
    int kernelMode;
    int numDevices;
    int modeZmq;
    char path_zmq[4096];
    bool useCmd;
    int mode_alveo;
    char nameProj[4096];  // used for create partitions
    std::string alveoProject; // used for load/compute TODO: consolidate with nameProj
    int numPureWorker;
    char *nameWorkers[128];
    int max_level;
    int max_iter;
    
    ToolOptions(int argc, char **argv);
};

enum {
	ZMQ_NONE=0,
	ZMQ_DRIVER=1,
	ZMQ_WORKER=2
};

#endif
