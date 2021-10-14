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

// mergeHeaders 1 section body end xilinxFuzzyMatch DO NOT REMOVE!

}

#endif /* XILINX_FUZZY_MATCH_HPP */
