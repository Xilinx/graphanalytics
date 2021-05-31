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
 * WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "graph.hpp"
#include "xf_graph_L3.hpp"
#include <vector>
#include <cstdint>
#include <string>
#include <dlfcn.h>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <cstring>
#include <unordered_map>
#include <memory>

using int64_t = std::int64_t;
using int32_t = std::int32_t;
using VERTEX = int64_t;

template <typename T>
class ListAccum : public std::vector<T> {
public:
    T &get(unsigned index) { return std::vector<T>::at(index); }
    const T &get(unsigned index) const { return std::vector<T>::at(index); }
    ListAccum &operator+=(const T &val) {
        std::vector<T>::push_back(val);
        return *this;
    }
};

struct XilCosinesimMatch {
    int64_t index_ = 0;
    float similarity_ = 0.0;
    
    XilCosinesimMatch() = default;
    XilCosinesimMatch(VERTEX index, float similarity) : index_(index), similarity_(similarity) {}
};

const int XF_GRAPH_UDF_GRAPH_PARTITION_ERROR = -1;

//#####################################################################################################################

namespace xai {

using Mutex = std::mutex;
using Lock = std::lock_guard<Mutex>;

static Mutex &getMutex() {
    static Mutex *pMutex = nullptr;
    if (pMutex == nullptr)
        pMutex = new Mutex();
    return *pMutex;
}

std::vector<uint64_t> IDMap;

}

//#####################################################################################################################

const std::string TIGERGRAPH_PATH = "/scratch/tigergraph/tigergraph/app/3.1.0";


// From L3_wrapper.cpp

class sharedHandlesCosSimDense {
   public:
    std::unordered_map<unsigned int, std::shared_ptr<xf::graph::L3::Handle> > handlesMap;
    static sharedHandlesCosSimDense& instance() {
        static sharedHandlesCosSimDense theInstance;
        return theInstance;
    }
};

extern "C" int load_graph_cosinesim_ss_dense_fpga_wrapper(uint32_t numDevices,
                                                  uint32_t cuNm,
                                                  xf::graph::Graph<int32_t, int32_t>** g) {
    //----------------- Text Parser --------------------------
    std::string opName;
    std::string kernelName;
    int requestLoad = 100;
    std::string xclbinPath;

    std::string basePath = TIGERGRAPH_PATH;
    std::string jsonFilePath = basePath + "/dev/gdk/gsql/src/QueryUdf/config_cosinesim_ss_dense_fpga.json";
    std::fstream userInput(jsonFilePath, std::ios::in);
    if (!userInput) {
        std::cout << "Error : config file " << jsonFilePath << " doesn't exist !" << std::endl;
        return XF_GRAPH_L3_ERROR_CONFIG_FILE_NOT_EXIST;
    }
    char line[1024] = {0};
    char* token;
    while (userInput.getline(line, sizeof(line))) {
        token = strtok(line, "\"\t ,}:{\n");
        while (token != NULL) {
            if (!std::strcmp(token, "operationName")) {
                token = strtok(NULL, "\"\t ,}:{\n");
                opName = token;
            } else if (!std::strcmp(token, "kernelName")) {
                token = strtok(NULL, "\"\t ,}:{\n");
                kernelName = token;
            } else if (!std::strcmp(token, "requestLoad")) {
                token = strtok(NULL, "\"\t ,}:{\n");
                requestLoad = std::atoi(token);
            } else if (!std::strcmp(token, "xclbinPath")) {
                token = strtok(NULL, "\"\t ,}:{\n");
                std::string tmpStr = token;
                xclbinPath = tmpStr;
            } 
            token = strtok(NULL, "\"\t ,}:{\n");
        }
    }
    userInput.close();

    //----------------- Setup denseSimilarityKernel thread ---------
    xf::graph::L3::Handle::singleOP op0;
    op0.operationName = (char*)opName.c_str();
    op0.setKernelName((char*)kernelName.c_str());
    op0.requestLoad = requestLoad;
    op0.xclbinPath = xclbinPath;
    op0.numDevices = numDevices;
    op0.cuPerBoard = cuNm;
    
    std::fstream xclbinFS(xclbinPath, std::ios::in);
    if (!xclbinFS) {
        std::cout << "Error : xclbinFile doesn't exist: " << xclbinPath << std::endl;
        return XF_GRAPH_L3_ERROR_XCLBIN_FILE_NOT_EXIST;
    }

    std::shared_ptr<xf::graph::L3::Handle> handleInstance(new xf::graph::L3::Handle);
    sharedHandlesCosSimDense::instance().handlesMap[0] = handleInstance;
    std::shared_ptr<xf::graph::L3::Handle> handle0 = sharedHandlesCosSimDense::instance().handlesMap[0];

    handle0->addOp(op0);
    int status = handle0->setUp();
    if (status < 0)
        return status;

    //-------------------------
    std::shared_ptr<xf::graph::L3::Handle> handle1 = sharedHandlesCosSimDense::instance().handlesMap[0];
    handle1->showHandleInfo(); // no-op in Release build
    //------------------------
    
    //---------------- Run Load Graph -----------------------------------
    for (uint32_t i = 0; i < numDevices * cuNm; ++i) {
        std::cout << "DEBUG: loadGraphMultiCardNonBlocking " << i << std::endl;
        (handle0->opsimdense)->loadGraphMultiCardNonBlocking(i / cuNm, i % cuNm, g[i][0]);
    }

    //--------------- Free and delete -----------------------------------

    for (uint32_t i = 0; i < numDevices * cuNm; ++i) {
        g[i]->freeBuffers();
        delete[] g[i]->numEdgesPU;
        delete[] g[i]->numVerticesPU;
    }
    delete[] g;
    
    return 0;
}

