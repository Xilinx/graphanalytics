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

// mergeHeaders 1 section include start xilinxComDetect DO NOT REMOVE!
#include "xilinxComDetectImpl.hpp"
#include <cstdint>
#include <vector>
// mergeHeaders 1 section include end xilinxComDetect DO NOT REMOVE!

namespace UDIMPL {

// mergeHeaders 1 section body start xilinxComDetect DO NOT REMOVE!
inline void udf_reset_nextId() {

    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    std::cout << "nextId_ = " << pContext->nextId_ <<std::endl;
    pContext->nextId_ = 0 ;
    pContext->degree_list.clear();
    pContext->edgeListMap.clear();
    pContext->dgrListMap.clear();
    pContext->edgeListVec.clear();
    pContext->dgrListVec.clear();
}
inline uint64_t udf_get_nextId(uint64_t primary_id, uint64_t out_degree){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    pContext->degree_list.push_back((long)out_degree);
    std::cout << "primary_id = " << primary_id << " louvainId = " << pContext->nextId_ <<" out_degree = " << out_degree <<std::endl;
    return pContext->nextId_++;

}

inline uint64_t udf_get_partition_size(){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
     xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
     std::cout << "Partition Size = " << pContext->nextId_ <<std::endl;
     return pContext->nextId_;
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
    return pContext->louvain_offset + louvain_id ;
}

inline void udf_set_louvain_offset(uint64_t louvain_offset){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    std::cout << "Louvain Offsets = " << louvain_offset <<std::endl;
    pContext->louvain_offset = louvain_offset;
}


inline void udf_set_louvain_edge_list(uint64_t primaryIdSource, uint64_t primaryIdTarget, uint64_t louvainIdSource, uint64_t louvainIdTarget, float wtAttr, uint64_t outDgr) {
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    std::cout <<"primaryIdSource = "<<primaryIdSource<< " primaryIdTarget = "<<primaryIdTarget <<" louvainIdSource: " << louvainIdSource << ";louvainIdTarget: " << louvainIdTarget << "; weight: " << wtAttr << "; outDgr: " << outDgr << std::endl;
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    pContext->edgeListMap[louvainIdSource].push_back(xilinx_apps::louvainmod::Edge((long)louvainIdSource,(long)louvainIdTarget, (double)wtAttr));
    pContext->dgrListMap[louvainIdTarget]=outDgr;
}

inline void udf_start_partition(string alveo_project){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    std::string alveo_parj_path = "/home2/tigergraph/" + alveo_project;
    pContext->setAlveoProject(alveo_parj_path);
    xilinx_apps::louvainmod::LouvainMod *pLouvainMod = pContext->getLouvainModObj();
    xilinx_apps::louvainmod::LouvainMod::PartitionOptions options; //use default value
    pLouvainMod->startPartitioning(options);
}



//Data has been populated and send to FPGA

inline void udf_save_alveo_partition() {
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    //build offsets_tg
    int offsets_tg_size = (pContext->degree_list).size();
    pContext->offsets_tg = &pContext->degree_list[0];
    for(int i = 1;i < offsets_tg_size;i++){
        pContext->offsets_tg[i] += pContext->offsets_tg[i-1];
    }
    std::cout<<"Last offsets_tg "<<pContext->offsets_tg[offsets_tg_size-1]<<std::endl;

    //build dgr list and edgelist
    //traverse the partition size for each louvainId, populate edgelist from edgeListMap
    pContext->start_vertex = pContext->louvain_offset;
    pContext->end_vertex = pContext->louvain_offset + pContext->nextId_ -1 ; // the end vertex on local partition
    for(int i= pContext->start_vertex;i <= pContext->end_vertex ; i++) {
        pContext->edgeListVec.insert(pContext->edgeListVec.end(),pContext->edgeListMap[i].begin(),pContext->edgeListMap[i].end());
        for(auto& it:pContext->edgeListMap[i]) {
            pContext->dgrListVec.push_back(pContext->dgrListMap[it.tail]);
        }
    }
    pContext->drglist_tg = pContext->dgrListVec.data();
    pContext->edgelist_tg = pContext->edgeListVec.data();

    std::cout << "edgelist size:" << pContext->edgeListVec.size() << std::endl;
    std::cout << "dgrlist size:" << pContext->dgrListVec.size() << std::endl;
    for(int i=0; i < 10; i++) {

        std::cout << i << " head from edgelist  " << pContext->edgelist_tg[i].head <<"; ";
        std::cout << " tail from edgelist: " << pContext->edgelist_tg[i].tail <<"; ";
        std::cout << " outDgr from dgrlist: " << pContext->drglist_tg[i] << std::endl;
    }


    std::cout << "start_vertex: " << pContext->start_vertex<<std::endl;
    std::cout << "end_vertex: " << pContext->end_vertex <<std::endl;
    

    xilinx_apps::louvainmod::LouvainMod *pLouvainMod = pContext->getLouvainModObj();
    xilinx_apps::louvainmod::LouvainMod::PartitionData partitionData;//write into partionData = {pContext->offsets_tg, pContext->edgelist_tg, pContext->drglist_tg, pContext->start_vertex,pContext->end_vertex, 0};
    partitionData.offsets_tg = pContext->offsets_tg;
    partitionData.edgelist_tg = pContext->edgelist_tg;
    partitionData.drglist_tg = pContext->drglist_tg;
    partitionData.start_vertex = pContext->start_vertex;
    partitionData.end_vertex = pContext->end_vertex;
    pLouvainMod->addPartitionData(partitionData);

}

inline void udf_finish_partition(){
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    xilinx_apps::louvainmod::LouvainMod *pLouvainMod = pContext->getLouvainModObj();
    pLouvainMod->finishPartitioning();
}



inline int udf_xilinx_comdetect_setup_nodes(std::string nodeNames, 
                                            std::string nodeIps)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    std::cout << "DEBUG: " << __FUNCTION__  
              << " nodeNames=" << nodeNames 
              << " nodeIps=" << nodeIps << std::endl;
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();

