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

// Enable this to dump graph vertices and edges, as seen by the partitioning logic
//#define XILINX_COM_DETECT_DUMP_GRAPH

// Enable this to dump an .mtx file of the graph, as seen by the partitioning logic
//#define XILINX_COM_DETECT_DUMP_MTX

#include <vector>
#include <map>
#include <fstream>

namespace xilComDetect {

using Mutex = std::mutex;

#define XILINX_COM_DETECT_DEBUG_MUTEX

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

using LouvainMod = xilinx_apps::louvainmod::LouvainMod;
using PartitionNameMode = xilinx_apps::louvainmod::PartitionNameMode;

struct LouvainVertex {
    uint64_t id_ = 0;  // compressed ID
    long outDegree_ = 0;
    
    LouvainVertex() = default;
    LouvainVertex(long outDegree) : outDegree_(outDegree) {}
};

class Partitions{
public:

      std::vector<long> mEdgePtrVec;
      std::vector<long> degree_list;
      std::vector<xilinx_apps::louvainmod::Edge> mEdgeVec;
      std::vector<long> mDgrVec;
      std::vector<long> addedOffset; //store each vertex edgelist offset
      std::map<uint64_t, LouvainVertex> vertexMap;  // maps from original (.mtx) vertex ID to properties of the vertex
      void clearPartitionData(){

          degree_list.clear();
          mEdgeVec.clear();
          mEdgePtrVec.clear();
          mDgrVec.clear();
          addedOffset.clear();
          vertexMap.clear();
      }

      ~Partitions() {clearPartitionData();};

};
class Context {
public:
    enum State {
        UninitializedState = 0,  // no state-updating function has been called
        CalledExecuteLouvainState,  // after execute called
        NumStates
    };


private:
    bool louvainModObjIsModified_ = false;  // true if a parameter has changed that requires a rebuild of
                                            // the LouvainMod object
    std::string curNodeIp_;
    std::string nodeIps_;
    std::string xGraphStorePath_;
    std::string xclbinPath_;
    std::string alveoProject_;
    unsigned numPartitions_;
    unsigned numDevices_ = 1;
    PartitionNameMode partitionNameMode_ = PartitionNameMode::Auto;

    unsigned nodeId_ = 0;
    unsigned numNodes_ = 1;
    State state_ = UninitializedState;
    LouvainMod *pLouvainMod_ = nullptr;

    //long* offsets_tg; //travel and add i-1 to i to build the offset_tg
    std::vector<int> numAlveoPartitions;