extern "C" void cosinesim_ss_dense_fpga(uint32_t devicesNeeded,
                                        int32_t sourceLen,
                                        int32_t* sourceWeight,
                                        int32_t topK,
                                        xf::graph::Graph<int32_t, int32_t>** g,
                                        int32_t* resultID,
                                        float* similarity) {
    //---------------- Run Load Graph -----------------------------------
//    std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__
//              << " XRT_INI_PATH=" << std::getenv("XRT_INI_PATH") << std::endl;

    std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time =
            std::chrono::high_resolution_clock::now();
    std::cout << "DEBUG: " << __FUNCTION__ << " start=" << l_start_time.time_since_epoch().count() 
              << std::endl;

    std::shared_ptr<xf::graph::L3::Handle> handle0 = 
                        sharedHandlesCosSimDense::instance().handlesMap[0];
    handle0->showHandleInfo(); // no-op in Release build
    int32_t requestNm = 1;
    int32_t hwNm = devicesNeeded;
    std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__ 
              << " hwNm=" << hwNm << std::endl;
    std::vector<xf::graph::L3::event<int> > eventQueue[requestNm];
    float** similarity0[requestNm];
    int32_t** resultID0[requestNm];
    int counter[requestNm][hwNm];
    for (int m = 0; m < requestNm; ++m) {
        similarity0[m] = new float*[hwNm];
        resultID0[m] = new int32_t*[hwNm];
        for (int i = 0; i < hwNm; ++i) {
            counter[m][i] = 0;
            similarity0[m][i] = xf::graph::internal::aligned_alloc<float>(topK);
            resultID0[m][i] = xf::graph::internal::aligned_alloc<int32_t>(topK);
            memset(resultID0[m][i], 0, topK * sizeof(int32_t));
            memset(similarity0[m][i], 0, topK * sizeof(float));
        }
    }
    //---------------- Run L3 API -----------------------------------
    for (int m = 0; m < requestNm; ++m) {
        eventQueue[m] = xf::graph::L3::cosineSimilaritySSDenseMultiCard(
            *handle0, hwNm, sourceLen, sourceWeight, topK, g,
            resultID0[m], similarity0[m]);
    }

    int ret = 0;
    for (int m = 0; m < requestNm; ++m) {
        for (unsigned int i = 0; i < eventQueue[m].size(); ++i) {
            ret += eventQueue[m][i].wait();
        }
    }
    for (int m = 0; m < requestNm; ++m) {
        for (int i = 0; i < topK; ++i) {
            similarity[i] = similarity0[m][0][counter[m][0]];
            int32_t prev = 0;
            resultID[i] = resultID0[m][0][counter[m][0]];
            counter[m][0]++;
            for (int j = 1; j < hwNm; ++j) {
                if (similarity[i] < similarity0[m][j][counter[m][j]]) {
                    similarity[i] = similarity0[m][j][counter[m][j]];
                    resultID[i] = resultID0[m][j][counter[m][j]];
                    counter[m][prev]--;
                    prev = j;
                    counter[m][j]++;
                }
            }
        }
    }

    for (int m = 0; m < requestNm; ++m) {
        for (int i = 0; i < hwNm; ++i) {
            free(similarity0[m][i]);
            free(resultID0[m][i]);
        }
        delete[] similarity0[m];
        delete[] resultID0[m];
    }
    std::chrono::time_point<std::chrono::high_resolution_clock> l_end_time =
            std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = l_end_time - l_start_time;
    double l_timeMs = l_durationSec.count() * 1e3;
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " runtime msec=  " << std::fixed << std::setprecision(6) 
              << l_timeMs << std::endl;
}


