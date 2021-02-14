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
#include "codevector.hpp"
#include "core.hpp"
#include <algorithm>
#include <functional>

namespace UDIMPL {

/* Start Xilinx UDF additions */


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
    return true;
}

inline double udf_elapsed_time(bool dummy) {
    return 1;
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
    std::cout << "val = " << res << std::endl;
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

// *NEW*
inline void udf_clear_cosinesim_vector_cache() {
    xai::get_patient_vector_map(true);  // true means delete the singleton map
}

inline bool udf_update_cosinesim_vector_cache(uint64_t patientId, const ListAccum<int64_t> &patientVec) {
    xai::PatientVectorMap *const pMap = xai::get_patient_vector_map();
    xai::CacheEntry &entry = (*pMap)[xai::PatientId(patientId)];
    int64_t normInt = patientVec.get(0);
    entry.norm_ = *reinterpret_cast<double *>(&normInt);
    entry.patientId_ = patientId;
    entry.vector_.resize(patientVec.size());
    for (std::size_t i = 0, end = patientVec.size(); i < end; ++i)
        entry.vector_[i] = patientVec.get(i);
    exit(1);  // TODO: remove this when done testing
#ifdef DEBUG_VALUES
    std::vector<xai::CosineVecValue> &testVec = (*pMap)[xai::PatientId(patientId)];
    std::cout << "PatientId:" << patientId << ", [";
    bool isFirst = true;
    for (auto val : testVec) {
        if (isFirst)
            isFirst = false;
        else
            std::cout << ", ";
        std::cout << val;
    }
    std::cout << "]" << std::endl;
#endif
    return true;
}

inline double udf_cos_theta_entry_list(const xai::CacheEntry &entry, const ListAccum<int64_t> &vec_B) {
    double res;
    int size = entry.vector_.size();
    int64_t norm_B = vec_B.get(0);
    double norm_d_B = *(reinterpret_cast<double*>(&norm_B));
    double prod = 0;
    int i = xai::startPropertyIndex;
    while (i < size) {
        prod = prod + entry.vector_[i] * vec_B.get(i);
        ++i;
    }
    res = prod / (entry.norm_ * norm_d_B);
    std::cout << "norm_d_A = " << entry.norm_ << ", norm_d_B = " << norm_d_B << ", val = " << res << std::endl;
    return res;
}

inline ListAccum<testResults> udf_cosinesim_sw(uint64_t numResults, const ListAccum<int64_t> &newPatientVec)
{
    HeapAccum<xai::CosineSimResult, xai::CosineSimResult::Comparator> heap;
    heap.resize(numResults);
    xai::PatientVectorMap *const pMap = xai::get_patient_vector_map();
    for (auto entry : *pMap) {
        double similarity = udf_cos_theta_entry_list(entry.second, newPatientVec);
        heap += xai::CosineSimResult(entry.first, similarity);
    }
    
    ListAccum<testResults> results;
    for (xai::CosineSimResult res : heap)
        results += testResults(VERTEX(res.patientId_), res.similarity_);
    return results;
}

/* End Xilinx Cosine Similarity Additions */
}

#endif /* EXPRFUNCTIONS_HPP_ */
