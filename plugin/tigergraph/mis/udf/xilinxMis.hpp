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

#ifndef XILINX_MIS_HPP
#define XILINX_MIS_HPP

// mergeHeaders 1 name xilinxMis
// mergeHeaders 1 section include start xilinxMis DO NOT REMOVE!
#include "xilinxMisImpl.hpp"
#include <cstdint>
#include <vector>
#include <random>
// mergeHeaders 1 section include end xilinxMis DO NOT REMOVE!

namespace UDIMPL {

// mergeHeaders 1 section body start xilinxMis DO NOT REMOVE!
inline int64_t rand_int (int minVal, int maxVal) {
    std::random_device rd;
    std::mt19937 e1(rd());
    std::uniform_int_distribution<int> dist(minVal, maxVal);
    return (int64_t) dist(e1);
}

inline void udf_init_graph() {
    xilMis::Graph *graph = xilMis::Graph::getGraph();
}

inline int udf_get_next_vid() {
    xilMis::Lock guard(xilMis::getMutex());
    xilMis::Graph *graph = xilMis::Graph::getGraph();
    return graph->getNextVid();
}

// mergeHeaders 1 section body end xilinxMis DO NOT REMOVE!

}

#endif /* XILINX_MIS_HPP */