//#####################################################################################################################



inline int udf_load_graph_cosinesim_ss_fpga(int64_t numVertices,
                                            int64_t vecLength,
                                            ListAccum<ListAccum<int64_t> >& oldVectors,
                                            int devicesNeeded) 
{
    xai::Lock lock(xai::getMutex());
    xai::IDMap.clear();
    ListAccum<XilCosinesimMatch> result;
    int32_t numEdges = vecLength - 3;

    // kernel has 3 PUs, the input data should be splitted into 4 parts
    const int splitNm = 3;    
    const int channelsPU = 4; // each PU has 4 HBM channels
    const int cuNm = 2;
    const int channelW = 16;
    // value to use for padding.  0 appears to be safe for cosine sim computation
    const int32_t nullVal = 0;  

    int32_t edgeAlign8 = ((numEdges + channelW - 1) / channelW) * channelW;
    // All channels need to have equal number of vertices except the last channel
    // The formula is to make sure the last channel always has data to process.
    // The last chanel may be assigned more data than other channels.
    int general = ((numVertices - (devicesNeeded * cuNm * splitNm * channelsPU + 1)) /
                   (devicesNeeded * cuNm * splitNm * channelsPU)) * channelsPU;
    int rest = numVertices - general * (devicesNeeded * cuNm * splitNm - 1);
    std::cout << "DEBUG: " << __FUNCTION__
              << " numVertices=" << numVertices << ", general=" << general 
              << ", Vertices PU 0-" << devicesNeeded*cuNm*splitNm-2 << ": "
              << general << ", total=" << general*(devicesNeeded*cuNm*splitNm-1)
              << ", last PU " << devicesNeeded*cuNm*splitNm-1 << ": " << rest
              << std::endl;

    if (rest < 0) {
        return XF_GRAPH_UDF_GRAPH_PARTITION_ERROR;
    }
    int32_t** numVerticesPU = new int32_t*[devicesNeeded * cuNm]; // vertex numbers in each PU
    int32_t** numEdgesPU = new int32_t*[devicesNeeded * cuNm];    // edge numbers in each PU

    //int tmpID[devicesNeeded * cuNm * channelsPU * splitNm];
    for (int i = 0; i < devicesNeeded * cuNm; ++i) {
        numVerticesPU[i] = new int32_t[splitNm];
        numEdgesPU[i] = new int32_t[splitNm];
        for (int j = 0; j < splitNm; ++j) {
            numEdgesPU[i][j] = numEdges;
            for (int k = 0; k < channelsPU; ++k) {
                //tmpID[i * splitNm * channelsPU + j * channelsPU + k] = 0;
            }
        }
    }
    //---------------- setup number of vertices in each PU ---------
    for (int i = 0; i < devicesNeeded * cuNm; ++i) {
        for (int j = 0; j < splitNm; ++j) {
            numVerticesPU[i][j] = general;
        }
    }
    numVerticesPU[devicesNeeded * cuNm - 1][splitNm - 1] = rest;

    xf::graph::Graph<int32_t, int32_t>** g = 
        new xf::graph::Graph<int32_t, int32_t>*[devicesNeeded * cuNm];
    int fpgaNodeNm = 0;
    for (int i = 0; i < devicesNeeded * cuNm; ++i) {
        g[i] = new xf::graph::Graph<int32_t, int32_t>("Dense", 4 * splitNm, numEdges, numVerticesPU[i]);
        g[i][0].numEdgesPU = new int32_t[splitNm];
        g[i][0].numVerticesPU = new int32_t[splitNm];
        g[i][0].edgeNum = numEdges;
        g[i][0].nodeNum = numVertices;
        g[i][0].splitNum = splitNm;
        g[i][0].refID = fpgaNodeNm;
        for (int j = 0; j < splitNm; ++j) {
            fpgaNodeNm += numVerticesPU[i][j];
            int depth = ((numVerticesPU[i][j] + channelsPU - 1) / channelsPU) * edgeAlign8;
            g[i][0].numVerticesPU[j] = numVerticesPU[i][j];
            g[i][0].numEdgesPU[j] = depth;
            for (int l = 0; l < channelsPU; ++l) {
                for (int k = 0; k < depth; ++k) {
                    g[i][0].weightsDense[j * channelsPU + l][k] = nullVal;
                }
            }
        }
    }

    int offset = 0;
    for (int m = 0; m < devicesNeeded * cuNm; ++m) {
        for (int i = 0; i < splitNm; ++i) {
            int cnt[channelsPU] = {0};
            int subChNm = (numVerticesPU[m][i] + channelsPU - 1) / channelsPU;
            for (int j = 0; j < numVerticesPU[m][i]; ++j) {
                int64_t lsb32 = oldVectors.get(offset).get(1);
                int64_t msb32 = oldVectors.get(offset).get(2);
                uint64_t fullID = ((msb32 << 32) & 0xFFFFFFF00000000) | (lsb32 & 0x00000000FFFFFFFF);
                xai::IDMap.push_back(fullID);
                for (int k = 3; k < vecLength; ++k) {
                    g[m][0].weightsDense[i * channelsPU + j / subChNm][cnt[j / subChNm] * edgeAlign8 + k - 3] =
                        oldVectors.get(offset).get(k);
                }
                cnt[j / subChNm] += 1;
                offset++;
            }
        }
    }

    int ret = load_graph_cosinesim_ss_dense_fpga_wrapper(devicesNeeded, cuNm, g);
    return ret;
}



