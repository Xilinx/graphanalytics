/*
 * Copyright 2020-2021 Xilinx, Inc.
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

#ifndef XILINX_COM_DETECT_HPP
#define XILINX_COM_DETECT_HPP

// mergeHeaders 1 name xilinxComDetect
#define XILINX_COM_DETECT_DEBUG_ON
// mergeHeaders 1 section include start xilinxComDetect DO NOT REMOVE!
#include "xilinxComDetectImpl.hpp"
#include <cstdint>
#include <vector>
// mergeHeaders 1 section include end xilinxComDetect DO NOT REMOVE!

namespace UDIMPL {

// mergeHeaders 1 section body start xilinxComDetect DO NOT REMOVE!
inline void udf_reset_nextId() {

    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    std::cout << "nextId_ = " << pContext->getNextId() <<std::endl;
    pContext->clearPartitionData();
}

inline uint64_t udf_get_nextId(uint64_t out_degree){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    //pContext->addDegreeList((long)out_degree);
    pContext->degree_list.push_back(out_degree);
#ifdef  XILINX_COM_DETECT_DUMP_GRAPH
    std::cout << " louvainId = " << pContext->getNextId() <<" out_degree = " << out_degree <<std::endl;
#endif
    uint64_t nextId = pContext->getNextId();
    uint64_t nextIdIncr = nextId +1 ;
    pContext->setNextId(nextIdIncr);
    return nextId;
}

inline uint64_t udf_get_partition_size(){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
     xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
#ifdef XILINX_COM_DETECT_DEBUG_ON
     std::cout << "Partition Size = " << pContext->getNextId()<<std::endl;
#endif
     return pContext->getNextId();
}
inline int udf_xilinx_comdetect_set_node_id(uint nodeId)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    std::cout << "DEBUG: " << __FUNCTION__ << " nodeId=" << nodeId << std::endl;
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();

    pContext->setNodeId(unsigned(nodeId));
    return nodeId;
}
//add new
inline uint64_t udf_get_global_louvain_id(uint64_t louvain_id){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    return pContext->getLouvainOffset() + louvain_id ;
}

inline void udf_set_louvain_offset(uint64_t louvain_offset){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
#ifdef XILINX_COM_DETECT_DUMP_GRAPH
    std::cout << "Louvain Offsets = " << louvain_offset <<std::endl;
#endif
    pContext->setLouvainOffset(louvain_offset);
}



inline void udf_start_partition(std::string alveo_project, int numVertices){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    std::cout << "DEBUG: before alveo_parj_path" << std::endl;
    pContext->setAlveoProject(alveo_project);
    xilinx_apps::louvainmod::LouvainMod *pLouvainMod = pContext->getLouvainModObj();
    xilinx_apps::louvainmod::LouvainMod::PartitionOptions options; //use default value
    options.totalNumVertices = numVertices;
#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "DEBUG: totalNumVertices: " << numVertices << std::endl;
#endif
    pLouvainMod->startPartitioning(options);
}



//Data has been populated and send to FPGA
inline void udf_save_EdgePtr( ){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
     xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
     //build offsets_tg

     pContext->mEdgePtrVec.push_back(0);
     for(int i = 1;i <= pContext->degree_list.size();i++){
         pContext->mEdgePtrVec.push_back(pContext->degree_list[i-1] + pContext->mEdgePtrVec[i-1]); //offsets_tg[i] += pContext->offsets_tg[i-1];
       }

     pContext->setOffsetsTg(pContext->mEdgePtrVec.data());
     int size= pContext->mEdgePtrVec.size();
     pContext->mEdgeVec.resize(pContext->mEdgePtrVec[size-1]); //NE
     pContext->mDgrVec.resize(pContext->mEdgePtrVec[size-1]); //NE
     pContext->addedOffset.resize(pContext->mEdgePtrVec.size()); //NV+1
     std::fill(pContext->addedOffset.begin(), pContext->addedOffset.end(),0);
     //std::cout<< "DEBUG:: mDgrVec SIZE:" << pContext->mDgrVec.size() <<" ;"<< pContext->mEdgePtrVec[size-1] << std::endl;

}

inline void udf_set_louvain_edge_list( uint64_t louvainIdSource, uint64_t louvainIdTarget, float wtAttr, uint64_t outDgr){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    int idx = louvainIdSource - pContext->getLouvainOffset();
    int startingOffset = pContext->mEdgePtrVec[idx];
    // set atomic int and increase
    pContext->mEdgeVec[startingOffset +  pContext->addedOffset[idx]] = xilinx_apps::louvainmod::Edge((long)louvainIdSource,(long)louvainIdTarget, (double)wtAttr);
   /*
    if(louvainIdSource < 10) {
        std::cout << "DEBUG:: louvainIdSource : " << louvainIdSource<< "; pContext->getLouvainOffset():" <<  pContext->getLouvainOffset() << std::endl;
        std::cout << "DEBUG:: idx: " << pContext->idx << std::endl;
        std::cout << "DEBUG:: starting offset: "  << pContext->mEdgePtrVec[pContext->idx]<< std::endl;


        //std::cout << "DEBUG:: startingOffset: " << pContext->startingOffset << "; idx : " << pContext->idx << std::endl;
    }*/
    pContext->mDgrVec[startingOffset+ pContext->addedOffset[idx]] = outDgr;
    pContext->addedOffset[idx]++;
}

