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


#include <cstdlib>
#include <cstdint>
#include <vector>
#include <iostream>
#include "cosinesim.hpp"


int main(int argc, char **argv) {
    const unsigned VectorLength = 200;
    const unsigned NumVectors = 5000;
    const int MaxValue = 16383;

    using CosineSim = xilinx_apps::cosinesim::CosineSim<std::int32_t>;

    std::srand(0x12345);
    std::vector<CosineSim::ValueType> testVector;  // "new vector" to match
    
    // Create the CosineSim object
    
    xilinx_apps::cosinesim::Options options;
    options.vecLength = VectorLength;
    options.numDevices = 1;
   
    xilinx_apps::cosinesim::CosineSim<std::int32_t> cosineSim(options);
    
    // Pick an index at random out of all the old vectors to use as the test vector to match
    
    const unsigned testVectorIndex = std::rand() % NumVectors;
    
    // Generate random vectors, writing each into the Alveo card
    
    std::cout << "Loading population vectors into Alveo card..." << std::endl;
    // Before loading population vector, call startLoadPopulation() to do the initialization;
    cosineSim.startLoadPopulation(NumVectors);
    // For each vector loading, call getPopulationVectorBuffer() to get the internal population vector buffer pointer and user write the vector into it;
    // At the end of each vector loading, call finishCurrentPopulationVector() for padding.
    for (unsigned vecNum = 0; vecNum < NumVectors; ++vecNum) {
    	xilinx_apps::cosinesim::RowIndex rowIndex = 0;
    	CosineSim::ValueType *pBuf = cosineSim.getPopulationVectorBuffer(rowIndex);
    	CosineSim::ValueType *p = pBuf;
        for (unsigned eltNum = 0; eltNum < VectorLength; ++eltNum) {
            const CosineSim::ValueType value = CosineSim::ValueType(std::rand() % MaxValue - (MaxValue / 2));
            *p++ = value;
            
            // If we've reached the index we've chosen as the test vector, save the test vector values
            if (vecNum == testVectorIndex)
                testVector.push_back(value);
        }
        cosineSim.finishCurrentPopulationVector(pBuf);
    }

    // After the whole population vectors loading finish, call finishLoadPopulationVectors();
    cosineSim.finishLoadPopulationVectors();
    
    // Run the match in the FPGA
    
    std::cout << "Running match for test vector #" << testVectorIndex << "..." << std::endl;
    std::vector<xilinx_apps::cosinesim::Result> results = cosineSim.matchTargetVector(10, testVector.data());
    results.clear();
    results = cosineSim.matchTargetVector(10, testVector.data());
    
    // Display the results
    
    std::cout << "Results:" << std::endl;
    std::cout << "Similarity   Vector #" << std::endl;
    std::cout << "----------   --------" << std::endl;
    for (xilinx_apps::cosinesim::Result &result : results)
        std::cout << result.similarity << "       " << result.index << std::endl;
    
    return 0;
}