inline ListAccum<XilCosinesimMatch> udf_cosinesim_ss_fpga(int64_t topK,
    int64_t numVertices, int64_t vecLength, ListAccum<int64_t>& newVector,
    int devicesNeeded)
{
    xai::Lock lock(xai::getMutex());
    ListAccum<XilCosinesimMatch> result;
    int32_t numEdges = vecLength - 3;
    const int splitNm = 3;    // kernel has 4 PUs, the input data should be splitted into 4 parts
    const int channelsPU = 4; // each PU has 4 HBM channels
    const int cuNm = 2;
    const int channelW = 16;
    const int32_t nullVal = 0;  // value to use for padding.  0 appears to be safe for cosine sim computation

    int32_t edgeAlign8 = ((numEdges + channelW - 1) / channelW) * channelW;
    //int general = ((numVertices + numDevices * cuNm * splitNm * channelsPU - 1) /
    //               (numDevices * cuNm * splitNm * channelsPU)) * channelsPU;
    // All channels need to have equal number of vertices except the last channel
    // The formula is to make sure the last channel always has data to process.
    // The last chanel may be assigned more data than other channels.
    int general = ((numVertices - (devicesNeeded * cuNm * splitNm * channelsPU + 1)) /
                  (devicesNeeded * cuNm * splitNm * channelsPU)) * channelsPU;
    int rest = numVertices - general * (devicesNeeded * cuNm * splitNm - 1);
    std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__
            << " numVertices=" << numVertices << ", general=" << general 
            << ", rest=" << rest 
            << ", split=" << devicesNeeded * cuNm * splitNm << std::endl;
    if (rest < 0) {
        //result += XilCosinesimMatch(VERTEX(-7), -7)
        return result;
    }

    //-------------------------------------------------------------------------
    std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time =
            std::chrono::high_resolution_clock::now();
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " preprocessing start=" << l_start_time.time_since_epoch().count() 
              << std::endl;
    //-------------------------------------------------------------------------

    int32_t** numVerticesPU = new int32_t*[devicesNeeded * cuNm]; // vertex numbers in each PU
    int32_t** numEdgesPU = new int32_t*[devicesNeeded * cuNm];    // edge numbers in each PU

    //int tmpID[devicesNeeded * cuNm * channelsPU * splitNm];
    for (int i = 0; i < devicesNeeded * cuNm; ++i) {
        numVerticesPU[i] = new int32_t[splitNm];
        numEdgesPU[i] = new int32_t[splitNm];
        for (int j = 0; j < splitNm; ++j) {
            numEdgesPU[i][j] = numEdges;
            for (int k = 0; k < channelsPU; ++k) {
                //tmpID[i * splitNm * channelsPU + j * channelsPU + k] = 0;
            }
        }
    }
    //---------------- setup number of vertices in each PU ---------
    for (int i = 0; i < devicesNeeded * cuNm; ++i) {
        for (int j = 0; j < splitNm; ++j) {
            numVerticesPU[i][j] = general;
        }
    }
    numVerticesPU[devicesNeeded * cuNm - 1][splitNm - 1] = rest;

    xf::graph::Graph<int32_t, int32_t>** g = new xf::graph::Graph<int32_t, int32_t>*[devicesNeeded * cuNm];
    int fpgaNodeNm = 0;
    for (int i = 0; i < devicesNeeded * cuNm; ++i) {
        g[i] = new xf::graph::Graph<int32_t, int32_t>("Dense", 4 * splitNm);
        g[i][0].numEdgesPU = new int32_t[splitNm];
        g[i][0].numVerticesPU = new int32_t[splitNm];
        g[i][0].edgeNum = numEdges;
        g[i][0].nodeNum = numVertices;
        g[i][0].splitNum = splitNm;
        g[i][0].refID = fpgaNodeNm;
        for (int j = 0; j < splitNm; ++j) {
            fpgaNodeNm += numVerticesPU[i][j];
            int depth = ((numVerticesPU[i][j] + channelsPU - 1) / channelsPU) * edgeAlign8;
            g[i][0].numVerticesPU[j] = numVerticesPU[i][j];
            g[i][0].numEdgesPU[j] = depth;
        }
    }
    //---------------- Generate Source Indice and Weight Array -------
    unsigned int sourceLen = edgeAlign8; // sourceIndice array length
    int32_t* sourceWeight =
        xf::graph::internal::aligned_alloc<int32_t>(sourceLen); // weights of source vertex's out members
    int32_t newVecLen = newVector.size() - 3;
    for (int i = 0; i < (int)sourceLen; i++) {
        if (i < newVecLen) {
            sourceWeight[i] = newVector.get(i + 3);
        } else {
            sourceWeight[i] = nullVal;
        }
    }
    float* similarity = xf::graph::internal::aligned_alloc<float>(topK);
    int32_t* resultID = xf::graph::internal::aligned_alloc<int32_t>(topK);
    memset(resultID, 0, topK * sizeof(int32_t));
    memset(similarity, 0, topK * sizeof(float));

    //---------------------------------------------------------------------------
    std::chrono::time_point<std::chrono::high_resolution_clock> l_end_time =
            std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = l_end_time - l_start_time;
    double l_timeMs = l_durationSec.count() * 1e3;
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " preprocessing runtime msec=  " << std::fixed << std::setprecision(6) 
              << l_timeMs << std::endl;
    //---------------------------------------------------------------------------

    //---------------- Run L3 API -----------------------------------
    //-------------------------------------------------------------------------
    std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time1 =
            std::chrono::high_resolution_clock::now();
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " cosinesim_ss_dense_fpga start=" << l_start_time1.time_since_epoch().count() << std::endl;
    //-------------------------------------------------------------------------

    cosinesim_ss_dense_fpga(
                  devicesNeeded * cuNm, sourceLen, sourceWeight, topK, g, 
                  resultID, similarity);
    //---------------------------------------------------------------------------
    std::chrono::time_point<std::chrono::high_resolution_clock> l_end_time1 =
            std::chrono::high_resolution_clock::now();
    l_durationSec = l_end_time1 - l_start_time1;
    l_timeMs = l_durationSec.count() * 1e3;
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " cosinesim_ss_dense_fpga runtime msec=  " << std::fixed << std::setprecision(6) 
              << l_timeMs << std::endl;
    //---------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time2 =
            std::chrono::high_resolution_clock::now();
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " postprocessing start=" << l_start_time2.time_since_epoch().count() << std::endl;
    //-------------------------------------------------------------------------

    for (unsigned int k = 0; k < topK; k++) {
        result += XilCosinesimMatch(VERTEX(xai::IDMap[resultID[k]]), similarity[k]);
    }
    //---------------------------------------------------------------------------
    std::chrono::time_point<std::chrono::high_resolution_clock> l_end_time2 =
            std::chrono::high_resolution_clock::now();
    l_durationSec = l_end_time2 - l_start_time2;
    l_timeMs = l_durationSec.count() * 1e3;
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " postprocessing runtime msec=  " << std::fixed << std::setprecision(6) 
              << l_timeMs << std::endl;
    //---------------------------------------------------------------------------

    return result;
}

