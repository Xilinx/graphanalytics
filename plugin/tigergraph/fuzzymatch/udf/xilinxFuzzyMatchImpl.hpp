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

#ifndef XILINX_FUZZYMATCH_IMPL_HPP
#define XILINX_FUZZYMATCH_IMPL_HPP

// Use inline definitions for dynamic loading functions
#define XILINX_FUZZYMATCH_INLINE_IMPL
#include "fuzzymatch.hpp"

// Enable this to turn on debug output
#define XILINX_FUZZYMATCH_DEBUG_ON

#include <vector>
#include <map>
#include <fstream>

namespace xilFuzzyMatch {

using Mutex = std::mutex;

#define XILINX_FUZZYMATCH_DEBUG_MUTEX

#ifdef XILINX_FUZZYMATCH_DEBUG_MUTEX
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

using FuzzyMatch = xilinx_apps::fuzzymatch::FuzzyMatch;

class Context {
public:
    enum State {
        UninitializedState = 0,  // no state-updating function has been called
        CalledExecuteFuzzyState,  // after execute called
        NumStates
    };


private:
    bool fuzzyMatchObjIsModified_ = false;  // true if a parameter has changed that requires a rebuild of

    std::string curNodeIp_;
    std::string nodeIps_;
    std::string xGraphStorePath_;
    std::string xclbinPath_;
    uint32_t numDevices_ = 1;

    uint32_t nodeId_ = 0;
    uint32_t numNodes_ = 1;
    State state_ = UninitializedState;
    FuzzyMatch *pFuzzyMatch_ = nullptr;

public:
    std::string curNodeHostname_;
    std::string deviceNames_ = "xilinx_u50_gen3x16_xdma_201920_3";

    static Context *getInstance() {
        static Context *s_pContext = nullptr;
        if (s_pContext == nullptr)
            s_pContext = new Context();
        return s_pContext;
    }
    
    Context() {
        // PLUGIN_CONFIG_PATH will be replaced by the actual config path during plugin installation
        std::fstream config_json(PLUGIN_CONFIG_PATH, std::ios::in);
        if (!config_json) {
            std::cout << "ERROR: config file doesn't exist:" << PLUGIN_CONFIG_PATH << std::endl;
            return;
        }

        char line[1024] = {0};
        char* token;
        nodeIps_.clear();
        bool scanNodeIp;
#ifdef XILINX_FUZZYMATCH_DEBUG_ON
        std::cout << "DEBUG: Parsing config file " << PLUGIN_CONFIG_PATH << std::endl;
#endif
        while (config_json.getline(line, sizeof(line))) {
            token = strtok(line, "\"\t ,}:{\n");
            scanNodeIp = false;
            while (token != NULL) {
                if (!std::strcmp(token, "curNodeHostname")) {
                    token = strtok(NULL, "\"\t ,}:{\n");
                    curNodeHostname_ = token;
                } else if (!std::strcmp(token, "curNodeIp")) {
                    token = strtok(NULL, "\"\t ,}:{\n");
                    curNodeIp_ = token;
                } else if (!std::strcmp(token, "deviceName")) {
                    token = strtok(NULL, "\"\t ,}:{\n");
                    deviceNames_ = token;
#ifdef XILINX_FUZZYMATCH_DEBUG_ON
                    std::cout << "    deviceNames_=" << deviceNames_ << std::endl;
#endif
                } else if (!std::strcmp(token, "xGraphStore")) {
                    token = strtok(NULL, "\"\t ,}:{\n");
                    xGraphStorePath_ = token;
                } else if (!std::strcmp(token, "numDevices")) {
                    token = strtok(NULL, "\"\t ,}:{\n");
                    numDevices_ = atoi(token);
#ifdef XILINX_FUZZYMATCH_DEBUG_ON
                    std::cout << "numDevices=" << numDevices_ << std::endl;
#endif
                } else if (!std::strcmp(token, "nodeIps")) {
                    // this field has multipe space separated IPs
                    scanNodeIp = true;
                    // read the next token
                    token = strtok(NULL, "\"\t ,}:{\n");
                    nodeIps_ += token;
#ifdef XILINX_FUZZYMATCH_DEBUG_ON
                    std::cout << "nodeIps_=" << nodeIps_ << std::endl;
#endif
                } else if (scanNodeIp) {
                    // In the middle of nodeIps field
                    nodeIps_ += " ";
                    nodeIps_ += token;
#ifdef XILINX_FUZZYMATCH_DEBUG_ON
                    std::cout << "nodeIps_=" << nodeIps_ << std::endl;
#endif
                }
                token = strtok(NULL, "\"\t ,}:{\n");
            }
        }
        config_json.close();
        if (deviceNames_ == "xilinx_u50_gen3x16_xdma_201920_3") {
            xclbinPath_ = PLUGIN_XCLBIN_PATH_U50;
        } else if (deviceNames_ == "xilinx_aws-vu9p-f1_shell-v04261818_201920_2") {
            xclbinPath_ = PLUGIN_XCLBIN_PATH_AWSF1;
        }
    }
    
    ~Context() { delete pFuzzyMatch_; }

    xilinx_apps::fuzzymatch::FuzzyMatch *getFuzzyMatchObj() 
    {
#ifdef XILINX_FUZZYMATCH_DEBUG_ON
        std::cout << "DEBUG: " << __FUNCTION__ << std::endl;
#endif
        if (fuzzyMatchObjIsModified_) {
#ifdef XILINX_FUZZYMATCH_DEBUG_ON
            std::cout << "DEBUG: fuzzymach options changed.  Deleting old fuzzymatch object (if it exists)." << std::endl;
#endif
            delete pFuzzyMatch_;
            pFuzzyMatch_ = nullptr;
            fuzzyMatchObjIsModified_ = false;
        }
        
        if (pFuzzyMatch_ == nullptr) {
            xilinx_apps::fuzzymatch::Options options;
            options.xclbinPath = xclbinPath_;
            options.deviceNames = deviceNames_;

#ifdef XILINX_FUZZYMATCH_DEBUG_ON
            std::cout << "DEBUG: fuzzymatch options:"
                    << "\n    xclbinPath=" << options.xclbinPath
                    << "\n    deviceNames=" << options.deviceNames
                    << std::endl;
#endif
            pFuzzyMatch_ = new xilinx_apps::fuzzymatch::FuzzyMatch(options);
        }
        
        return pFuzzyMatch_;
    }

    void setCurNodeIp(std::string curNodeIp) {
        std::cout << "DEBUG: " << __FUNCTION__ << " curNodeIp=" << curNodeIp << std::endl;
        curNodeIp_ = curNodeIp;
    }
    std::string getCurNodeIp() { return curNodeIp_; }

    void setNodeIps(std::string nodeIps) {
        std::cout << "DEBUG: " << __FUNCTION__ << " nodeIps=" << nodeIps << std::endl;
        nodeIps_ = nodeIps;
    }
    std::string getNodeIps() { return nodeIps_; }

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
   
    void setXGraphStorePath(const std::string& path){
        xGraphStorePath_= path;
    }

    const std::string &getXGraphStorePath() const {
        return xGraphStorePath_;
    }
};

}  // namespace xilFuzzyMatch

#include "fuzzymatch_loader.cpp"
#include "fuzzymatch_utils.cpp"

#endif /* XILINX_FUZZYMATCH_IMPL_HPP */


