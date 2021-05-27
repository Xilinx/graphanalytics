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

inline int udf_xilinx_comdetect_set_node_id(uint nodeId)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    std::cout << "DEBUG: " << __FUNCTION__ << " nodeId=" << nodeId << std::endl;
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();

    pContext->setNodeId(unsigned(nodeId));
    return nodeId;
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

//inline string udf_open_alveo(int mode)
//{
//    std::lock_guard<std::mutex> lockGuard(xai::writeMutexOpenAlveo);
//    if (!xai::openedAlveo) {
//        std::cout << "DEBUG: " << __FUNCTION__ << " xai::openedAlveo=" << xai::openedAlveo << std::endl;
//        if(xai::openedAlveo) return "";
//        xai::openedAlveo = true;
//        string result("Initialized Alveo");
//        try
//        {
//            std::cout << "DEBUG: Opening XAI library " << std::endl;
//            xai::xaiLoader.load_library(xai::host_libname);
//            std::cout << "DEBUG: Opened XAI library " << std::endl;
//        }
//        catch (std::exception& e) {
//            std::cerr << "ERROR: An exception occurred: " << e.what() << std::endl;
//            (result += ": STD Exception:")+=e.what();
//        }
//        catch (...) {
//            std::cerr << "ERROR: An unknown exception occurred." << std::endl;
//            (result += ": Unknown Exception:")+=" Reason unknown, check LD_LIBRARY_PATH";
//        }
//        return result;
//    }
//    return "";
//}

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

    std::string cur_node_hostname, cur_node_ip, node_ips;
    // PLUGIN_CONFIG_PATH will be replaced by the actual config path during plugin installation
    std::fstream config_json(PLUGIN_CONFIG_PATH, std::ios::in);
    if (!config_json) {
        std::cout << "ERROR: config file doesn't exist:" << PLUGIN_CONFIG_PATH << std::endl;
        return(2);
    }

    char line[1024] = {0};
    char* token;
    while (config_json.getline(line, sizeof(line))) {
        token = strtok(line, "\"\t ,}:{\n");
        while (token != NULL) {
            if (!std::strcmp(token, "curNodeHostname")) {
                token = strtok(NULL, "\"\t ,}:{\n");
                cur_node_hostname = token;
            } else if (!std::strcmp(token, "curNodeIp")) {
                token = strtok(NULL, "\"\t ,}:{\n");
                cur_node_ip = token;
            } else if (!std::strcmp(token, "nodeIps")) {
                token = strtok(NULL, "\"\t ,}:{\n");
                node_ips = token;
            } 
            token = strtok(NULL, "\"\t ,}:{\n");
        }
    }
    config_json.close();

    pContext->setAlveoProject(alveo_project);
    pContext->setNumNodes(num_nodes);
    pContext->setNumPartitions(num_partitions);
    pContext->setNumDevices(num_devices);
    pContext->setCurNodeIp(cur_node_ip);
    pContext->setNodeIps(node_ips);

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
              << " cur_node_ip=" << cur_node_ip 
              << " cur_node_hostname=" << cur_node_hostname 
              << " node_ips=" << node_ips << std::endl;
        
    
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
