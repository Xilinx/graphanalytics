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

#ifndef SYNTHEA_DEMO_HPP
#define SYNTHEA_DEMO_HPP

// mergeHeaders 1 name syntheaDemo

// mergeHeaders 1 section include start syntheaDemo DO NOT REMOVE!
#include "codevector.hpp"
#include <algorithm>
// mergeHeaders 1 section include end syntheaDemo DO NOT REMOVE!

//#####################################################################################################################

namespace UDIMPL {

// mergeHeaders 1 section body start syntheaDemo DO NOT REMOVE!

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
    syntheaDemo::getTimerStartTime() = std::chrono::high_resolution_clock::now();
    return true;
}

inline double udf_elapsed_time(bool dummy) {
    syntheaDemo::t_time_point cur_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = cur_time - syntheaDemo::getTimerStartTime();
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
    int i = syntheaDemo::startPropertyIndex;
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
    std::vector<int> retcodes = syntheaDemo::makeCosineVector(property, returnVecLength, codes);
    for (int value : retcodes) {
        result += value;
    }
    return result;
}

// mergeHeaders 1 section body end syntheaDemo DO NOT REMOVE!

}  // namespace UDIMPL
#endif