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

#ifndef XILINXRECOMENGINE_HPP
#define XILINXRECOMENGINE_HPP

// Use inline definitions for dynamic loading functions
#define XILINX_COSINESIM_INLINE_IMPL
#include "cosinesim.hpp"

// Enable this to turn on debug output
#define XILINX_RECOM_DEBUG_ON


namespace xilRecom {

using Mutex = std::mutex;

//#define XILINX_RECOM_DEBUG_MUTEX

#ifdef XILINX_RECOM_DEBUG_MUTEX
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


using CosineSim = xilinx_apps::cosinesim::CosineSim<std::int32_t>;

class Context {
public:
    using IdMap = std::vector<std::uint64_t>;
    
private:
    unsigned nodeId_;
    unsigned numDevices_ = 1;
    xilinx_apps::cosinesim::ColIndex vectorLength_ = 0;
    bool vectorLengthSet_ = false;
    bool isInitialized_ = false;
    CosineSim *pCosineSim_ = nullptr;
    IdMap idMap_;  // maps from vector ID to FPGA row number
    
public:
    static Context *getInstance() {
        static Context *s_pContext = nullptr;
        if (s_pContext == nullptr)
            s_pContext = new Context();
        return s_pContext;
    }
    
    Context() = default;
    ~Context() { delete pCosineSim_; }
    
    void setNodeId(unsigned nodeId) {
        std::cout << "DEBUG: " << __FUNCTION__ << " nodeId=" << nodeId << std::endl;
        nodeId_ = nodeId;
    }

    unsigned getNodeId() const { return nodeId_;}

    void setNumDevices(unsigned numDevices) {
        if (numDevices != numDevices_)
            clear();
        numDevices_ = numDevices;
    }
    
    unsigned getNumDevices() const { return numDevices_; }
    
    void setVectorLength(xilinx_apps::cosinesim::ColIndex vectorLength) {
        if (vectorLengthSet_ && vectorLength != vectorLength_)
            clear();
        vectorLength_ = vectorLength;
        vectorLengthSet_ = true;
    }
    
    xilinx_apps::cosinesim::ColIndex getVectorLength() const { return vectorLength_; }
    
    CosineSim *getCosineSimObj() {
        if (pCosineSim_ == nullptr) {
            xilinx_apps::cosinesim::Options options;
            options.vecLength = vectorLength_;
            options.numDevices = numDevices_;
            options.xclbinPath = PLUGIN_XCLBIN_PATH;
            options.deviceNames = "xilinx_u50_gen3x16_xdma_201920_3";
#ifdef XILINX_RECOM_DEBUG_ON
            std::cout << "DEBUG: cosinesim options: " 
                    << "\n    vecLength=" << options.vecLength
                    << "\n    numDevices=" << options.numDevices
                    << "\n    xclbinPath=" << options.xclbinPath << std::endl;
#endif
            pCosineSim_ = new CosineSim(options);
#ifdef XILINX_RECOM_DEBUG_ON
            std::cout << "DEBUG: Created cosinesim object " << pCosineSim_
                    << " with vecLength=" << options.vecLength
                    << ", numDevices=" << options.numDevices << std::endl;
#endif
        }
        
        return pCosineSim_;
    }

    IdMap &getIdMap() { return idMap_; }
    
    void setInitialized() { isInitialized_ = true; }
    
    bool isInitialized() const { return isInitialized_; }

    void clear() {
        isInitialized_ = false;
        vectorLengthSet_ = false;
        idMap_.clear();
        delete pCosineSim_;
        pCosineSim_ = nullptr;
    }
};


}  // namespace xilRecom

#include "cosinesim_loader.cpp"

#endif /* XILINXRECOMENGINE_HPP */

