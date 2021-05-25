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
    unsigned nodeId_ = 0;
    unsigned numNodes_ = 1;
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

    unsigned getNodeId() { return nodeId_; }
    
    void setNumNodes(unsigned numNodes) 
    {
        std::cout << "DEBUG: " << __FUNCTION__ << " numNodes=" << numNodes << std::endl;
        numNodes_ = numNodes;
    }

    unsigned getNumNodes() { return numNodes_; }

    void setNumDevices(unsigned numDevices) 
    {
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

}  // namespace xilComDetect

#include "louvainmod_loader.cpp"

#endif /* XILINX_COM_DETECT_IMPL_HPP */