inline int udf_save_alveo_partition(uint numPar) {
    std::cout << "INFO: " << __FUNCTION__ << " numPar=" << numPar << std::endl;

    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    //build offsets_tg
    
    //build dgr list and edgelist

    //traverse the partition size for each louvainId, populate edgelist from edgeListMap
    pContext->setStartVertex(long(pContext->getLouvainOffset()));
    pContext->setEndVertex(long(pContext->getLouvainOffset() + pContext->getNextId())) ; // the end vertex on local partition
    pContext->setDrglistTg(pContext->mDgrVec.data());
    pContext->setEdgelistTg(pContext->mEdgeVec.data());

#ifdef XILINX_COM_DETECT_DEBUG_ON
    std::cout << "edgelist size:" << pContext->mEdgeVec.size() << std::endl;
    std::cout << "dgrlist size:" << pContext->mDgrVec.size() << std::endl;
    for(int i=0; i < 10; i++) {
        std::cout << i << " head from edgelist  " << pContext->getEdgelistTg()[i].head <<"; ";
        std::cout << " tail from edgelist: " << pContext->getEdgelistTg()[i].tail <<"; ";
        std::cout << " outDgr from dgrlist: " << pContext->getDrglistTg()[i] << std::endl;
    }
    std::cout << "start_vertex: " << pContext->getStartVertex()<<std::endl;
    std::cout << "end_vertex: " << pContext->getEndVertex() <<std::endl;
#endif

    long NV_par_recommand;
    long NV_par_max = 64*1000*1000;
    if (numPar>1)
        NV_par_recommand = ( pContext->getNextId()+ numPar-1) / numPar;//allow to partition small graph with -par_num
    else
        NV_par_recommand = (long)((float)NV_par_max * 0.80);//20% space for ghosts.

    xilinx_apps::louvainmod::LouvainMod *pLouvainMod = pContext->getLouvainModObj();
    xilinx_apps::louvainmod::LouvainMod::PartitionData partitionData;
    partitionData.offsets_tg = pContext->getOffsetsTg(); //offsets_tg;
    partitionData.edgelist_tg = pContext->getEdgelistTg(); // edgelist_tg;
    partitionData.drglist_tg = pContext->getDrglistTg(); //drglist_tg;
    partitionData.start_vertex = pContext->getStartVertex(); //start_vertex;
    partitionData.end_vertex = pContext->getEndVertex(); //end_vertex;
    partitionData.NV_par_recommand = NV_par_recommand;
    int64_t number_of_partitions = (int64_t)pLouvainMod->addPartitionData(partitionData);
    return number_of_partitions;

}

