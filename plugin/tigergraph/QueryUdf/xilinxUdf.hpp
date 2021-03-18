/*
 * Copyright 2020 Xilinx, Inc.
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
/**
 * @file graph.hpp
 * @brief  This files contains graph definition.
 */

#ifndef _XILINXUDF_HPP_
#define _XILINXUDF_HPP_

#include "tgFunctions.hpp"
#include "loader.hpp"
#include "codevector.hpp"
#include <algorithm>

// Error codes from L3 starts from -1
// Error codes from UDF starts from -1001
#define XF_GRAPH_UDF_GRAPH_PARTITION_ERROR -1001

namespace UDIMPL {

/* Start Xilinx UDF additions */

inline double udf_bfs_fpga(int64_t sourceID,
                           ListAccum<int64_t>& offsetsList,
                           ListAccum<int64_t>& indicesList,
                           ListAccum<float>& weightsList,
                           ListAccum<int64_t>& predecent,
                           ListAccum<int64_t>& distance) {
    int numEdges = indicesList.size();
    int numVertices = offsetsList.size() - 1;
    uint32_t* predecentTmp = xf::graph::internal::aligned_alloc<uint32_t>(((numVertices + 15) / 16) * 16);
    uint32_t* distanceTmp = xf::graph::internal::aligned_alloc<uint32_t>(((numVertices + 15) / 16) * 16);
    memset(predecentTmp, -1, sizeof(uint32_t) * (((numVertices + 15) / 16) * 16));
    memset(distanceTmp, -1, sizeof(uint32_t) * (((numVertices + 15) / 16) * 16));
    xf::graph::Graph<uint32_t, uint32_t> g("CSR", numVertices, numEdges);

    int count = 0;
    while (count < numEdges) {
        if (count < offsetsList.size()) {
            int value0 = (int)(offsetsList.get(count));
            g.offsetsCSR[count] = value0;
        }
        int value = (int)(indicesList.get(count));
        float value1 = (float)(weightsList.get(count));
        g.indicesCSR[count] = value;
        g.weightsCSR[count] = 1;
        count++;
    }
    int res = bfs_fpga_wrapper(numVertices, numEdges, sourceID, g, predecentTmp, distanceTmp);

    for (int i = 0; i < numVertices; ++i) {
        if (predecentTmp[i] == (uint32_t)(-1)) {
            predecent += -1;
            distance += -1;
        } else {
            predecent += predecentTmp[i];
            distance += distanceTmp[i];
        }
    }
    g.freeBuffers();
    free(predecentTmp);
    free(distanceTmp);
    return res;
}

inline double udf_load_xgraph_fpga(ListAccum<int64_t>& offsetsList,
                                   ListAccum<int64_t>& indicesList,
                                   ListAccum<float>& weightsList) {
    int numEdges = indicesList.size();
    int numVertices = offsetsList.size() - 1;
    xf::graph::Graph<uint32_t, float> g("CSR", numVertices, numEdges);

    int count = 0;
    while (count < numEdges) {
        if (count < offsetsList.size()) {
            int value0 = (int)(offsetsList.get(count));
            g.offsetsCSR[count] = value0;
        }
        int value = (int)(indicesList.get(count));
        float value1 = (float)(weightsList.get(count));
        g.indicesCSR[count] = value;
        g.weightsCSR[count] = value1;
        count++;
    }
    int res = load_xgraph_fpga_wrapper(numVertices, numEdges, g);

    g.freeBuffers();
    return res;
}

inline double udf_shortest_ss_pos_wt_fpga(int64_t sourceID,
                                          int64_t numEdges,
                                          int64_t numVertices,
                                          ListAccum<int64_t>& predecent,
                                          ListAccum<float>& distance) {
    uint32_t length = ((numVertices + 1023) / 1024) * 1024;
    float** result;
    uint32_t** pred;
    result = new float*[1];
    pred = new uint32_t*[1];
    result[0] = xf::graph::internal::aligned_alloc<float>(length);
    pred[0] = xf::graph::internal::aligned_alloc<uint32_t>(length);
    memset(result[0], 0, length * sizeof(float));
    memset(pred[0], 0, length * sizeof(uint32_t));

    xf::graph::Graph<uint32_t, float> g("CSR", numVertices, numEdges);

    int res = shortest_ss_pos_wt_fpga_wrapper(numVertices, sourceID, 1, g, result, pred);

    for (int i = 0; i < numVertices; ++i) {
        predecent += pred[0][i];
        distance += result[0][i];
    }
    free(result[0]);
    free(pred[0]);
    delete[] result;
    delete[] pred;
    return res;
}

inline double udf_load_xgraph_pageRank_wt_fpga(ListAccum<int64_t>& offsetsList,
                                               ListAccum<int64_t>& indicesList,
                                               ListAccum<float>& weightsList) {
    int numEdges = indicesList.size();
    int numVertices = offsetsList.size() - 1;
    xf::graph::Graph<uint32_t, float> g("CSR", numVertices, numEdges);

    int count = 0;
    while (count < numEdges) {
        if (count < offsetsList.size()) {
            int value0 = (int)(offsetsList.get(count));
            g.offsetsCSR[count] = value0;
        }
        int value = (int)(indicesList.get(count));
        float value1 = (float)(weightsList.get(count));
        g.indicesCSR[count] = value;
        g.weightsCSR[count] = value1;
        count++;
    }
    int res = load_xgraph_pageRank_wt_fpga_wrapper(numVertices, numEdges, g);

    g.freeBuffers();
    return res;
}

inline double udf_pageRank_wt_fpga(
    int64_t numVertices, int64_t numEdges, float alpha, float tolerance, int64_t maxIter, ListAccum<float>& rank) {
    float* rankValue = new float[numVertices];

    xf::graph::Graph<uint32_t, float> g("CSR", numVertices, numEdges);

    int res = pageRank_wt_fpga_wrapper(alpha, tolerance, maxIter, g, rankValue);

    for (int i = 0; i < numVertices; ++i) {
        rank += rankValue[i];
    }
    delete[] rankValue;
    return res;
}

inline bool concat_uint64_to_str(string& ret_val, uint64_t val) {
    (ret_val += " ") += std::to_string(val);
    return true;
}

inline int64_t float_to_int_xilinx(float val) {
    return (int64_t)val;
}

inline int64_t udf_reinterpret_double_as_int64(double val) {
    int64_t double_to_int64 = *(reinterpret_cast<int64_t*>(&val));
    return double_to_int64;
}

inline double udf_reinterpret_int64_as_double(int64_t val) {
    double int64_to_double = *(reinterpret_cast<double*>(&val));
    return int64_to_double;
}

inline int64_t udf_lsb32bits(uint64_t val) {
    return val & 0x00000000FFFFFFFF;
}

inline int64_t udf_msb32bits(uint64_t val) {
    return (val >> 32) & 0x00000000FFFFFFFF;
}

inline VERTEX udf_getvertex(uint64_t vid) {
    return VERTEX(vid);
}

inline bool udf_setcode(int property, uint64_t startCode, uint64_t endCode, int64_t size) {
    return true;
}

inline bool udf_reset_timer(bool dummy) {
    xai::timer_start_time = std::chrono::high_resolution_clock::now();
    return true;
}

inline double udf_elapsed_time(bool dummy) {
    xai::t_time_point cur_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = cur_time - xai::timer_start_time;
    double l_timeMs = l_durationSec.count() * 1e3;
    return l_timeMs;
}

inline double udf_cos_theta(ListAccum<int64_t> vec_A, ListAccum<int64_t> vec_B) {
    double res;
    int size = vec_A.size();
    int64_t norm_A = vec_A.get(0);
    double norm_d_A = *(reinterpret_cast<double*>(&norm_A));
    int64_t norm_B = vec_B.get(0);
    double norm_d_B = *(reinterpret_cast<double*>(&norm_B));
    double prod = 0;
    int i = xai::startPropertyIndex;
    while (i < size) {
        prod = prod + vec_A.get(i) * vec_B.get(i);
        ++i;
    }
    res = prod / (norm_d_A * norm_d_B);
    //std::cout << "val = " << res << std::endl;
    return res;
}

inline ListAccum<int64_t> udf_get_similarity_vec(int64_t property,
                                                 int64_t returnVecLength,
                                                 ListAccum<uint64_t>& property_vector) {
    ListAccum<uint64_t> result;
    int64_t size = property_vector.size();
    std::vector<uint64_t> codes;
    for (uint64_t val : property_vector) {
        codes.push_back(val);
    }
    std::vector<int> retcodes = xai::makeCosineVector(property, returnVecLength, codes);
    for (int value : retcodes) {
        result += value;
    }
    return result;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
inline int udf_load_cu_cosinesim_ss_fpga(int devicesNeeded) 
{
    const int cuNm = 2;
    int ret = load_cu_cosinesim_ss_dense_fpga_wrapper(devicesNeeded, cuNm);
    return ret;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
inline int udf_load_graph_cosinesim_ss_fpga(int64_t numVertices,
                                            int64_t vecLength,
                                            ListAccum<ListAccum<int64_t> >& oldVectors,
                                            int devicesNeeded) 
{
    xai::Lock lock(xai::getMutex());
    xai::IDMap.clear();
    ListAccum<testResults> result;
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

    int tmpID[devicesNeeded * cuNm * channelsPU * splitNm];
    for (int i = 0; i < devicesNeeded * cuNm; ++i) {
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

inline ListAccum<testResults> udf_cosinesim_ss_fpga(int64_t topK,
    int64_t numVertices, int64_t vecLength, ListAccum<int64_t>& newVector,
    int devicesNeeded)
{
    xai::Lock lock(xai::getMutex());
    ListAccum<testResults> result;
    int32_t numEdges = vecLength - 3;
    const int splitNm = 3;    // kernel has 4 PUs, the input data should be splitted into 4 parts
    const int channelsPU = 4; // each PU has 4 HBM channels
    const int cuNm = 2;
    const int channelW = 16;
    const int32_t nullVal = 0;  // value to use for padding.  0 appears to be safe for cosine sim computation

    int32_t edgeAlign8 = ((numEdges + channelW - 1) / channelW) * channelW;
    //int general = ((numVertices + deviceNeeded * cuNm * splitNm * channelsPU - 1) /
    //               (deviceNeeded * cuNm * splitNm * channelsPU)) * channelsPU;
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
        //result += testResults(VERTEX(-7), -7)
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

    int tmpID[devicesNeeded * cuNm * channelsPU * splitNm];
    for (int i = 0; i < devicesNeeded * cuNm; ++i) {
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

    int ret = cosinesim_ss_dense_fpga(
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
        result += testResults(VERTEX(xai::IDMap[resultID[k]]), similarity[k]);
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

/* End Xilinx Cosine Similarity Additions */
}

#endif /* EXPRFUNCTIONS_HPP_ */
