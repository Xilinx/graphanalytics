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


#ifndef _XILINX_TG_CORE_HPP_
#define _XILINX_TG_CORE_HPP_

#include "codevector.hpp"
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <functional>

namespace xai {

using PatientId = std::uint64_t;

struct CacheEntry {
    double norm_ = 0.0;
    PatientId patientId_ = 0;
    std::vector<CosineVecValue> vector_;
    
    CacheEntry() = default;
};

using PatientVectorMap = std::unordered_map<PatientId, CacheEntry>;
using CosineSimilarity = float;


struct CosineSimResult {
    PatientId patientId_ = 0;
    CosineSimilarity similarity_ = 0;
    
    CosineSimResult() = default;
    CosineSimResult(PatientId patientId, CosineSimilarity similarity)
    : patientId_(patientId), similarity_(similarity)
    {}
    
    bool operator==(const CosineSimResult &rhs) const { return similarity_ == rhs.similarity_; }
    bool operator<(const CosineSimResult &rhs) const { return similarity_ < rhs.similarity_; }
    
    // Hack for HeapAccum needing comparator whose constructor takes a bool (?)
    struct Comparator {
        Comparator(bool) {}
        bool operator()( const CosineSimResult & lhs, const CosineSimResult & rhs ) const { return lhs < rhs; }
    };
};


extern PatientVectorMap *get_patient_vector_map(bool isDelete = false);

}  // namespace xai

#endif // _XILINX_TG_CORE_HPP_