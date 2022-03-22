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
#include <unordered_map>

namespace xilMis {

using Mutex = std::mutex;

//#define XILINX_MIS_DEBUG_MUTEX

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
        pMis_ = nullptr;
        pGraph_ = nullptr;

        misObjIsModified_ = true;
        misGraphIsModified_ = true;

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
#ifdef XILINX_MIS_DEBUG_ON
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
#ifdef XILINX_MIS_DEBUG_ON
                    std::cout << "    deviceNames_=" << deviceNames_ << std::endl;
#endif
                } else if (!std::strcmp(token, "xGraphStore")) {
                    token = strtok(NULL, "\"\t ,}:{\n");
                    xGraphStorePath_ = token;
                } else if (!std::strcmp(token, "numDevices")) {
                    token = strtok(NULL, "\"\t ,}:{\n");
                    numDevices_ = atoi(token);
#ifdef XILINX_MIS_DEBUG_ON
                    std::cout << "numDevices=" << numDevices_ << std::endl;
#endif
                } else if (!std::strcmp(token, "nodeIps")) {
                    // this field has multipe space separated IPs
                    scanNodeIp = true;
                    // read the next token
                    token = strtok(NULL, "\"\t ,}:{\n");
                    nodeIps_ += token;
#ifdef XILINX_MIS_DEBUG_ON
                    std::cout << "nodeIps_=" << nodeIps_ << std::endl;
#endif
                } else if (scanNodeIp) {
                    // In the middle of nodeIps field
                    nodeIps_ += " ";
                    nodeIps_ += token;
#ifdef XILINX_MIS_DEBUG_ON
                    std::cout << "nodeIps_=" << nodeIps_ << std::endl;
#endif
                }
                token = strtok(NULL, "\"\t ,}:{\n");
            }
        }
        config_json.close();

        // set xclbinPath
        if (deviceNames_ == "xilinx_u50_gen3x16_xdma_201920_3") {
            xclbinPath_ = PLUGIN_XCLBIN_PATH_U50;
        } else if (deviceNames_ == "xilinx_u55c_gen3x16_xdma_base_2") {
            xclbinPath_ = PLUGIN_XCLBIN_PATH_U55C;
        }
    }

    ~Context() { delete pGraph_; delete pMis_; }

    static Context *getContext() {
        static Context *l_context = nullptr;
        if (l_context == nullptr)
            l_context = new Context();
        return l_context;
    }

    void addVertexToMap( int x ) { v_id_map[vid_] = x; }
    int getNextVid() { return vid_++; }
    void addRowPtrEntry( int x ) { rowPtr_.push_back( rowPtr_[row_id_++] + x ); }
    void addColIdxEntry( int x ) { colIdx_.push_back( x ); }
    std::string getXclbinPath() { return xclbinPath_; }
    std::string getDeviceNames() { return deviceNames_; }

    std::vector<int>& getRowPtr() { return rowPtr_; }
    std::vector<int>& getColIdx() { return colIdx_; }

    void resetContext() {
        // clear variables
        vid_ = 0;
        row_id_ = 0;
        rowPtr_.clear();
        colIdx_.clear();
        v_id_map.clear();

        // add default zero entry
        rowPtr_.push_back(0);

        // force recreate MIS Object
        misGraphIsModified_ = true;
    }

    xilinx_apps::mis::MIS *getMisObj() {

        if (misObjIsModified_) {
#ifdef XILINX_MIS_DEBUG_ON
            std::cout << "DEBUG: mis options changed.  Deleting old mis object (if it exists)." << std::endl;
#endif
            delete pGraph_;
            delete pMis_;
            pMis_ = nullptr;
            misObjIsModified_ = false;
            misGraphIsModified_ = true;
        }

        if (pMis_ == nullptr) {
            // set MIS options
            xilinx_apps::mis::Options options;
            options.xclbinPath = getXclbinPath();
            options.deviceNames = getDeviceNames();

#ifdef XILINX_MIS_DEBUG_ON
            std::cout << "DEBUG: mis options:"
                      << "\n    xclbinPath=" << options.xclbinPath
                      << "\n    deviceNames=" << options.deviceNames
                      << std::endl;
#endif

            // create MIS object
            pMis_ = new xilinx_apps::mis::MIS(options);
            // start MIS, program xclbin: one time operation
            pMis_->startMis();
        }

        return pMis_;
    }

    void setMisGraph() {
        if (misGraphIsModified_) {
            // create graph object
            pGraph_ = new xilinx_apps::mis::GraphCSR(std::move(rowPtr_), std::move(colIdx_));
            // set MIS graph
            pMis_->setGraph(pGraph_);

            misGraphIsModified_ = false;
        }
    }

    std::unordered_map<int, int> v_id_map;

private:
    int vid_;
    int row_id_;
    std::vector<int> rowPtr_;
    std::vector<int> colIdx_;
    xilinx_apps::mis::MIS *pMis_;
    xilinx_apps::mis::GraphCSR *pGraph_;

    std::string deviceNames_ = "xilinx_u50_gen3x16_xdma_201920_3";
    std::string xclbinPath_;
    uint32_t numDevices_ = 1;
    std::string nodeIps_;
    std::string curNodeHostname_;
    std::string curNodeIp_;
    std::string xGraphStorePath_;
    bool misObjIsModified_;
    bool misGraphIsModified_;


};

} /* namespace xilMis */

#include "mis_loader.cpp"

#endif /* XILINX_MIS_IMPL_HPP */