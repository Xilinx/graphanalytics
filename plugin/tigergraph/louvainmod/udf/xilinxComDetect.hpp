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

inline string udf_open_alveo(int mode)
{
    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    return "";
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
    std::cout << "Calling create_partitions. input_graph: " <<  input_graph.c_str()
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


inline int udf_execute_alveo_louvain(
    std::string input_graph, std::string partitions_project, 
    std::string num_devices, std::string num_patitions, 
    std::string num_workers, std::string community_file)
{        

    std::lock_guard<std::mutex> lockGuard(xilComDetect::getMutex());
    xilComDetect::Context *pContext = xilComDetect::Context::getInstance();

    if (pContext->getState() >= xilComDetect::Context::CalledExecuteLouvainState)
        return 0;
    
    std::cout << "DEBUG: " << __FUNCTION__ 
              << " Context::getState()=" << pContext->getState() << std::endl;

    pContext->setState(xilComDetect::Context::CalledExecuteLouvainState);
    std::string workernum("1");
    bool isDriver = xilComDetect::isHostTheDriver(workernum);
    int my_argc = 18;
    std::string worker_or_driver("-workerAlone");
    char* optional_arg = nullptr;
    if(isDriver) {
        worker_or_driver = "-driverAlone";
    } else {
        optional_arg = (char*) workernum.c_str();
        my_argc++;
    }
    partitions_project += ".par.proj";

    char* my_argv[] = {"host.exe", "-x", PLUGIN_XCLBIN_PATH, input_graph.c_str(), 
                       "-o", community_file.c_str(), "-fast", "-dev", 
                       num_devices.c_str(), "-par_num", num_patitions.c_str(),
                       "-load_alveo_partitions", partitions_project.c_str(), 
                       "-setwkr", num_workers.c_str(), "tcp://192.168.1.21:5555", "tcp://192.168.1.31:5555", 
                           worker_or_driver.c_str(), optional_arg, nullptr};

    std::cout
        << "Calling execute_louvain. input_graph: " <<  input_graph.c_str()
        << " num_partitions: " << num_patitions.c_str()
        << " project: " << partitions_project.c_str()
        << " num workers: " << num_workers.c_str()
        << " worker_or_driver: " << worker_or_driver.c_str()
        << " worker num: " << workernum.c_str() << "\n"
        << std::flush;

    //if(isDriver) {
    //unsigned int sleep_time = 240;
    //std::cout << "KD: Going to slpep for seconds: " << sleep_time << "\n";
    //std::this_thread::sleep_for (std::chrono::seconds(sleep_time));
    //std::cout << "KD: Woke up from sleep after seconds: " << sleep_time << "\n";
    //}
//    for (int i = 0; i < sizeof(my_argv)/sizeof(char *) - 1; ++i)
//        std::cout << "load partitions arg " << i << " = " << my_argv[i] << std::endl;
    int retVal = load_alveo_partitions(my_argc, (char**)(my_argv));
    std::cout << "KD: Returned from execute_louvain, isDriver = " << isDriver << "\n" << std::flush;
    return retVal;
}

// mergeHeaders 1 section body end xilinxComDetect DO NOT REMOVE!

}

#endif /* XILINX_COM_DETECT_HPP */
