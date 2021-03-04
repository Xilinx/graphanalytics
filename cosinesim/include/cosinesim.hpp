/*
 * Copyright 2020-2021 Xilinx, Inc.
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

#ifndef XILINX_APPS_COSINESIM_HPP
#define XILINX_APPS_COSINESIM_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include "xf_graph_L3_handle.hpp"

namespace xilinx_apps {
namespace cosinesim {



using RowIndex = std::int64_t;
using ColIndex = std::int32_t;

class ImplBase {
public:
	virtual ~ImplBase(){};
	virtual void *getPopulationVectorBuffer(RowIndex &rowIndex) = 0;
};

extern "C" {
    ImplBase *createImpl();
    void destroyImpl(ImplBase *pImpl);
}

template <typename Value>
class CosineSim {
public:
    

    struct Result {
        RowIndex index_ = -1L;
        double similarity_ = 0.0;
        
        Result(RowIndex index, double similarity) {
            index_ = index;
            similarity_ = similarity;
        }
    };
    
    struct Options {
        
    };
    

    CosineSim(ColIndex vecLength, const Options &options) : vecLength_(vecLength), options_(options), pImpl_(createImpl()) {};
    ColIndex getVectorLength() const { return vecLength_; }
    
    void openFpga(...);
    void startLoadPopulation();  //

    Value *getPopulationVectorBuffer(RowIndex &rowIndex) {
        // figure out where in weightDense to start writing
        // memset vector padding (8 bytes for example) to 0
        // return pointer into weightDense
         return reinterpret_cast<Value *>(pImpl_->getPopulationVectorBuffer(rowIndex));
    }
    void finishCurrentPopulationVector();
    void finishLoadPopulationVectors();
    
    int loadOldVectors(RowIndex numRows, const Value *elements) {
//        startLoadOldVectors();
//        for i in elements[]
//            pBuf = getOldVectorBuffer();
//            memcpy(pBuf, oldVectors[i], numElements * sizeof(int32));
//            finishCurrentOldVector();
//        finishLoadOldVectors();
    	return 1;
    }
    
    std::vector<Result> matchTargetVector(unsigned numResults, const Value *elements);
    void closeFpga();
    
private:
    Options options_;
    ColIndex vecLength_ = 0;
    RowIndex numRows_ = 0;
    std::unique_ptr<ImplBase> pImpl_;
    /*
    int CosineSim<Value>::loadgraph_cosinesim_ss_dense_fpga(uint32_t deviceNeeded, uint32_t cuNm,
        xf::graph::Graph<int32_t, int32_t>** g);
    void cosinesim_ss_dense_fpga(uint32_t deviceNeeded, int32_t sourceLen, int32_t* sourceWeight,
        int32_t topK, xf::graph::Graph<int32_t, int32_t>** g, int32_t* resultID, float* similarity);
*/
};

