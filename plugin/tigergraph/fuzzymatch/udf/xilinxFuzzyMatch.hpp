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

#ifndef XILINX_FUZZY_MATCH_HPP
#define XILINX_FUZZY_MATCH_HPP

// mergeHeaders 1 name xilinxFuzzyMatch
// mergeHeaders 1 section include start xilinxFuzzyMatch DO NOT REMOVE!
#include "xilinxFuzzyMatchImpl.hpp"
#include <cstdint>
#include <vector>
// mergeHeaders 1 section include end xilinxFuzzyMatch DO NOT REMOVE!

namespace UDIMPL {

// mergeHeaders 1 section body start xilinxFuzzyMatch DO NOT REMOVE!

inline int udf_xilinx_fuzzymatch_set_node_id(uint nodeId)
{
    std::lock_guard<std::mutex> lockGuard(xilFuzzyMatch::getMutex());
    std::cout << "DEBUG: " << __FUNCTION__ << " nodeId=" << nodeId << std::endl;
    xilFuzzyMatch::Context *pContext = xilFuzzyMatch::Context::getInstance();

    pContext->setNodeId(unsigned(nodeId));
    return nodeId;
}

// Return value:
//    0: Success
//   -1: Failed to initialize Alveo device
//   
inline int udf_fuzzymatch_alveo(ListAccum<string> sourceList, ListAccum<string> targetList) 
{

    std::cout << "INFO: udf_fuzzymatch_alveo " << std::endl;

    std::vector<std::string> sourceVector;
    xilFuzzyMatch::Context *pContext = xilFuzzyMatch::Context::getInstance();
    xilinx_apps::fuzzymatch::FuzzyMatch *pFuzzyMatch = pContext->getFuzzyMatchObj();
    bool match_result;

    if (pFuzzyMatch->startFuzzyMatch() < 0) {
        std::cout << "ERROR: Failed to initialize Alveo device" << std::endl;
        return -1;
    }

    uint32_t sourceListLen = sourceList.size();
    for (unsigned i = 0 ; i < sourceListLen; ++i)
        sourceVector.push_back(sourceList.get(i));

    std::cout << "sourceVector size=" << sourceVector.size() << std::endl;
    pFuzzyMatch->fuzzyMatchLoadVec(sourceVector);

    uint32_t targetListLen = targetList.size();
    for (unsigned i = 0 ; i < targetListLen; ++i) {       
        // TigerGraph uses gcc 4.8.5 while latest Linux has newer gcc (e.g. 9.3 
        // on Ubuntu 20.04). This causes incompatibility issue when passing 
        // std::string between TG UDF and Xilinx libraries. The workaround documented 
        // in https://stackoverflow.com/questions/33394934/converting-std-cxx11string-to-stdstring
        // is added to Xilinx standalone library Makefile: -D_GLIBCXX_USE_CXX11_ABI=0
        match_result = pFuzzyMatch->executefuzzyMatch(targetList.get(i));
        std::cout << i << "," << targetList.get(i) << "," << (match_result ? "KO" : "OK") << ","
                  << (match_result ? ":Sender" : "") << std::endl;
    }

    return 0;
}

// mergeHeaders 1 section body end xilinxFuzzyMatch DO NOT REMOVE!

}

#endif /* XILINX_FUZZY_MATCH_HPP */
