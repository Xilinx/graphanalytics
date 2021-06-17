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

#include <vector>
#include <map>
#include <fstream>

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

using LouvainMod = xilinx_apps::louvainmod::LouvainMod;

class Context {
public:
    enum State {
        UninitializedState = 0,  // no state-updating function has been called
        CalledExecuteLouvainState,  // after execute called
        NumStates
    };


private:
    std::string alveoProject_;
    unsigned numPartitions_;
    unsigned numDevices_ = 1;
    std::string curNodeIp_;
    std::string nodeIps_;
    unsigned nodeId_ = 0;
    unsigned numNodes_ = 1;
    State state_ = UninitializedState;
    LouvainMod *pLouvainMod_ = nullptr;
    std::vector<long> degree_list;//long* offsets_tg; //travel and add i-1 to i to build the offset_tg
    std::vector<int> numAlveoPartitions;

    uint64_t nextId_ = 0 ; // can be used as partition size after traverse the graph done
    uint64_t louvain_offset = 0 ;
    // Partition data
    long* offsets_tg;
    xilinx_apps::louvainmod::Edge* edgelist_tg;
    long* drglist_tg;
    long  start_vertex;     // If a vertex is smaller than star_vertex, it is a ghost
    long  end_vertex;


    // use the below to store the information and build final partition data
    //key-> value : louvainId->set of graphEdge
    std::map<uint64_t, std::vector<xilinx_apps::louvainmod::Edge>> edgeListMap;
    std::map<uint64_t, long> dgrListMap;
    std::vector<xilinx_apps::louvainmod::Edge> edgeListVec;
    std::vector<long> dgrListVec;


public:


    static Context *getInstance() {
        static Context *s_pContext = nullptr;
        if (s_pContext == nullptr)
            s_pContext = new Context();
        return s_pContext;
    }
    
    Context() = default;
    ~Context() { delete pLouvainMod_; }

    xilinx_apps::louvainmod::LouvainMod *getLouvainModObj() {
        if (pLouvainMod_ == nullptr) {
            xilinx_apps::louvainmod::Options options;

            std::string cur_node_hostname, cur_node_ip, node_ips, xGstorePath;
            // PLUGIN_CONFIG_PATH will be replaced by the actual config path during plugin installation
            std::fstream config_json(PLUGIN_CONFIG_PATH, std::ios::in);
            if (!config_json) {
                std::cout << "ERROR: config file doesn't exist:" << PLUGIN_CONFIG_PATH << std::endl;
                return(2);
            }

            char line[1024] = {0};
            char* token;
            node_ips = "";
            bool scanNodeIp;
            while (config_json.getline(line, sizeof(line))) {
                token = strtok(line, "\"\t ,}:{\n");
                scanNodeIp = false;
                while (token != NULL) {
                    if (!std::strcmp(token, "curNodeHostname")) {
                        token = strtok(NULL, "\"\t ,}:{\n");
                        cur_node_hostname = token;
                    } else if (!std::strcmp(token, "curNodeIp")) {
                        token = strtok(NULL, "\"\t ,}:{\n");
                        cur_node_ip = token;
                    } else if (!std::strcmp(token, "xGraphStore")) {
                        token = strtok(NULL, "\"\t ,}:{\n");
                        xGstorePath = token;
                    } else if (!std::strcmp(token, "nodeIps")) {
                        // this field has multipe space separated IPs
                        scanNodeIp = true;
                        // read the next token
                        token = strtok(NULL, "\"\t ,}:{\n");
                        node_ips += token;
                        std::cout << "node_ips=" << node_ips << std::endl;
                    } else if (scanNodeIp) {
                        // In the middle of nodeIps field
                        node_ips += " ";
                        node_ips += token;
                        std::cout << "node_ips=" << node_ips << std::endl;
                    }
                    token = strtok(NULL, "\"\t ,}:{\n");
                }
            }
            config_json.close();

            options.xclbinPath = PLUGIN_XCLBIN_PATH;
            options.nameProj = alveoProject_;
            options.devNeed_cmd = numNodes_;
            options.nodeId = nodeId_;
            options.hostName = cur_node_hostname;
            options.clusterIpAddresses = node_ips;
            options.hostIpAddress = cur_node_ip;

#ifdef XILINX_COM_DETECT_DEBUG_ON
            std::cout << "DEBUG: louvainmod options: = xclbinPath" << options.xclbinPath
                    << ", nameProj=" << options.nameProj
                    << ", devNeed_cmd=" << options.devNeed_cmd
                    << ", nodeId=" <<options.nodeId
                    << ", hostName=" << options.hostName
                    << ", clusterIpAddresses=" << options.clusterIpAddresses
                    << ", hostIpAddress=" << options.hostIpAddress <<std::endl;
#endif
            pLouvainMod_= new xilinx_apps::louvainmod::LouvainMod(options);
            
        }
        
        return pLouvainMod_;
    }


    void setAlveoProject(std::string alveoProject) {
        std::cout << "DEBUG: " << __FUNCTION__ << " AlveoProject=" << alveoProject << std::endl;
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
    
    void addDegreeList(long p_degree){
        degree_list.push_back(p_degree); 
    }
    
    int getDegreeListSize(){
        return degree_list.size(); 
    }
    
    std::vector<long> getDegreeList(){
        return degree_list; 
    }
    
    void clearPartitionData(){
        nextId_ = 0 ;
        degree_list.clear();
        edgeListMap.clear();
        dgrListMap.clear();
        edgeListVec.clear();
        dgrListVec.clear();  
    }
    
    void clear() {
        state_ = UninitializedState;
    }

    std::map<uint64_t, long>& getDgrListMap() {
        return dgrListMap;
    }

    void setDgrListMap(std::map<uint64_t, long> dgrListMap) {
        this->dgrListMap = dgrListMap;
    }


    const std::vector<long>& getDgrListVec() {
        return dgrListVec;
    }

    void setDgrListVec(const std::vector<long>& dgrListVec) {
        this->dgrListVec = dgrListVec;
    }

    void addDgrListVec(const long dgr) {
        this->dgrListVec.push_back(dgr);
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

    std::map<uint64_t, std::vector<xilinx_apps::louvainmod::Edge> >& getEdgeListMap() {
        return edgeListMap;
    }

    void setEdgeListMap(
            std::map<uint64_t, std::vector<xilinx_apps::louvainmod::Edge> >& edgeListMap) {
        this->edgeListMap = edgeListMap;
    }

    void addEdgeListMap(
            uint64_t s, xilinx_apps::louvainmod::Edge e) {
        this->edgeListMap[s].push_back(e);
    }

    std::vector<xilinx_apps::louvainmod::Edge>& getEdgeListVec() {
        return edgeListVec;
    }

    void setEdgeListVec(
            const std::vector<xilinx_apps::louvainmod::Edge>& edgeListVec) {
        this->edgeListVec = edgeListVec;
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
};

}  // namespace xilComDetect

#include "louvainmod_loader.cpp"

#endif /* XILINX_COM_DETECT_IMPL_HPP */