    std::istringstream issNodeNames(nodeNames);
    std::istringstream issNodeIps(nodeIps);
    std::string nodeName;
    std::string nodeIp;
    unsigned numNodes = 0;
    while ((issNodeNames >> nodeName) && (issNodeIps >> nodeIp))
    {
        std::cout << "DEBUG " << numNodes++ << ":" << nodeName 
                  << nodeIp << std::endl;
    } while (issNodeNames && issNodeIps);


    pContext->setNumNodes(unsigned(numNodes));
    return numNodes;
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


inline int udf_create_and_load_alveo_partitions(bool use_saved_partition, 
    string graph_file, string alveo_project, uint num_nodes, uint num_partitions, 
    uint num_devices)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    int ret=0;


    pContext->setAlveoProject(alveo_project);
    xilinx_apps::louvainmod::LouvainMod *pLouvainMod = pContext->getLouvainModObj();
    xilinx_apps::louvainmod::LouvainMod::PartitionOptions options;
    pLouvainMod->startPartitioning(options);
    
/*
    std::lock_guard<std::mutex> lockGuard(xai::writeMutex);
    int ret=0;
    if (!xai::loadedAlveo) {
        if(xai::loadedAlveo) return 0;
        xai::loadedAlveo = true;
        xai::graph_file = graph_file;
*/      

    unsigned nodeId = pContext->getNodeId();
    int argc = 9;
    char* argv[] = {"host.exe", graph_file.c_str(), "-fast", "-par_num", std::to_string(num_partitions).c_str(),
        "-create_alveo_partitions", "-name", alveo_project.c_str(), "-server_par", NULL};
    std::cout << "DEBUG: " << __FUNCTION__ << " nodeId=" << nodeId
              << " graph_file=" << graph_file.c_str() 
              << " num_partitions=" << num_partitions 
              << " louvain project=" << alveo_project.c_str() 
//              << " cur_node_ip=" << cur_node_ip 
//              << " cur_node_hostname=" << cur_node_hostname 
//              << " node_ips=" << node_ips 
              << std::endl;
        
    
    if (!use_saved_partition && nodeId == 0) {
        std::cout << "DEBUG: Calling create_alveo_partitions" << std::endl;
        ret = create_and_load_alveo_partitions(argc, (char**)(argv));
    }
    return ret;
}


inline bool udf_close_alveo(int mode)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    return true;
}