//#####################################################################################################################

const unsigned VectorLength = 200;
const unsigned NumVectors = 5000;
const int MaxValue = 16383;


void dumpVector(const ListAccum<int64_t> &vec) {
    for (unsigned i = 0, end = vec.size(); i < end; ++i) {
        if (i % 10 == 0)
            std::cout << std::endl << i << ": ";
        std::cout << vec.get(i) << ' ';
    }
    std::cout << std::endl;
}


int main(int argc, char **argv) {
    std::srand(0x12345);
    ListAccum<int64_t> testVector;
    testVector.resize(VectorLength + 3, 0);
    ListAccum<ListAccum<int64_t>> population;
    population.resize(NumVectors);
    
    // Pick an index at random out of all the old vectors to use as the test vector to match
    
    const unsigned testVectorIndex = unsigned(std::rand() % NumVectors);
    
    // Generate random vectors, writing each into the Alveo card
    
    std::cout << "Loading population vectors into Alveo card..." << std::endl;
    for (unsigned vecNum = 0; vecNum < NumVectors; ++vecNum) {
        ListAccum<int64_t> &curVec = population.get(vecNum);
        curVec.resize(VectorLength + 3, 0);
        curVec.get(1) = int64_t(vecNum);
        
        for (unsigned eltNum = 0; eltNum < VectorLength; ++eltNum) {
            int64_t value = std::rand() % MaxValue - (MaxValue / 2);
            curVec.get(eltNum + 3) = value;
            
            // If we've reached the index we've chosen as the test vector, save the test vector values
            if (vecNum == testVectorIndex)
                testVector.get(eltNum + 3) = value;
        }
    }
    udf_load_graph_cosinesim_ss_fpga(NumVectors, VectorLength + 3, population, 1);
//    dumpVector(testVector);
    
    
    // Run the match in the FPGA
    
    std::cout << "Running match for test vector #" << testVectorIndex << "..." << std::endl;
//    ListAccum<XilCosinesimMatch> results = udf_cosinesim_ss_fpga(10, NumVectors, VectorLength, testVector, 1);
    ListAccum<XilCosinesimMatch> results = udf_cosinesim_ss_fpga(10, NumVectors, VectorLength + 3, population.get(testVectorIndex), 1);
    
    // Display the results
    
    std::cout << "Results:" << std::endl;
    std::cout << "Similarity   Vector #" << std::endl;
    std::cout << "----------   --------" << std::endl;
    for (auto &result : results)
        std::cout << result.similarity_ << "       " << VERTEX(result.index_) << std::endl;
    
    return 0;
}