/*
//#####################################################################################################################

//
// Implementation
//

template <typename Value>
int CosineSim<Value>::loadOldVectors(RowIndex numRows, const Value *elements) {
    const int splitNm = 3;    // kernel has 4 PUs, the input data should be splitted into 4 parts
    const int channelsPU = 4; // each PU has 4 HBM channels
    const int cuNm = 2;
    int deviceNeeded = 1;
    const int channelW = 16;

    int32_t edgeAlign8 = ((vecLength_ + channelW - 1) / channelW) * channelW;
    int general = ((numRows + deviceNeeded * cuNm * splitNm * channelsPU - 1) /
                   (deviceNeeded * cuNm * splitNm * channelsPU)) *
                  channelsPU;
    int rest = numRows - general * (deviceNeeded * cuNm * splitNm - 1);
    if (rest < 0) {
        exit(1);
    }
    int32_t** numVerticesPU = new int32_t*[deviceNeeded * cuNm]; // vertex numbers in each PU
    int32_t** numEdgesPU = new int32_t*[deviceNeeded * cuNm];    // edge numbers in each PU

    int tmpID[deviceNeeded * cuNm * channelsPU * splitNm];
    for (int i = 0; i < deviceNeeded * cuNm; ++i) {
        numVerticesPU[i] = new int32_t[splitNm];
        numEdgesPU[i] = new int32_t[splitNm];
        for (int j = 0; j < splitNm; ++j) {
            numEdgesPU[i][j] = vecLength_;
            for (int k = 0; k < channelsPU; ++k) {
                tmpID[i * splitNm * channelsPU + j * channelsPU + k] = 0;
            }
        }
    }
    //---------------- setup number of vertices in each PU ---------
    for (int i = 0; i < deviceNeeded * cuNm; ++i) {
        for (int j = 0; j < splitNm; ++j) {
            numVerticesPU[i][j] = general;
        }
    }
    numVerticesPU[deviceNeeded * cuNm - 1][splitNm - 1] = rest;

    xf::graph::Graph<int32_t, int32_t>** g = new xf::graph::Graph<int32_t, int32_t>*[deviceNeeded * cuNm];
    int fpgaNodeNm = 0;
    for (int i = 0; i < deviceNeeded * cuNm; ++i) {
        g[i] = new xf::graph::Graph<int32_t, int32_t>("Dense", 4 * splitNm, vecLength_, numVerticesPU[i]);
        g[i][0].numEdgesPU = new int32_t[splitNm];
        g[i][0].numVerticesPU = new int32_t[splitNm];
        g[i][0].edgeNum = vecLength_;
        g[i][0].nodeNum = numRows;
        g[i][0].splitNum = splitNm;
        g[i][0].refID = fpgaNodeNm;
        for (int j = 0; j < splitNm; ++j) {
            fpgaNodeNm += numVerticesPU[i][j];
            int depth = ((numVerticesPU[i][j] + channelsPU - 1) / channelsPU) * edgeAlign8;
            g[i][0].numVerticesPU[j] = numVerticesPU[i][j];
            g[i][0].numEdgesPU[j] = depth;
            for (int l = 0; l < channelsPU; ++l) {
                for (int k = 0; k < depth; ++k) {
                    g[i][0].weightsDense[j * channelsPU + l][k] = nullValue_;
                }
            }
        }
    }

    int offset = 0;
    for (int m = 0; m < deviceNeeded * cuNm; ++m) {
        for (int i = 0; i < splitNm; ++i) {
            int cnt[channelsPU] = {0};
            int subChNm = (numVerticesPU[m][i] + channelsPU - 1) / channelsPU;
            for (int j = 0; j < numVerticesPU[m][i]; ++j) {
                for (int k = 0; k < vecLength_; ++k) {
                    g[m][0].weightsDense[i * channelsPU + j / subChNm][cnt[j / subChNm] * edgeAlign8 + k] =
                        elements[offset];
                    ++offset;
                }
                cnt[j / subChNm] += 1;
            }
        }
    }

    int ret = loadgraph_cosinesim_ss_dense_fpga_wrapper(deviceNeeded, cuNm, g);
    std::cout << "udf_loadgraph_cosinesim_ss_dense_fpga ret = " << ret << std::endl;
    return ret;
}


template <typename Value>
std::vector<typename  CosineSim<Value>::Result> CosineSim<Value>::matchNewVector(unsigned numResults,
        const Value *elements)
{
    std::vector<Result> result;
    const int splitNm = 3;    // kernel has 4 PUs, the input data should be splitted into 4 parts
    const int channelsPU = 4; // each PU has 4 HBM channels
    const int cuNm = 2;
    int deviceNeeded = 1;
    const int channelW = 16;

    int32_t edgeAlign8 = ((vecLength_ + channelW - 1) / channelW) * channelW;
    int general = ((numRows_ + deviceNeeded * cuNm * splitNm * channelsPU - 1) /
                   (deviceNeeded * cuNm * splitNm * channelsPU)) *
                  channelsPU;
    int rest = numRows_ - general * (deviceNeeded * cuNm * splitNm - 1);
    if (rest < 0) {
        exit(1);
    }
    int32_t** numVerticesPU = new int32_t*[deviceNeeded * cuNm]; // vertex numbers in each PU
    int32_t** numEdgesPU = new int32_t*[deviceNeeded * cuNm];    // edge numbers in each PU

    int tmpID[deviceNeeded * cuNm * channelsPU * splitNm];
    for (int i = 0; i < deviceNeeded * cuNm; ++i) {
        numVerticesPU[i] = new int32_t[splitNm];
        numEdgesPU[i] = new int32_t[splitNm];
        for (int j = 0; j < splitNm; ++j) {
            numEdgesPU[i][j] = vecLength_;
            for (int k = 0; k < channelsPU; ++k) {
                tmpID[i * splitNm * channelsPU + j * channelsPU + k] = 0;
            }
        }
    }
    //---------------- setup number of vertices in each PU ---------
    for (int i = 0; i < deviceNeeded * cuNm; ++i) {
        for (int j = 0; j < splitNm; ++j) {
            numVerticesPU[i][j] = general;
        }
    }
    numVerticesPU[deviceNeeded * cuNm - 1][splitNm - 1] = rest;

    xf::graph::Graph<int32_t, int32_t>** g = new xf::graph::Graph<int32_t, int32_t>*[deviceNeeded * cuNm];
    int fpgaNodeNm = 0;
    for (int i = 0; i < deviceNeeded * cuNm; ++i) {
        g[i] = new xf::graph::Graph<int32_t, int32_t>("Dense", 4 * splitNm, vecLength_, numVerticesPU[i]);
        g[i][0].numEdgesPU = new int32_t[splitNm];
        g[i][0].numVerticesPU = new int32_t[splitNm];
        g[i][0].edgeNum = vecLength_;
        g[i][0].nodeNum = numRows_;
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
    for (int i = 0; i < sourceLen; i++) {
        if (i < vecLength_) {
            sourceWeight[i] = elements.get(i + 3);
        } else {
            sourceWeight[i] = nullValue_;
        }
    }
    float* similarity = xf::graph::internal::aligned_alloc<float>(numResults);
    int32_t* resultID = xf::graph::internal::aligned_alloc<int32_t>(numResults);
    memset(resultID, 0, numResults * sizeof(int32_t));
    memset(similarity, 0, numResults * sizeof(float));

    //---------------- Run L3 API -----------------------------------
    int ret = cosinesim_ss_dense_fpga(deviceNeeded * cuNm, sourceLen, sourceWeight, numResults, g, resultID, similarity);

    for (unsigned int k = 0; k < numResults; k++) {
        result.emplace_back(resultID[k], similarity[k]);
    }

    return std::vector<Result>();
}


template <typename Value>
int CosineSim<Value>::loadgraph_cosinesim_ss_dense_fpga(uint32_t deviceNeeded, uint32_t cuNm,
        xf::graph::Graph<int32_t, int32_t>** g)
{
    //----------------- Text Parser --------------------------
    std::string opName;
    std::string kernelName;
    int requestLoad;
    std::string xclbinPath;
    std::string xclbinPath2;

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
            } else if (!std::strcmp(token, "xclbinPath2")) {
                token = strtok(NULL, "\"\t ,}:{\n");
                std::string tmpStr2 = token;
                xclbinPath2 = tmpStr2;
            } else if (!std::strcmp(token, "deviceNeeded")) {
                token = strtok(NULL, "\"\t ,}:{\n");
                //             deviceNeeded = std::atoi(token);
            }
            token = strtok(NULL, "\"\t ,}:{\n");
        }
    }
    userInput.close();

    //----------------- Setup shortestPathFloat thread ---------
    xf::graph::L3::Handle::singleOP op0;
    op0.operationName = "similarityDense";
    op0.setKernelName("denseSimilarityKernel");
    op0.requestLoad = 100;
    op0.xclbinFile = xclbinFileName_.c_str();
    op0.xclbinFile2 = std::string();
    op0.deviceNeeded = deviceNeeded;
    op0.cuPerBoard = cuNm;
    
    std::fstream xclbinFS(xclbinPath, std::ios::in);
    if (!xclbinFS) {
        std::cout << "Error : xclbinFile doesn't exist: " << xclbinPath << std::endl;
        return -3;
    }

    std::fstream xclbinFS2(xclbinPath2, std::ios::in);
    if (deviceNeeded > 1 && !xclbinFS2) {
        std::cout << "Error : xclbinFile2 doesn't exist: " << xclbinPath2 << std::endl;
        return XF_GRAPH_L3_ERROR_XCLBIN2_FILE_NOT_EXIST;
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
    handle1->debug();
    //------------------------
    
    //---------------- Run Load Graph -----------------------------------
    for (int i = 0; i < deviceNeeded * cuNm; ++i) {
        (handle0->opsimdense)->loadGraphMultiCardNonBlocking(i / cuNm, i % cuNm, g[i][0]);
    }

    //--------------- Free and delete -----------------------------------

    for (int i = 0; i < deviceNeeded * cuNm; ++i) {
        g[i]->freeBuffers();
        delete[] g[i]->numEdgesPU;
        delete[] g[i]->numVerticesPU;
    }
    delete[] g;
    
    return 0;
}


template <typename Value>
void CosineSim<Value>::cosinesim_ss_dense_fpga(uint32_t deviceNeeded, int32_t sourceLen, int32_t* sourceWeight,
        int32_t topK, xf::graph::Graph<int32_t, int32_t>** g, int32_t* resultID, float* similarity)
{
    //---------------- Run Load Graph -----------------------------------
    std::cout << "INFO: L3_wrapper::cosinesim_ss_dense_fpga starting..." << std::endl;
    std::shared_ptr<xf::graph::L3::Handle> handle0 = sharedHandlesCosSimDense::instance().handlesMap[0];
    handle0->debug();
    int32_t requestNm = 1;
    //    int ret = xf::graph::L3::cosineSimilaritySSDenseMultiCard(handle0, deviceNeeded, sourceLen, sourceWeight,
    //    topK, g,
    //                                                              resultID, similarity);
    int32_t hwNm = deviceNeeded;
    std::cout << "hwNm = " << hwNm << std::endl;
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
        eventQueue[m] = xf::graph::L3::cosineSimilaritySSDenseMultiCard(handle0, hwNm, sourceLen, sourceWeight, topK, g,
                                                                        resultID0[m], similarity0[m]);
    }

    int ret = 0;
    for (int m = 0; m < requestNm; ++m) {
        for (int i = 0; i < eventQueue[m].size(); ++i) {
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
}
*/
} // namespace cosinesim
} // namespace xilinx_apps


#endif /* XILINX_APPS_COSINESIM_HPP */
