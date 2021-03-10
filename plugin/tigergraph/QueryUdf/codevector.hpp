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
/*
 * File:   CodeVector.hpp
 * Author: dliddell
 *
 * Created on December 24, 2019, 5:08 PM
 */

#ifndef CODEVECTOR_HPP
#define CODEVECTOR_HPP

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <chrono>

namespace xai {

typedef std::chrono::time_point<std::chrono::high_resolution_clock> t_time_point, *pt_time_point; 
extern t_time_point timer_start_time;

class CodeToIdMap;
extern CodeToIdMap* pMap;
extern const unsigned int startPropertyIndex;
extern std::vector<uint64_t> IDMap;

using Mutex = std::mutex;
using Lock = std::lock_guard<Mutex>;

static Mutex &getMutex() {
    static Mutex *pMutex = nullptr;
    if (pMutex == nullptr)
        pMutex = new Mutex();
    return *pMutex;
}


typedef std::int32_t CosineVecValue; ///< A value for an element of a cosine similarity vector
typedef std::uint64_t SnomedCode;    ///< A SNOMED CT medical code
typedef std::uint64_t SnomedId;  ///< SNOMED CT code as a hashed int (to distribute SNOMED codes uniformly)
const unsigned SnomedCodeNumBits = 60;  ///< Number of significant bits in a SnomedCode (enough for 18 decimal digits)
const unsigned SnomedIdNumBits = 32;  ///< Number of significant bits in a SnomedId
typedef unsigned SnomedConcept;      ///< Integer that distinguishes one concept from
                                     /// another (allergy vs. treatment, etc.)

// Standard values for cosine vector elements

#ifdef UNIT_TEST
const CosineVecValue MaxVecValue = 11000;
const CosineVecValue MinVecValue = 10000;
const CosineVecValue NullVecValue = -10000;
#else
const CosineVecValue MaxVecValue = 10480;
const CosineVecValue MinVecValue = MaxVecValue / 2;
const CosineVecValue NullVecValue = -MaxVecValue;
#endif


class Bucket {
    SnomedId m_bucketStartId;
    SnomedId m_bucketEndId;
    CosineVecValue m_startValue;
    CosineVecValue m_endValue;
    CosineVecValue m_nullValue;
    std::int64_t m_accumulator = 0; // sum of all codes added
    int m_codeCount = 0;            // how many codes were added
    bool m_isDone = false;          // true if addCode was called with a code >= m_bucketEndCode

   public:
    Bucket(SnomedId bucketStartCode,
           SnomedId bucketEndCode,
           CosineVecValue startValue,
           CosineVecValue endValue,
           CosineVecValue nullValue)
        : m_bucketStartId(bucketStartCode),
          m_bucketEndId(bucketEndCode),
          m_startValue(startValue),
          m_endValue(endValue),
          m_nullValue(nullValue) {}

    /**
     * Attempt to add the given code to the bucket.  If the code is within the
     * bucket's range, accumulate the code
     * into the bucket average.  Otherwise, just ignore the request.
     *
     * @param code the code to add
     */
    void addId(SnomedId id) {
        if (id >= m_bucketEndId)
            m_isDone = true;
        else if (id >= m_bucketStartId) {
            m_accumulator += std::int64_t(id);
            ++m_codeCount;
        }
    }

    /**
     * Returns the average of all added codes, scaled to the active range, or
     * returns the null value if no codes have
     * been added
     *
     * @return the CosineVecValue for all added codes
     */
    CosineVecValue getCosineVecVale() const {
        if (m_codeCount == 0) return m_nullValue;

        double average = double(m_accumulator) / m_codeCount;
        CosineVecValue val = CosineVecValue((average - m_bucketStartId) * (m_endValue - m_startValue) /
                                            (m_bucketEndId - m_bucketStartId)) +
                             m_startValue;
        return val;
    }
};

std::vector<CosineVecValue> makeCosineVector(SnomedConcept concept,
                                             unsigned vectorLength,
                                             const std::vector<SnomedCode>& codes);

} // namespace

#endif /* CODEVECTOR_HPP */
