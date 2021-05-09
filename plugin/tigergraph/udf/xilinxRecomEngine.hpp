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

#ifndef XILINX_RECOM_ENGINE_HPP
#define XILINX_RECOM_ENGINE_HPP

// mergeHeaders 1 name xilinxRecomEngine

// mergeHeaders 1 section include start xilinxRecomEngine DO NOT REMOVE!
#include "xilinxRecomEngineImpl.hpp"
#include <cstdint>
#include <vector>
// mergeHeaders 1 section include end xilinxRecomEngine DO NOT REMOVE!

namespace UDIMPL {

// mergeHeaders 1 section body start xilinxRecomEngine DO NOT REMOVE!

// Deprecated
inline int udf_load_cu_cosinesim_ss_fpga(int devicesNeeded) 
{
    const int cuNm = 2;
    // TODO: replicate this functionality (separate Alveo init from load) in cosine sim API
//    int ret = load_cu_cosinesim_ss_dense_fpga_wrapper(devicesNeeded, cuNm);
    int ret = 0;
    return ret;
}

inline int udf_xilinx_recom_set_node_id(uint nodeId) {
    std::cout << __FUNCTION__ << " nodeId=" << nodeId << std::endl;

    xilRecom::Lock lock(xilRecom::getMutex());
    xilRecom::Context *pContext = xilRecom::Context::getInstance();
    pContext->setNodeId(unsigned(nodeId));
    return 0;
}

inline uint udf_xilinx_recom_get_node_id() {
    xilRecom::Lock lock(xilRecom::getMutex());
    xilRecom::Context *pContext = xilRecom::Context::getInstance();
    return pContext->getNodeId();
}


inline int udf_xilinx_recom_set_num_devices(std::uint64_t numDevices) {
    std::cout << "DEBUG: UDF START" << __FUNCTION__ << " numDevices=" << numDevices << std::endl;

    xilRecom::Lock lock(xilRecom::getMutex());
    xilRecom::Context *pContext = xilRecom::Context::getInstance();
    pContext->setNumDevices(unsigned(numDevices));
    return 0;
}

inline uint64_t udf_xilinx_recom_get_num_devices() {
    xilRecom::Lock lock(xilRecom::getMutex());
    xilRecom::Context *pContext = xilRecom::Context::getInstance();
    return pContext->getNumDevices();
}

inline bool udf_xilinx_recom_is_initialized() {
    xilRecom::Lock lock(xilRecom::getMutex());
    xilRecom::Context *pContext = xilRecom::Context::getInstance();
    return pContext->isInitialized();
}

// Each element in arrayNumVertices holds the number of vertices on a node
inline bool udf_xilinx_recom_start_load_population(ArrayAccum<SumAccum<uint64_t> > arrayNumVectors, int64_t nodeId) {
    std::cout << "DEBUG: UDF START " << __FUNCTION__ << std::endl;

    xilRecom::Lock lock(xilRecom::getMutex());
    xilRecom::Context *pContext = xilRecom::Context::getInstance(); 
    uint ctxNodeId = pContext->getNodeId();

    if (nodeId != ctxNodeId) {
        std::cout << "ERROR: UDF nodeId " << nodeId << 
                  " does not match CosineSim nodeId " << ctxNodeId << std::endl;
        return false;
    }

    xilinx_apps::cosinesim::RowIndex numVectors = xilinx_apps::cosinesim::RowIndex(arrayNumVectors.data_[nodeId]);
    
    std::cout << "DEBUG: " << __FUNCTION__ << " nodeID=" << nodeId 
              << " numVectors=" << numVectors << std::endl;

    xilRecom::Context::IdMap &idMap = pContext->getIdMap();
    idMap.clear();
    idMap.resize(numVectors, 0);

    xilRecom::CosineSim *pCosineSim = pContext->getCosineSimObj();
    pCosineSim->startLoadPopulation(numVectors);

    std::cout << "DEBUG: UDF END " << __FUNCTION__ << std::endl;
    return true;
}

inline int udf_xilinx_recom_load_population_vector(ListAccum<int64_t> popVector, uint64_t id) {
    //std::cout << "DEBUG: UDF START " << __FUNCTION__ << std::endl;

    xilRecom::Lock lock(xilRecom::getMutex());
    xilRecom::Context *pContext = xilRecom::Context::getInstance();

    xilinx_apps::cosinesim::ColIndex vectorLength = xilinx_apps::cosinesim::ColIndex(
            xilinx_apps::cosinesim::ColIndex(popVector.size()));
    pContext->setVectorLength(vectorLength);

    xilRecom::Context::IdMap &idMap = pContext->getIdMap();
    uint ctxNodeId = pContext->getNodeId();
    xilRecom::CosineSim *pCosineSim = pContext->getCosineSimObj();

    std::cout << "DEBUG: UDF " << __FUNCTION__ << " nodeID=" << ctxNodeId 
              << " vectorLength=" << vectorLength << std::endl;

    xilinx_apps::cosinesim::RowIndex rowIndex = 0;
    xilRecom::CosineSim::ValueType *pBuf = pCosineSim->getPopulationVectorBuffer(rowIndex);
    for (xilinx_apps::cosinesim::ColIndex eltNum = 0; eltNum < vectorLength; ++eltNum)
        *pBuf++ = xilRecom::CosineSim::ValueType(popVector.get(eltNum));
    
    pCosineSim->finishCurrentPopulationVector(pBuf);
    idMap[rowIndex] = id;

    //std::cout << "DEBUG: UDF END " << __FUNCTION__ << std::endl;
    return 0;
}

//
inline bool udf_xilinx_recom_finish_load_population(int64_t nodeId) {
    std::cout << "DEBUG: UDF START " << __FUNCTION__ << std::endl;
    xilRecom::Lock lock(xilRecom::getMutex());
    xilRecom::Context *pContext = xilRecom::Context::getInstance();
    xilRecom::CosineSim *pCosineSim = pContext->getCosineSimObj();

    uint ctxNodeId = pContext->getNodeId();
    std::cout << "DEBUG: UDF START " << __FUNCTION__ << " nodeID=" << ctxNodeId << std::endl;

    if (nodeId != ctxNodeId) {
        std::cout << "ERROR: UDF nodeId " << nodeId << 
                  " does not match CosineSim nodeId " << ctxNodeId << std::endl;
        return false;
    }

    pCosineSim->finishLoadPopulationVectors();
    pContext->setInitialized();  // FPGA(s) now ready to match

    std::cout << "DEBUG: UDF END " << __FUNCTION__ << std::endl;
    return true;
}


inline int udf_xilinx_recom_load_population_vectors(ListAccum<ListAccum<int64_t> >& popVectors,
        ListAccum<uint64_t> &ids) {
    xilRecom::Lock lock(xilRecom::getMutex());
    xilRecom::Context *pContext = xilRecom::Context::getInstance();
    
    // Set the vector length up front, as changing it causes the context to be invalidated
    
    xilinx_apps::cosinesim::ColIndex vectorLength = xilinx_apps::cosinesim::ColIndex(
            xilinx_apps::cosinesim::ColIndex(popVectors.get(0).size()));
    pContext->setVectorLength(vectorLength);
    
    // If there are no vectors, consider the FPGA to be uninitialized
    
    xilinx_apps::cosinesim::RowIndex numVectors = xilinx_apps::cosinesim::RowIndex(popVectors.size());
    if (numVectors < 1) {
        pContext->clear();
        return 0;
    }

    // Make sure that the number of vectors and number of IDs match.  Error out if not
    
    if (numVectors != ids.size()) {
        std::cout << "ERROR: xilinxRecomEngine: number of population vectors " << vectorLength
                << " does not match number of IDs " << ids.size() << " in udf_xilinx_recom_load_population_vectors"
                << std::endl;
        return -2;
    }
    
    // Prepare the FPGA row to vertex ID map
    
    xilRecom::Context::IdMap &idMap = pContext->getIdMap();
    idMap.clear();
    idMap.resize(numVectors, 0);
    
    // Fill the FPGA(s) with vectors
    
    try {
        xilRecom::CosineSim *pCosineSim = pContext->getCosineSimObj();
    //    pCosineSim->openFpga();
        pCosineSim->startLoadPopulation(numVectors);
        for (xilinx_apps::cosinesim::RowIndex vecNum = 0; vecNum < numVectors; ++vecNum) {
            const ListAccum<int64_t> &curRowVec = popVectors.get(vecNum);
            xilinx_apps::cosinesim::RowIndex rowIndex = 0;
            xilRecom::CosineSim::ValueType *pBuf = pCosineSim->getPopulationVectorBuffer(rowIndex);
            for (xilinx_apps::cosinesim::ColIndex eltNum = 0; eltNum < vectorLength; ++eltNum)
                *pBuf++ = xilRecom::CosineSim::ValueType(curRowVec.get(eltNum));
            pCosineSim->finishCurrentPopulationVector(pBuf);
            idMap[rowIndex] = ids.get(vecNum);
        }
        pCosineSim->finishLoadPopulation();
        pContext->setInitialized();  // FPGA(s) now ready to match
        return 0;
    }
    catch (const xilinx_apps::cosinesim::Exception &ex) {
        // Write actual error to GPE log
        std::cout << "ERROR: xilinxRecomEngine: " << ex.what() << std::endl;
        return -1;
    }
}

// Enable this to print profiling messages to the log (via stdout)
#define XILINX_RECOM_PROFILE_ON

inline ListAccum<XilCosinesimMatch> udf_xilinx_recom_match_target_vector(int64_t topK, ListAccum<int64_t> targetVector)
{
    xilRecom::Lock lock(xilRecom::getMutex());
    ListAccum<XilCosinesimMatch> result;
    xilRecom::Context *pContext = xilRecom::Context::getInstance();

    if (!pContext->isInitialized())
        return result;
    xilRecom::Context::IdMap &idMap = pContext->getIdMap();

    uint ctxNodeId = pContext->getNodeId();
    std::cout << "DEBUG: UDF START " << __FUNCTION__ << " nodeID=" << ctxNodeId << std::endl;

    //-------------------------------------------------------------------------
#ifdef XILINX_RECOM_PROFILE_ON
    std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time =
            std::chrono::high_resolution_clock::now();
    std::cout << "PROFILING: " << __FILE__ << "::" << __FUNCTION__ 
              << " preprocessing start=" << l_start_time.time_since_epoch().count() 
              << std::endl;
#endif
    //-------------------------------------------------------------------------

    std::vector<xilRecom::CosineSim::ValueType> nativeTargetVector;
    const xilinx_apps::cosinesim::ColIndex vectorLength = pContext->getVectorLength();
    nativeTargetVector.reserve(vectorLength);
    for (xilinx_apps::cosinesim::ColIndex eltNum = 0; eltNum < vectorLength; ++eltNum)
        nativeTargetVector.push_back(targetVector.get(eltNum));
    
    try {
        xilRecom::CosineSim *pCosineSim = pContext->getCosineSimObj();

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

        for (xilinx_apps::cosinesim::Result &apiResult : apiResults) {
            if (apiResult.index < 0 || apiResult.index >= xilinx_apps::cosinesim::RowIndex(idMap.size()))
                continue;
            result += XilCosinesimMatch(VERTEX(idMap[apiResult.index]), apiResult.similarity);
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

    }
    catch (const xilinx_apps::cosinesim::Exception &ex) {
        std::cout << "ERROR: xilinxRecomEngine: " << ex.what() << std::endl;
//        pContext->clear();  // maybe don't do this if problem is transient
    }

    std::cout << "DEBUG: UDF END " << __FUNCTION__ << std::endl;
    return result;
}

/* End Xilinx Cosine Similarity Additions */
// mergeHeaders 1 section body end xilinxRecomEngine DO NOT REMOVE!

}

#endif /* EXPRFUNCTIONS_HPP_ */
