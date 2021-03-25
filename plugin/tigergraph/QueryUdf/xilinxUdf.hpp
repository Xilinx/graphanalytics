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
//#include "loader.hpp"
// mergeHeaders 1 include start syntheaDemo DO NOT REMOVE!
#include "codevector.hpp"
#include <algorithm>
// mergeHeaders 1 include end syntheaDemo DO NOT REMOVE!

// mergeHeaders 1 include start xilinxRecomEngine DO NOT REMOVE!
#include "xilinxRecomEngine.hpp"
#include <cstdint>
#include <vector>
// mergeHeaders 1 include end xilinxRecomEngine DO NOT REMOVE!

// Error codes from L3 starts from -1
// Error codes from UDF starts from -1001
#define XF_GRAPH_UDF_GRAPH_PARTITION_ERROR -1001

namespace UDIMPL {

/* Start Xilinx UDF additions */

// mergeHeaders 1 body start syntheaDemo DO NOT REMOVE!

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
    xai::getTimerStartTime() = std::chrono::high_resolution_clock::now();
    return true;
}

inline double udf_elapsed_time(bool dummy) {
    xai::t_time_point cur_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = cur_time - xai::getTimerStartTime();
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
    // TODO: replicate this functionality (separate Alveo init from load) in cosine sim API
//    int ret = load_cu_cosinesim_ss_dense_fpga_wrapper(devicesNeeded, cuNm);
    int ret = 0;
    return ret;
}
// mergeHeaders 1 body end syntheaDemo DO NOT REMOVE!


// mergeHeaders 1 body start xilinxRecomEngine DO NOT REMOVE!

inline int udf_xilinx_recom_set_num_devices(std::uint64_t numDevices) {
    xai::Lock lock(xai::getMutex());
    xai::Context *pContext = xai::Context::getInstance();
    pContext->setNumDevices(unsigned(numDevices));
    return 0;
}

inline uint64_t udf_xilinx_recom_get_num_devices() {
    xai::Lock lock(xai::getMutex());
    xai::Context *pContext = xai::Context::getInstance();
    return pContext->getNumDevices();
}

inline bool udf_xilinx_recom_is_initialized() {
    xai::Lock lock(xai::getMutex());
    xai::Context *pContext = xai::Context::getInstance();
    return pContext->isInitialized();
}

inline int udf_load_graph_cosinesim_ss_fpga(int64_t numVertices,
                                           int64_t vecLength,
                                           ListAccum<ListAccum<int64_t> >& oldVectors,
                                           int devicesNeeded) {
    xai::Lock lock(xai::getMutex());
    xai::Context *pContext = xai::Context::getInstance();
    
    // Set the vector length up front, as changing it causes the context to be invalidated
    
    xilinx_apps::cosinesim::ColIndex inputVectorLength = xilinx_apps::cosinesim::ColIndex(
            xilinx_apps::cosinesim::ColIndex(oldVectors.get(0).size()));
    xilinx_apps::cosinesim::ColIndex vectorLength = inputVectorLength - 3;
    pContext->setVectorLength(vectorLength);
    
    // If there are no vectors, consider the FPGA to be uninitialized
    
    xilinx_apps::cosinesim::RowIndex numVectors = xilinx_apps::cosinesim::RowIndex(oldVectors.size());
    if (numVectors < 1) {
        pContext->clear();
        return 0;
    }

    // Prepare the FPGA row to vertex ID map
    
    xai::Context::IdMap &idMap = pContext->getIdMap();
    idMap.clear();
    idMap.resize(numVectors, 0);
    std::cout << "UDF after idMap resize: numVectors=" << numVectors << ", idMap.size=" << idMap.size() << std::endl;
    std::cout << "UDF load point 1, idMap.size=" << idMap.size() << std::endl;
    std::cout << "UDF load point 2, idMap.size=" << idMap.size() << std::endl;
    
    // Fill the FPGA(s) with vectors
    
    xai::CosineSim *pCosineSim = pContext->getCosineSimObj();
    std::cout << "UDF pConsineSim=" << pCosineSim
            << ", numVertices=" << numVertices
            << ", numVectors=" << numVectors
            << std::endl;
    std::cout << "UDF load point 3, idMap.size=" << idMap.size() << std::endl;
//    pCosineSim->openFpga();
    pCosineSim->startLoadPopulation(numVectors);
    std::cout << "UDF after startLoadPopulation" << std::endl;
    std::cout << "UDF load point 4, idMap.size=" << idMap.size() << std::endl;
    for (xilinx_apps::cosinesim::RowIndex vecNum = 0; vecNum < numVectors; ++vecNum) {
        const ListAccum<int64_t> &curRowVec = oldVectors.get(vecNum);
        xilinx_apps::cosinesim::RowIndex rowIndex = 0;
        xai::CosineSim::ValueType *pBuf = pCosineSim->getPopulationVectorBuffer(rowIndex);
        for (xilinx_apps::cosinesim::ColIndex eltNum = 0; eltNum < vectorLength; ++eltNum)
            *pBuf++ = xai::CosineSim::ValueType(curRowVec.get(eltNum + 3));
        pCosineSim->finishCurrentPopulationVector(pBuf);
        uint64_t vertexId = ((curRowVec.get(2) << 32) & 0xFFFFFFF00000000) | (curRowVec.get(1) & 0x00000000FFFFFFFF);
        std::cout << "UDF load loop: rowIndex=" << rowIndex << ", vertexId=" << vertexId << "idMap.size:before="
                << idMap.size();
        idMap[rowIndex] = vertexId;
        std::cout << ", idMap.size:after=" << idMap.size() << std::endl;
    }
    std::cout << "UDF before finishLoadPopulationVectors, idMap.size=" << idMap.size() << std::endl;
    pCosineSim->finishLoadPopulationVectors();
    std::cout << "UDF after finishLoadPopulationVectors, idMap.size=" << idMap.size()
            << ", idMap=" << (void *)(&idMap) << std::endl;
    pContext->setInitialized();  // FPGA(s) now ready to match
    return 0;
}