/*
inline int udf_create_alveo_partitions(std::string input_graph, std::string partitions_project, std::string num_patitions)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    pContext->clear();
    int argc = 8;
    char* argv[] = {"host.exe", input_graph.c_str(), "-fast", "-par_num", num_patitions.c_str(),
          "-create_alveo_partitions", "-name", partitions_project.c_str(), nullptr};
    std::cout << "DEBUG: Calling create_partitions. input_graph: " <<  input_graph.c_str()
              << " num_partitions: " << num_patitions.c_str() << " project: " << partitions_project.c_str()
              << std::flush;
//    for (int i = 0; i < argc; ++i)
//        std::cout << "create partitions arg " << i << " = " << argv[i] << std::endl;
    return create_alveo_partitions(argc, (char**)(argv));
}
*/

inline int udf_execute_reset(int mode) {
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    pContext->clear();
}


inline float udf_louvain_alveo(
    int64_t max_iter, int64_t max_level, float tolerence, bool intermediateResult,
    bool verbose, string result_file, bool final_Q, bool all_Q)
{        
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    xilinx_apps::louvainmod::LouvainMod *pLouvainMod = pContext->getLouvainModObj();

    unsigned nodeId = pContext->getNodeId();
    std::string alveoProject = pContext->getAlveoProject() + ".par.proj";
    unsigned numWorkers = pContext->getNumNodes() - 1;
    unsigned numDevices = pContext->getNumDevices();
    unsigned numPartitions = pContext->getNumPartitions(); 
    std::string curNodeIp = pContext->getCurNodeIp();
    std::string nodeIps = pContext->getNodeIps();

    //if (pContext->getState() >= xilComDetect::Context::CalledExecuteLouvainState)
    //    return 0;
    
    //std::cout << "DEBUG: " << __FUNCTION__ 
    //          << " Context::getState()=" << pContext->getState() << std::endl;

    //pContext->setState(xilComDetect::Context::CalledExecuteLouvainState);
    unsigned modeZmq;

    std::istringstream issNodeIps(nodeIps);
    //std::string nodeName;
    std::string nodeIp;
    //char hostname[64 + 1];
    char* nameWorkers[128];
    //int  res_hostname = gethostname(hostname, 64 + 1);
    //if (res_hostname != 0) {
    //    std::cout << "ERROR: gethostname failed" << std::endl;
    //    return res_hostname;
    //}
    std::string tcpConn;
    //std::cout << "DEBUG: nodeId " << nodeId << " hostname=" << hostString << std::endl;
    unsigned iTcpConn = 0;
    while (issNodeIps >> nodeIp) {
        if (nodeIp != curNodeIp) {
            tcpConn = "tcp://" + nodeIp + ":5555";
            std::cout << "DEBUG: zmq=" << tcpConn << std::endl;
            nameWorkers[iTcpConn] = strcpy(new char[tcpConn.length() + 1], 
                                           tcpConn.c_str());
            iTcpConn++;
        } else
            std::cout << "DEBUG: skip nodeIp " << nodeIp << std::endl;
    };

    for (int i=0; i < iTcpConn; i++)
        std::cout << "DEBUG: nameWorker " << i << "=" << nameWorkers[i] << std::endl;

    // nodeId 0 is always the driver. All other nodes are workers.
    if (nodeId == 0) {
        modeZmq = 1; // driver
    } else {
        modeZmq = 2; // worker
    }
    
    std::cout
        << "DEBUG: " << __FUNCTION__ 
        << " XCLBIN_PATH=" << PLUGIN_XCLBIN_PATH
        << " numDevices=" << numDevices << " numPartitions=" << numPartitions
        << " alveoProject=" << alveoProject << " numWorkers=" << numWorkers
        << " nodeId=" << nodeId << std::endl;
    
    float retVal = 0;
    retVal = compute_louvain_alveo(    
                    PLUGIN_XCLBIN_PATH, true, numDevices, 
                    numPartitions, alveoProject.c_str(), 
                    modeZmq, numWorkers, nameWorkers, nodeId,
                    result_file.c_str(),
		    max_iter, max_level, tolerence, intermediateResult, verbose, final_Q, all_Q); 
    
    std::cout << "DEBUG: Returned from execute_louvain. Q=" << retVal << std::endl;
    return retVal;
}

// mergeHeaders 1 section body end xilinxComDetect DO NOT REMOVE!

}

#endif /* XILINX_COM_DETECT_HPP */