    uint64_t nextId_ = 0 ; // can be used as partition size after traverse the graph done
    uint64_t louvain_offset = 0 ;
    // Partition data
    long* offsets_tg;
    const xilinx_apps::louvainmod::Edge* edgelist_tg;
    long* drglist_tg;
    long  start_vertex;     // If a vertex is smaller than star_vertex, it is a ghost
    long  end_vertex;
 
public:
    std::string curNodeHostname_;
    std::string deviceNames_ = "xilinx_u50_gen3x16_xdma_201920_3";
    Partitions* curParPtr=nullptr;

#ifdef XILINX_COM_DETECT_DUMP_MTX
    std::ofstream mtxFstream;
#endif

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
#ifdef XILINX_COM_DETECT_DEBUG_ON
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
#ifdef XILINX_COM_DETECT_DEBUG_ON
                    std::cout << "    deviceNames_=" << deviceNames_ << std::endl;
#endif
                } else if (!std::strcmp(token, "xGraphStore")) {
                    token = strtok(NULL, "\"\t ,}:{\n");
                    xGraphStorePath_ = token;
                } else if (!std::strcmp(token, "numDevices")) {
                    token = strtok(NULL, "\"\t ,}:{\n");
                    numDevices_ = atoi(token);
#ifdef XILINX_COM_DETECT_DEBUG_ON
                    std::cout << "numDevices=" << numDevices_ << std::endl;
#endif
                } else if (!std::strcmp(token, "nodeIps")) {
                    // this field has multipe space separated IPs
                    scanNodeIp = true;
                    // read the next token
                    token = strtok(NULL, "\"\t ,}:{\n");
                    nodeIps_ += token;
#ifdef XILINX_COM_DETECT_DEBUG_ON
                    std::cout << "node_ips=" << nodeIps_ << std::endl;
#endif
                } else if (scanNodeIp) {
                    // In the middle of nodeIps field
                    nodeIps_ += " ";
                    nodeIps_ += token;
#ifdef XILINX_COM_DETECT_DEBUG_ON
                    std::cout << "node_ips=" << nodeIps_ << std::endl;
#endif
                }
                token = strtok(NULL, "\"\t ,}:{\n");
            }
        }
        config_json.close();
        if (deviceNames_ == "xilinx_u50_gen3x16_xdma_201920_3")
            xclbinPath_ = PLUGIN_XCLBIN_PATH;
        else if (deviceNames_ == "xilinx_u55c_gen3x16_xdma_base_2")
            xclbinPath_ = PLUGIN_XCLBIN_PATH_U55C;
    }
    
    ~Context() { delete pLouvainMod_; }

    xilinx_apps::louvainmod::LouvainMod *getLouvainModObj() {
#ifdef XILINX_COM_DETECT_DEBUG_ON
        std::cout << "DEBUG: " << __FUNCTION__ << std::endl;
#endif
        if (louvainModObjIsModified_) {
#ifdef XILINX_COM_DETECT_DEBUG_ON
            std::cout << "DEBUG: louvainmod options changed.  Deleting old louvainmod object (if it exists)."
                << std::endl;
#endif
            delete pLouvainMod_;
            pLouvainMod_ = nullptr;
            louvainModObjIsModified_ = false;
        }
        
        if (pLouvainMod_ == nullptr) {
            xilinx_apps::louvainmod::Options options;
            options.xclbinPath = xclbinPath_;
            options.nameProj = alveoProject_;
            options.numDevices = numDevices_;
            options.deviceNames = deviceNames_;
            options.nodeId = nodeId_;
            options.hostName = curNodeHostname_;
            options.clusterIpAddresses = nodeIps_;
            options.hostIpAddress = curNodeIp_;
            options.partitionNameMode = partitionNameMode_;

#ifdef XILINX_COM_DETECT_DEBUG_ON
            std::cout << "DEBUG: louvainmod options:"
                    << "\n    xclbinPath=" << options.xclbinPath
                    << "\n    nameProj=" << options.nameProj
                    << "\n    numDevices=" << options.numDevices
                    << "\n    deviceNames=" << options.deviceNames
                    << "\n    nodeId=" << options.nodeId
                    << "\n    hostName=" << options.hostName
                    << "\n    clusterIpAddresses=" << options.clusterIpAddresses
                    << "\n    hostIpAddress=" << options.hostIpAddress <<std::endl;
#endif
            pLouvainMod_= new xilinx_apps::louvainmod::LouvainMod(options);
        }
        
        return pLouvainMod_;
    }


    void setAlveoProject(std::string alveoProject) {
        std::cout << "DEBUG: " << __FUNCTION__ << " AlveoProject=" << alveoProject << std::endl;
        std::string newAlveoProject = this->getXGraphStorePath() + "/" + alveoProject;
        if (newAlveoProject != alveoProject_)
            louvainModObjIsModified_ = true;
        alveoProject_ = newAlveoProject;
    }

    void setAlveoProjectRaw(std::string alveoProject) {
        std::cout << "DEBUG: " << __FUNCTION__ << " AlveoProject=" << alveoProject << std::endl;
        if (alveoProject != alveoProject_)
            louvainModObjIsModified_ = true;
        alveoProject_ = alveoProject;
    }
    std::string getAlveoProject() { return alveoProject_; }

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

    void setNumPartitions(unsigned numPartitions) {
        std::cout << "DEBUG: " << __FUNCTION__ << " numPartitions=" << numPartitions << std::endl;
        numPartitions_ = numPartitions;
    }
    unsigned getNumPartitions() { return numPartitions_; }

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
    
    PartitionNameMode getPartitionNameMode() const { return partitionNameMode_; }
    void setPartitionNameMode(PartitionNameMode partitionNameMode) {
        if (partitionNameMode != partitionNameMode_)
            louvainModObjIsModified_ = true;
        partitionNameMode_ = partitionNameMode;
    }

    void clearPartitionData(){
        nextId_ = 0 ;

    }
    
    void clear() {
        state_ = UninitializedState;
    }

    long *getDrglistTg() {
        return drglist_tg;
    }

    void setDrglistTg(long * drglistTg) {
        drglist_tg = drglistTg;
    }

    const xilinx_apps::louvainmod::Edge* getEdgelistTg() {
        return edgelist_tg;
    }

    void setEdgelistTg(const xilinx_apps::louvainmod::Edge* edgelistTg) {
        edgelist_tg = edgelistTg;
    }

    long getEndVertex() {
        return end_vertex;
    }

    void setEndVertex(long endVertex) {
        end_vertex = endVertex;
    }

    uint64_t getLouvainOffset() const {
        return louvain_offset;
    }


    void setLouvainOffset(uint64_t louvainOffset = 0) {
        louvain_offset = louvainOffset;
    }

    uint64_t getNextId() const {
        return nextId_;
    }

    void setNextId(uint64_t nextId = 0) {
        nextId_ = nextId;
    }

    void clearNumAlveoPartitions() {
        numAlveoPartitions.clear();
    }
    
    const std::vector<int>& getNumAlveoPartitions() {
        return numAlveoPartitions;
    }

    void setNumAlveoPartitions(const std::vector<int>& numAlveoPartitions) {
        this->numAlveoPartitions = numAlveoPartitions;
    }

    void addNumAlveoPartitions(const int num) {
        this->numAlveoPartitions.push_back(num);
    }

    long *getOffsetsTg() const {
        return offsets_tg;
    }

    void setOffsetsTg(long * offsetsTg) {
        offsets_tg = offsetsTg;
    }

    long getOffsetTg(int idx) const {
          return offsets_tg[idx];
      }

    void setOffsetsTg(int idx, long offsetsTg) {
        offsets_tg[idx] = offsetsTg;
    }


    long getStartVertex() const {
        return start_vertex;
    }

    void setStartVertex(long startVertex) {
        start_vertex = startVertex;
    }
    
    void setXGraphStorePath(const std::string& path){
        xGraphStorePath_= path;
    }

    const std::string &getXGraphStorePath() const {
        return xGraphStorePath_;
    }
};

}  // namespace xilComDetect

#include "louvainmod_loader.cpp"

#endif /* XILINX_COM_DETECT_IMPL_HPP */