// Enable this to print profiling messages to the log (via stdout)
#define XILINX_RECOM_PROFILE_ON

inline ListAccum<testResults> udf_cosinesim_ss_fpga(int64_t topK,
    int64_t numVertices, int64_t vecLength, ListAccum<int64_t>& newVector,
    int devicesNeeded)
{
    xai::Lock lock(xai::getMutex());
    ListAccum<testResults> result;
    std::cout << "UDF start match" << std::endl;
    xai::Context *pContext = xai::Context::getInstance();

    if (!pContext->isInitialized())
        return result;
    std::cout << "UDF after isInitialized" << std::endl;
    xai::Context::IdMap &idMap = pContext->getIdMap();
    std::cout << "UDF match idMap.size=" << idMap.size() << ", idMap=" << (void *)(&idMap) << std::endl;

//    std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__
//            << " numVertices=" << numVertices << ", general=" << general 
//            << ", rest=" << rest 
//            << ", split=" << devicesNeeded * cuNm * splitNm << std::endl;

    //-------------------------------------------------------------------------
#ifdef XILINX_RECOM_PROFILE_ON
    std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time =
            std::chrono::high_resolution_clock::now();
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " preprocessing start=" << l_start_time.time_since_epoch().count() 
              << std::endl;
#endif
    //-------------------------------------------------------------------------

    std::vector<xai::CosineSim::ValueType> nativeTargetVector;
    const xilinx_apps::cosinesim::ColIndex vectorLength = pContext->getVectorLength();
    nativeTargetVector.reserve(vectorLength);
    for (xilinx_apps::cosinesim::ColIndex eltNum = 0; eltNum < vectorLength; ++eltNum)
        nativeTargetVector.push_back(newVector.get(eltNum + 3));
    xai::CosineSim *pCosineSim = pContext->getCosineSimObj();

    //---------------------------------------------------------------------------
#ifdef XILINX_RECOM_PROFILE_ON
    std::chrono::time_point<std::chrono::high_resolution_clock> l_end_time =
            std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = l_end_time - l_start_time;
    double l_timeMs = l_durationSec.count() * 1e3;
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " preprocessing runtime msec=  " << std::fixed << std::setprecision(6) 
              << l_timeMs << std::endl;
#endif
    //---------------------------------------------------------------------------

    //---------------- Run L3 API -----------------------------------
    //-------------------------------------------------------------------------
#ifdef XILINX_RECOM_PROFILE_ON
    std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time1 =
            std::chrono::high_resolution_clock::now();
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " cosinesim_ss_dense_fpga start=" << l_start_time1.time_since_epoch().count() << std::endl;
#endif
    //-------------------------------------------------------------------------

    std::vector<xilinx_apps::cosinesim::Result> apiResults
            = pCosineSim->matchTargetVector(topK, nativeTargetVector.data());
    
    //---------------------------------------------------------------------------
#ifdef XILINX_RECOM_PROFILE_ON
    std::chrono::time_point<std::chrono::high_resolution_clock> l_end_time1 =
            std::chrono::high_resolution_clock::now();
    l_durationSec = l_end_time1 - l_start_time1;
    l_timeMs = l_durationSec.count() * 1e3;
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " cosinesim_ss_dense_fpga runtime msec=  " << std::fixed << std::setprecision(6) 
              << l_timeMs << std::endl;
#endif
    //---------------------------------------------------------------------------

    //-------------------------------------------------------------------------
#ifdef XILINX_RECOM_PROFILE_ON
    std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time2 =
            std::chrono::high_resolution_clock::now();
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " postprocessing start=" << l_start_time2.time_since_epoch().count() << std::endl;
#endif
    //-------------------------------------------------------------------------

    std::cout << "UDF results size=" << apiResults.size() << std::endl;
    std::cout << "UDF idMap size=" << idMap.size() << std::endl;
    for (xilinx_apps::cosinesim::Result &apiResult : apiResults) {
        std::cout << "UDF apiResult.index=" << apiResult.index_ << std::endl;
        if (apiResult.index_ < 0 || apiResult.index_ >= xilinx_apps::cosinesim::RowIndex(idMap.size()))
            continue;
        result += testResults(VERTEX(idMap[apiResult.index_]), apiResult.similarity_);
    }

    //---------------------------------------------------------------------------
#ifdef XILINX_RECOM_PROFILE_ON
    std::chrono::time_point<std::chrono::high_resolution_clock> l_end_time2 =
            std::chrono::high_resolution_clock::now();
    l_durationSec = l_end_time2 - l_start_time2;
    l_timeMs = l_durationSec.count() * 1e3;
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " postprocessing runtime msec=  " << std::fixed << std::setprecision(6) 
              << l_timeMs << std::endl;
#endif
    //---------------------------------------------------------------------------

    return result;
}

/* End Xilinx Cosine Similarity Additions */
// mergeHeaders 1 body end xilinxRecomEngine DO NOT REMOVE!

}

#endif /* EXPRFUNCTIONS_HPP_ */
