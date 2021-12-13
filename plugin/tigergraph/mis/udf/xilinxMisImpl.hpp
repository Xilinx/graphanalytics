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
#define XILINX_MIS_INLINE_IMPL
#include "xilinxmis.hpp"

// Enable this to turn on debug output
#define XILINX_MIS_DEBUG_ON

// Enable this to dump context vertices and edges
//#define XILINX_MIS_DUMP_context

// Enable this to dump an .mtx file of the context
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

class Context {
public:
    Context() {
        // default values
        vid_ = 0;
        row_id_ = 0;
        rowPtr_.push_back(0);

        // set xclbinPath
        if (deviceNames_ == "xilinx_u50_gen3x16_xdma_201920_3") {
            xclbinPath_ = PLUGIN_XCLBIN_PATH_U50;
        } else if (deviceNames_ == "xilinx_aws-vu9p-f1_shell-v04261818_201920_2") {
            xclbinPath_ = PLUGIN_XCLBIN_PATH_AWSF1;
        }

        std::cout << "DEBUG: " << __FILE__ << "Context"
                  << "\n    deviceNames=" << deviceNames_ 
                  << "\n    xclbinPath_=" << xclbinPath_
                  << std::endl; 
    }

    static Context *getContext() {
        static Context *l_context = nullptr;
        if (l_context == nullptr)
            l_context = new Context();
        return l_context;
    }

    int getNextVid() { return vid_++; }
    void addRowPtrEntry( uint32_t x ) { rowPtr_.push_back( rowPtr_[row_id_++] + x ); }
    void addColIdxEntry( uint32_t x ) { colIdx_.push_back( x ); }
    std::string getXclbinPath() { return xclbinPath_; }
    std::string getDeviceNames() { return deviceNames_; }

    std::vector<uint32_t>& getRowPtr() { return rowPtr_; }
    std::vector<uint32_t>& getColIdx() { return colIdx_; }

private:
    int vid_;
    int row_id_;
    std::vector<uint32_t> rowPtr_;
    std::vector<uint32_t> colIdx_;

    std::string deviceNames_ = "xilinx_u50_gen3x16_xdma_201920_3";
    std::string xclbinPath_;

};

} /* namespace xilMis */

#include "mis_loader.cpp"

#endif /* XILINX_MIS_IMPL_HPP */