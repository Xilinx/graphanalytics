/*
 * Copyright 2020 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#pragma once

#ifndef _XF_GRAPH_L3_OP_LOUVAINMODULARITY_HPP_
#define _XF_GRAPH_L3_OP_LOUVAINMODULARITY_HPP_

#include "graph.hpp"
#include "op_base.hpp"
#include "openclHandle.hpp"

#include "xilinxlouvainInternal.h"
#include <time.h>
#include "common.hpp"

namespace xf {
namespace graph {
namespace L3 {

class opLouvainModularity : public opBase {
   public:
    static uint32_t cuPerBoardLouvainModularity;

    static uint32_t dupNmLouvainModularity;

    std::thread louvainModularityThread;

    std::vector<event<int> > eventQueue;

    class clHandle* handles;

    KMemorys_host* buff_hosts;

    KMemorys_host_prune* buff_hosts_prune;

    opLouvainModularity() : opBase(){};

    void setHWInfo(uint32_t numDev, uint32_t CUmax);

    void freeLouvainModularity();

    void init(class openXRM* xrm, std::string kernelName, std::string kernelAlias,
              std::string xclbinFile, uint32_t* deviceIDs, uint32_t* cuIDs, 
              unsigned int requestLoad);

    void loadGraph(graphNew* G, int flowMode, bool opts_coloring, long opts_minGraphSize, double opts_C_thresh, int numThreads);

    static int compute(unsigned int deviceID,
                       unsigned int cuID,
                       unsigned int channelID,
                       xrmContext* ctx,
                       xrmCuResource* resR,
                       std::string instanceName,
                       clHandle* handles,
					   int flowMode,
                       GLV* pglv_iter,
                       double opts_C_thresh,
					   KMemorys_host* buff_host,
					   KMemorys_host_prune* buff_host_prune,
                       int* eachItrs,
                       double* currMod,
                       long*   numClusters,
                       double* eachTimeInitBuff,
                       double* eachTimeReadBuff);

    void demo_par_core(int id_dev,
    				   int flowMode,
                       GLV* pglv_orig,
                       GLV* pglv_iter,
                       bool opts_coloring,
                       long opts_minGraphSize,
                       double opts_threshold,
                       double opts_C_thresh,
                       int numThreads);

    event<int> addwork(GLV* glv, int flowMode,
                       double opts_C_thresh,
                       int* eachItrs,
                       double* currMod,
                       long*   numClusters,
                       double* eachTimeInitBuff,
                       double* eachTimeReadBuff);

   private:
    std::vector<int> deviceOffset;
    uint32_t numDevices_;
    uint32_t maxCU_;

    static void bufferInit(clHandle* hds, long NV, long NE_mem_1, long NE_mem_2, KMemorys_host* buff_host);

    static void UsingFPGA_MapHostClBuff_prune(
    		clHandle* 				hds,
    		long             		NV,
    		long             		NE_mem_1,
    		long             		NE_mem_2,
    		KMemorys_host_prune     *buff_hosts_prune);

    static int cuExecute(
        clHandle* hds, cl::Kernel& kernel0, unsigned int num_runs, std::vector<cl::Event>* evIn, cl::Event* evOut);

    static void migrateMemObj(clHandle* hds,
                              bool type,
                              unsigned int num_runs,
                              std::vector<cl::Memory>& ob,
                              std::vector<cl::Event>* evIn,
                              cl::Event* evOut);

    static void cuRelease(xrmContext* ctx, xrmCuResource* resR);

    static void postProcess();

    static void PhaseLoop_UsingFPGA_1_KernelSetup(bool isLargeEdge,
                                                  cl::Kernel& kernel_louvain,
                                                  std::vector<cl::Memory>& ob_in,
                                                  std::vector<cl::Memory>& ob_out,
                                                  clHandle* hds);
    static void PhaseLoop_UsingFPGA_1_KernelSetup_prune(bool isLargeEdge,
                                                  cl::Kernel& kernel_louvain,
                                                  std::vector<cl::Memory>& ob_in,
                                                  std::vector<cl::Memory>& ob_out,
                                                  clHandle* hds);
};
} // L3
} // graph
} // xf

#endif
