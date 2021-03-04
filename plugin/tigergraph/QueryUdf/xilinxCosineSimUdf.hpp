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
 * WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef XILINXCOSINESIMUDF_HPP
#define XILINXCOSINESIMUDF_HPP

// mergeHeaders 1 include start - DO NOT REMOVE


// mergeHeaders 1 include start xilinxCosineSimUdf.hpp DO NOT REMOVE
#include "loader.hpp"
// mergeHeaders 1 include end xilinxCosineSimUdf.hpp DO NOT REMOVE

// mergeHeaders 1 body start xilinxCosineSimUdf.hpp DO NOT REMOVE
inline int udf_loadgraph_cosinesim_ss_fpga(int64_t numVertices,
                                              int64_t vecLength,
                                              ListAccum<ListAccum<int64_t> >& oldVectors) {
    xai::IDMap.clear();
    int32_t numEdges = vecLength - 3;

    const int splitNm = 3;    // kernel has 4 PUs, the input data should be splitted into 4 parts
    const int channelsPU = 4; // each PU has 4 HBM channels
    const int cuNm = 2;
    int deviceNeeded = 1;
    const int channelW = 16;
    const int32_t nullVal = 0;  // value to use for padding.  0 appears to be safe for cosine sim computation

    int32_t edgeAlign8 = ((numEdges + channelW - 1) / channelW) * channelW;
    int general = ((numVertices + deviceNeeded * cuNm * splitNm * channelsPU - 1) /
                   (deviceNeeded * cuNm * splitNm * channelsPU)) *
                  channelsPU;
    int rest = numVertices - general * (deviceNeeded * cuNm * splitNm - 1);
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
            numEdgesPU[i][j] = numEdges;
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
    for (int m = 0; m < deviceNeeded * cuNm; ++m) {
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

    int ret = loadgraph_cosinesim_ss_dense_fpga_wrapper(deviceNeeded, cuNm, g);
    std::cout << "udf_loadgraph_cosinesim_ss_dense_fpga ret = " << ret << std::endl;
    return ret;
}

inline ListAccum<testResults> udf_cosinesim_ss_fpga(int64_t topK,
                                                    int64_t numVertices,
                                                    int64_t vecLength,
                                                    ListAccum<int64_t>& newVector) {
    ListAccum<testResults> result;
    int32_t numEdges = vecLength - 3;
    const int splitNm = 3;    // kernel has 4 PUs, the input data should be splitted into 4 parts
    const int channelsPU = 4; // each PU has 4 HBM channels
    const int cuNm = 2;
    int deviceNeeded = 1;
    const int channelW = 16;
    const int32_t nullVal = 0;  // value to use for padding.  0 appears to be safe for cosine sim computation

    int32_t edgeAlign8 = ((numEdges + channelW - 1) / channelW) * channelW;
    int general = ((numVertices + deviceNeeded * cuNm * splitNm * channelsPU - 1) /
                   (deviceNeeded * cuNm * splitNm * channelsPU)) *
                  channelsPU;
    int rest = numVertices - general * (deviceNeeded * cuNm * splitNm - 1);
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
            numEdgesPU[i][j] = numEdges;
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
        }
    }
    //---------------- Generate Source Indice and Weight Array -------
    unsigned int sourceLen = edgeAlign8; // sourceIndice array length
    int32_t* sourceWeight =
        xf::graph::internal::aligned_alloc<int32_t>(sourceLen); // weights of source vertex's out members
    int32_t newVecLen = newVector.size() - 3;
    for (int i = 0; i < sourceLen; i++) {
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

    //---------------- Run L3 API -----------------------------------
    int ret = cosinesim_ss_dense_fpga(deviceNeeded * cuNm, sourceLen, sourceWeight, topK, g, resultID, similarity);

    for (unsigned int k = 0; k < topK; k++) {
        result += testResults(VERTEX(xai::IDMap[resultID[k]]), similarity[k]);
    }

    return result;
}

// mergeHeaders 1 body end xilinxCosineSimUdf.hpp DO NOT REMOVE

#endif /* XILINXCOSINESIMUDF_HPP */

