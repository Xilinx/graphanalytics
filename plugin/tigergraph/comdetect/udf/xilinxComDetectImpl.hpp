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

#ifndef XILINX_COM_DETECT_IMPL_HPP
#define XILINX_COM_DETECT_IMPL_HPP

// Use inline definitions for dynamic loading functions
#define XILINX_LOUVAINMOD_INLINE_IMPL
#include "xilinxlouvain.h"

// Enable this to turn on debug output
#define XILINX_COM_DETECT_DEBUG_ON


namespace xilComDetect {

using Mutex = std::mutex;

//#define XILINX_COM_DETECT_DEBUG_MUTEX

#ifdef XILINX_COM_DETECT_DEBUG_MUTEX
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
    enum State {
        UninitializedState = 0,  // no state-updating function has been called
        CalledExecuteLouvainState,  // after execute called
        NumStates
    };
    
private:
    unsigned nodeId_;
    unsigned numNodes_;
    State state_ = UninitializedState;
    unsigned numDevices_ = 1;
    
public:
    static Context *getInstance() {
        static Context *s_pContext = nullptr;
        if (s_pContext == nullptr)
            s_pContext = new Context();
        return s_pContext;
    }
    
    Context() = default;
    ~Context() {}

    void setNodeId(unsigned nodeId) {
        std::cout << "DEBUG: " << __FUNCTION__ << " nodeId=" << nodeId << std::endl;
        nodeId_ = nodeId;
    }

    void setNumNodes(unsigned numNodes) {
        std::cout << "DEBUG: " << __FUNCTION__ << " numNodes=" << numNodes << std::endl;
        numNodes_ = numNodes;
    }

    void setNumDevices(unsigned numDevices) {
        if (numDevices != numDevices_)
            clear();
        numDevices_ = numDevices;
    }
    
    unsigned getNumDevices() const { return numDevices_; }
    
    State getState() const { return state_; }
    void setState(State state) { state_ = state; }
    
    void clear() {
        state_ = UninitializedState;
    }
};


inline bool isHostTheDriver(std::string& workernum)
{
    bool retVal = false;
    const char* driverHostName = "xsj-dxgradb01";
    char hostname[HOST_NAME_MAX + 1];
    int res_hostname = gethostname(hostname, HOST_NAME_MAX + 1);
    if (res_hostname != 0) {
        std::cout << "XAIDEBUG: gethostname failed\n";
        return retVal;
    }
    std::string hostString(hostname);
    std::cout << "XAIDEBUG: host_name: " << hostString;
    int res_comp = hostString.compare(driverHostName);
    if (res_comp == 0) {
        retVal = true;
        workernum = "0";
        std::cout << " Driver \n" << std::flush;
    } else {
        std::cout << " Worker \n" << std::flush;
        if (hostString.compare("xsj-dxgradb02") == 0) {
            workernum = "1";
        } else if (hostString.compare("xsj-dxgradb03") == 0) {
            workernum = "2";
        } else {
            workernum = "3";
        }
    }
    return retVal;
}

}  // namespace xilComDetect

#include "louvainmod_loader.cpp"

#endif /* XILINX_COM_DETECT_IMPL_HPP */


