/*
 * Copyright 2021 Xilinx, Inc.
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

#ifndef XILINX_MIS_IMPL_HPP
#define XILINX_MIS_IMPL_HPP

// Use inline definitions for dynamic loading functions
#define XILINX_LOUVAINMOD_INLINE_IMPL
//#include "xilinxmis.h"

// Enable this to turn on debug output
#define XILINX_MIS_DEBUG_ON

// Enable this to dump graph vertices and edges
//#define XILINX_MIS_DUMP_GRAPH

// Enable this to dump an .mtx file of the graph
//#define XILINX_MIS_DUMP_MTX

#include <vector>
#include <map>
#include <fstream>

namespace xilMis {

using Mutex = std::mutex;

#define XILINX_MIS_DEBUG_MUTEX

#ifdef XILINX_MIS_DEBUG_MUTEX
struct Lock {
    using RealLock = std::lock_guard<Mutex>;
    RealLock lock_;

    Lock(Mutex &m)
    : lock_(m)
    {
        std::cout << "MUTEX: " << (void *) (&m) << std::endl;
    }
};
#else
using Lock = std::lock_guard<Mutex>;
#endif

inline Mutex &getMutex() {
    static Mutex *pMutex = nullptr;
    if (pMutex == nullptr)
        pMutex = new Mutex();
    return *pMutex;
}

class Graph {
public:
    Graph() {
        vid_ = 0;
    }

    static Graph *getGraph() {
        static Graph *l_graph = nullptr;
        if (l_graph == nullptr)
            l_graph = new Graph();
        return l_graph;
    }

    int getNextVid() { return vid_++; }

private:
    int vid_;

};

} /* namespace xilMis */

#endif /* XILINX_MIS_IMPL_HPP */