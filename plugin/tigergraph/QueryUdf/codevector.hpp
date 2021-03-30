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
//extern t_time_point timer_start_time;

inline t_time_point &getTimerStartTime() {
    static t_time_point s_startTime;
    return s_startTime;
}

//class CodeToIdMap;
//extern CodeToIdMap* pMap;

const unsigned int startPropertyIndex = 3; // Start index of property in the
                                           // vector, 0, 1 and 2 are ressrved
                                           // for norm, id */

//extern std::vector<uint64_t> IDMap;

using Mutex = std::mutex;

//#define XILINX_RECOM_DEBUG_MUTEX

#ifdef XILINX_RECOM_DEBUG_MUTEX
struct Lock {
    using RealLock = std::lock_guard<Mutex>;
    RealLock lock_;
    
    Lock(Mutex &m) 
    : lock_(m)
    {
        std::cout << "MUTEX: " << (void *) (&m) << std::endl;
    }
};
#else
using Lock = std::lock_guard<Mutex>;
#endif

inline Mutex &getMutex() {
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


// Cormen/Knuth hash algo
//
inline std::uint64_t hash(std::uint64_t x) {
    static double A = 0.5 * (std::sqrt(5.0) - 1.0);
    static std::uint64_t S = std::floor(A * double(SnomedCode(1) << SnomedCodeNumBits));
    x = x * S;
    x = x >> (SnomedCodeNumBits - SnomedIdNumBits);
    x = x & ((std::uint64_t(1) << SnomedIdNumBits) - 1);
    return x;
}


/**
* Converts a set of codes into a vector of integers to use in a cosine
* similarity computation
*
* @param concept the SNOMED concept (an arbitrary unsigned integer) to which the
* codes belong
* @param vectorLength the number of elements to generate for the cosine vector
* @param codes the set of codes to encode into the output vector.  The vector
* does not need to be sorted
* @return a vector of integers to embed into the cosine similarity vector
*
* See
* https://confluence.xilinx.com/display/DBA/Mapping+SNOMED+CT+Codes+into+Vector
* for details.
*/

inline std::vector<CosineVecValue> makeCosineVector(SnomedConcept concept,
                                             unsigned vectorLength,
                                             const std::vector<SnomedCode>& codes) {
    Lock lock(getMutex());
    std::vector<CosineVecValue> outVec;
    outVec.reserve(vectorLength);
    const SnomedId numIds = SnomedId(1) << SnomedIdNumBits;

    // Create the set of buckets

    std::vector<Bucket> buckets;
    for (std::uint64_t i = 0; i < vectorLength; ++i) {
        SnomedId pos = SnomedId(i * numIds / vectorLength);
        SnomedId nextPos = SnomedId((i + 1) * numIds / vectorLength);
        SnomedId bucketStartId = pos;
        SnomedId bucketEndId = (nextPos >= numIds) ? numIds : nextPos;
        buckets.emplace_back(bucketStartId, bucketEndId, MinVecValue, MaxVecValue, NullVecValue);
    }

    // Fill the buckets with codes.  When a bucket is full, dump the summary of
    // the bucket to the vector.

    for (unsigned i = 0; i < codes.size(); ++i) {
        // Convert the code to an ID (small int)
        SnomedId id = SnomedId(xai::hash(codes[i]));

        // Determine which bucket the ID goes into.  If the ID is out of range
        // (because it wasn't accounted for when
        // IDs were reserved for the concept), ignore the ID/code.
        const unsigned bucketNum = id * vectorLength / numIds;
        if (bucketNum >= vectorLength)
            continue;

        // Add the ID to the bucket
        buckets[bucketNum].addId(id);
    }

    // Dump all buckets and return the vector

    for (const Bucket& bucket : buckets)
        outVec.push_back(bucket.getCosineVecVale());

    return outVec; // Move semantics
}


} // namespace

#endif /* CODEVECTOR_HPP */
