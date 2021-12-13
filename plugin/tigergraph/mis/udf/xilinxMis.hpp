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

inline int udf_get_next_vid() {
    xilMis::Lock guard(xilMis::getMutex());
    xilMis::Context *context = xilMis::Context::getContext();
    return context->getNextVid();
}

inline void udf_build_row_ptr(int num_edges)
{
    xilMis::Context *context = xilMis::Context::getContext();
    context->addRowPtrEntry(num_edges);
}

inline void udf_build_col_idx(int vid)
{
    xilMis::Context *context = xilMis::Context::getContext();
    context->addColIdxEntry(vid);
}

inline int udf_xilinx_mis()
{
    xilMis::Context *context = xilMis::Context::getContext();

    // set MIS options
    xilinx_apps::mis::Options options;
    options.xclbinPath = context->getXclbinPath();
    std::cout << "DEBUG: XCLBIN=" << options.xclbinPath << std::endl;
    options.deviceNames = context->getDeviceNames();

    xilinx_apps::mis::MIS xmis(options);

    xilinx_apps::mis::GraphCSR<int> graph(context->getRowPtr(), context->getColIdx());

    xmis.startMis();


    xmis.setGraph(&graph);

    xmis.executeMIS();

    return xmis.count();
}

// mergeHeaders 1 section body end xilinxMis DO NOT REMOVE!

}

#endif /* XILINX_MIS_HPP */