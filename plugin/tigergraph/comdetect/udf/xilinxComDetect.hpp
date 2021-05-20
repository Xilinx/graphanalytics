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

inline int udf_xilinx_comdetect_set_num_nodes(uint numNodes)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    std::cout << "DEBUG: " << __FUNCTION__ << " numNodes=" << numNodes << std::endl;
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();

    pContext->setNumNodes(unsigned(numNodes));
    return numNodes;
}

inline bool udf_close_alveo(int mode)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    return true;
}


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


inline int udf_execute_reset(int mode) {
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();
    pContext->clear();
}


inline float udf_execute_alveo_louvain(
    std::string input_graph, std::string partitions_project, 
    std::string num_devices, std::string num_patitions, 
    std::string num_workers, std::string community_file)
{        

    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();

    //if (pContext->getState() >= xilComDetect::Context::CalledExecuteLouvainState)
    //    return 0;
    
    //std::cout << "DEBUG: " << __FUNCTION__ 
    //          << " Context::getState()=" << pContext->getState() << std::endl;

    //pContext->setState(xilComDetect::Context::CalledExecuteLouvainState);
    std::string workernum;

    std::string worker_or_driver("-workerAlone");
    int my_argc = 18;
    char* optional_arg = nullptr;

    // nodeId 0 is always the driver. All other nodes are workers.
    unsigned nodeId = pContext->getNodeId();
    if (nodeId == 0) {
        worker_or_driver = "-driverAlone";
    } else {
        optional_arg = (char*) workernum.c_str();
        my_argc++;
    }
    workernum = std::to_string(nodeId);

    std::cout << "DEBUG: " << __FUNCTION__ << " workernum=" << workernum 
              << " isdriver=" << (nodeId == 0) << std::endl;

    char* my_argv[] = {"host.exe", "-x", PLUGIN_XCLBIN_PATH, input_graph.c_str(), 
                       "-o", community_file.c_str(), "-fast", "-dev", 
                       num_devices.c_str(), "-par_num", num_patitions.c_str(),
                       "-load_alveo_partitions", partitions_project.c_str(), 
                       "-setwkr", num_workers.c_str(), "tcp://192.168.1.21:5555", "tcp://192.168.1.31:5555", 
                           worker_or_driver.c_str(), optional_arg, nullptr};

    std::cout
        << "DEBUG: "
        << "Calling execute_louvain. input_graph: " <<  input_graph.c_str()
        << " num_partitions: " << num_patitions.c_str()
        << " project: " << partitions_project.c_str()
        << " num workers: " << num_workers.c_str()
        << " worker_or_driver: " << worker_or_driver.c_str()
        << " worker num: " << workernum.c_str() << "\n"
        << std::flush;

//    for (int i = 0; i < sizeof(my_argv)/sizeof(char *) - 1; ++i)
//        std::cout << "load partitions arg " << i << " = " << my_argv[i] << std::endl;
    float retVal = load_alveo_partitions(my_argc, (char**)(my_argv));
    std::cout << "DEBUG: Returned from execute_louvain. Q=" << retVal << std::endl;
    return retVal;
}

// mergeHeaders 1 section body end xilinxComDetect DO NOT REMOVE!

}

#endif /* XILINX_COM_DETECT_HPP */
