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


namespace xai {

using CosineSim = xilinx_apps::cosinesim::CosineSim<std::int32_t>;

class Context {
public:
    using IdMap = std::vector<std::uint64_t>;
    
private:
    unsigned numDevices_ = 1;
    xilinx_apps::cosinesim::ColIndex vectorLength_ = 0;
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
    
    void setNumDevices(unsigned numDevices) {
        if (numDevices != numDevices_)
            clear();
        numDevices_ = numDevices;
    }
    
    unsigned getNumDevices() const { return numDevices_; }
    
    void setVectorLength(xilinx_apps::cosinesim::ColIndex vectorLength) {
        if (vectorLength != vectorLength_)
            clear();
        vectorLength_ = vectorLength;
    }
    
    xilinx_apps::cosinesim::ColIndex getVectorLength() const { return vectorLength_; }
    
    CosineSim *getCosineSimObj() {
        if (pCosineSim_ == nullptr) {
            xilinx_apps::cosinesim::Options options;
            options.vecLength = vectorLength_;
            options.devicesNeeded = numDevices_;
            options.xclbinPath = "TG_COSINESIM_XCLBIN";
            pCosineSim_ = new CosineSim(options);
#ifdef XILINX_RECOM_DEBUG_ON
            std::cout << "DEBUG: Created cosinesim object " << pCosineSim_
                    << " with vecLength=" << options.vecLength
                    << ", devicesNeeded=" << options.devicesNeeded << std::endl;
#endif
        }
        
        return pCosineSim_;
    }

    IdMap &getIdMap() { return idMap_; }
    
    void setInitialized() { isInitialized_ = true; }
    
    bool isInitialized() const { return isInitialized_; }

    void clear() {
        isInitialized_ = false;
        idMap_.clear();
        delete pCosineSim_;
        pCosineSim_ = nullptr;
    }
};


}  // namespace xai

#include "cosinesim_loader.cpp"

#endif /* XILINXRECOMENGINE_HPP */

