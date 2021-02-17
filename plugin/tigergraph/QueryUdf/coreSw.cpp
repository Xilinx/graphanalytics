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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include <iostream>
#include "codevector.hpp"

// *NEW*
#include "core.hpp"
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <mutex>

namespace {

using namespace xai;

using Mutex = std::mutex;
using Lock = std::lock_guard<Mutex>;

Mutex &getMutex() {
    static Mutex *pMutex = nullptr;
    if (pMutex == nullptr)
        pMutex = new Mutex();
    return *pMutex;
}

}

//#####################################################################################################################

namespace xai {

const unsigned int startPropertyIndex = 3; // Start index of property in the
                                           // vector, 0, 1 and 2 are ressrved
                                           // for norm, id */
CodeToIdMap* pMap = nullptr;
std::vector<uint64_t> IDMap;
std::int64_t abs64(std::int64_t x) {
    return (x >= 0) ? x : -x;
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

std::vector<CosineVecValue> makeCosineVector(SnomedConcept concept,
                                             unsigned vectorLength,
                                             const std::vector<SnomedCode>& codes) {
    Lock lock(getMutex());
//    std::cout << "makeCosineVector: start" << std::endl;
    std::vector<CosineVecValue> outVec;
    outVec.reserve(vectorLength);
//    std::cout << "makeCosineVector: after reserve" << std::endl;
    CodeToIdMap* pIdMap = CodeToIdMap::getInstance();
    // const SnomedId numIds = pIdMap->getNumReservedIds(concept);
    SnomedId numIds = pIdMap->getNumReservedIds(concept);
    if (numIds < vectorLength) numIds = vectorLength;

    // Create the set of buckets

//    std::cout << "makeCosineVector: before buckets" << std::endl;
    std::vector<Bucket> buckets;
    for (std::uint64_t i = 0; i < vectorLength; ++i) {
        SnomedId pos = SnomedId(i * numIds / vectorLength);
        SnomedId nextPos = SnomedId((i + 1) * numIds / vectorLength);
        SnomedId bucketStartId = pos;
        SnomedId bucketEndId = (nextPos >= numIds) ? numIds : nextPos;
        buckets.emplace_back(bucketStartId, bucketEndId, MinVecValue, MaxVecValue, NullVecValue);
    }
//    std::cout << "makeCosineVector: after buckets" << std::endl;
    // Fill the buckets with codes.  When a bucket is full, dump the summary of
    // the bucket to the vector.

    for (unsigned i = 0; i < codes.size(); ++i) {
        // Convert the code to an ID (small int)
        SnomedId id = pIdMap->addCode(concept, codes[i]);

        // Determine which bucket the ID goes into.  If the ID is out of range
        // (because it wasn't accounted for when
        // IDs were reserved for the concept), ignore the ID/code.
        const unsigned bucketNum = id * vectorLength / numIds;
        if (bucketNum >= vectorLength) continue;

        // Add the ID to the bucket
        buckets[bucketNum].addId(id);
    }
    // Dump all buckets and return the vector
//    std::cout << "makeCosineVector: after add codes" << std::endl;

    for (const Bucket& bucket : buckets) outVec.push_back(bucket.getCosineVecVale());
//    std::cout << "makeCosineVector: end" << std::endl;

    return outVec; // Move semantics
}


// *NEW*

PatientVectorMap *get_patient_vector_map(bool isDelete = false) {
    static PatientVectorMap *sp_map = nullptr;
    if (isDelete) {
        delete sp_map;
        sp_map = nullptr;
    }
    else if (sp_map == nullptr)
        sp_map = new PatientVectorMap();
    return sp_map;
}


}  // namespace xai