inline void udf_finish_partition(MapAccum<uint64_t, int64_t> numAlveoPars){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    xilinx_apps::louvainmod::LouvainMod *pLouvainMod = pContext->getLouvainModObj();
    for(unsigned i=0;i<numAlveoPars.size();i++){
        pContext->addNumAlveoPartitions(((int)numAlveoPars.get(i)));
    }
#ifdef XILINX_COM_DETECT_DEBUG_ON
    for(unsigned i=0;i<numAlveoPars.size();i++){
        std::cout << "numAlveoPartition: " << pContext->getNumAlveoPartitions()[i] <<std::endl;
    }
#endif
    pLouvainMod->finishPartitioning(pContext->getNumAlveoPartitions().data());
}

// TODO: Change signature as needed
// This function combined with GSQL code should traverse memory of TigerGraph on Each
// server and build the partitions for each server in the form Louvain host code can consume it
inline int udf_load_alveo(uint num_partitions, uint num_devices)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    int retVal = 0;
    retVal = load_alveo_partitions(num_partitions, num_devices);    

 /* TODO
    std::lock_guard<std::mutex> lockGuard(xai::writeMutex);
    int ret=0;
    if (!xai::loadedAlveo) {
        if(xai::loadedAlveo) return 0;
        xai::loadedAlveo = true;
        xai::num_partitions = num_partitions;
        xai::num_devices = num_devices;
    }
*/    
    // TODO: make call into host code and add needed functions
    return retVal;
}

inline bool udf_close_alveo(int mode)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    return true;
}

inline int udf_execute_reset(int mode) {
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    pContext->clear();
}


inline float udf_louvain_alveo(
    std::string graph_name, int64_t max_iter, int64_t max_level, float tolerance, bool intermediateResult,
    bool verbose, string result_file, bool final_Q, bool all_Q)
{        
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();

    //std::string alveo_parj_path = pContext->getXGraphStorePath() + "/" + graph_name;
    //std::cout << "DEBUG: in udf_louvain_alveo alveo_parj_path = " << alveo_parj_path << std::endl;
    //pContext->setAlveoProject(alveo_parj_path);
    pContext->setAlveoProject(graph_name);

    xilinx_apps::louvainmod::LouvainMod *pLouvainMod = pContext->getLouvainModObj();

    // when restore pContext, nodeId  is missing
 //   unsigned nodeId = pContext->getNodeId();
    std::string alveoProject = pContext->getAlveoProject() + ".par.proj";
    xilComDetect::LouvainMod::ComputeOptions computeOpts;

    float finalQ;

    pLouvainMod->setAlveoProject((char *)alveoProject.c_str());

    computeOpts.outputFile = (char *)result_file.c_str();
    computeOpts.max_iter = max_iter;
    computeOpts.max_level = max_level; 
    computeOpts.tolerance = tolerance; 
    computeOpts.intermediateResult = intermediateResult;
    computeOpts.final_Q = final_Q;
    computeOpts.all_Q = all_Q; 

    std::cout
        << "DEBUG: " << __FUNCTION__ 
        << "\n    computeOpts.max_iter=" << computeOpts.max_iter 
        << "\n    computeOpts.max_level=" << computeOpts.max_level
        << "\n    computeOpts.tolerance=" << computeOpts.tolerance 
        << "\n    computeOpts.intermediateResult=" << computeOpts.intermediateResult
        << "\n    computeOpts.final_Q=" << computeOpts.final_Q 
        << "\n    computeOpts.all_Q=" << computeOpts.all_Q 
        << std::endl;
 
    finalQ = pLouvainMod->loadAlveoAndComputeLouvain(computeOpts); 
    
    std::cout << "DEBUG: Returned from " << __FUNCTION__ 
              << " finalQ=" << finalQ << std::endl;

    return finalQ;
}

// mergeHeaders 1 section body end xilinxComDetect DO NOT REMOVE!

}

#endif /* XILINX_COM_DETECT_HPP */
