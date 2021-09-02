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


// Compile with:
// g++ cppdemo.cpp --std=c++11

// Use a temporary implementation of CosineSim until real API is ready
//#define USE_LOCAL_CLASS


#include "cosinesim.hpp"
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <iostream>
#include <string>


using CosineSim = xilinx_apps::cosinesim::CosineSim<std::int32_t>;

// argv
// 1: NumDevices
// 2: Iterations
// 3: NumVectors
int main(int argc, char **argv) {
    const unsigned VectorLength = 200;
    
    const int MaxValue = 16383;
    unsigned NumDevices = 1;
    unsigned Iterations = 1;
    unsigned NumVectors = 5000;
    std::string deviceNames = "xilinx_u50_gen3x16_xdma_201920_3";

    if (argc > 1)
        NumDevices = std::stoi(argv[1]);

    if (argc > 2) {
        Iterations = std::stoi(argv[2]);
    }

    if (argc > 3) {
        NumVectors = std::stoi(argv[3]);
    } 

    if (argc > 4) {
        deviceNames = argv[4];
    } 



    std::cout << "INFO: Running " << Iterations << " iterations with NumVectors=" << NumVectors << std::endl;

    std::srand(0x12345);
    std::vector<CosineSim::ValueType> testVector;  // "new vector" to match
    
    // Create the CosineSim object

    xilinx_apps::cosinesim::Options options;
    options.vecLength = VectorLength;
    options.numDevices = NumDevices;
    options.deviceNames = deviceNames;

    std::cout << "-------- START COSINESIME TEST numDevices=" << options.numDevices << "----------" << std::endl;

    //user can set xclbinPath through jsonPath
    //options.jsonPath = "Debug/config_cosinesim_ss_dense_fpga.json";
    std::vector<xilinx_apps::cosinesim::Result> results;
    try {
        xilinx_apps::cosinesim::CosineSim<std::int32_t> cosineSim(options);

        // Pick an index at random out of all the old vectors to use as the test vector to match

        const int testVectorIndex = std::rand() % NumVectors;

        // Generate random vectors, writing each into the Alveo card

        std::cout << "INFO: Loading population vectors into Alveo card..." << std::endl;
        //cosineSim.openFpga();
        cosineSim.startLoadPopulation(NumVectors);
        for (unsigned vecNum = 0; vecNum < NumVectors; ++vecNum) {
            xilinx_apps::cosinesim::RowIndex rowIndex = 0;
            CosineSim::ValueType *pBuf = cosineSim.getPopulationVectorBuffer(rowIndex);
            CosineSim::ValueType *p = pBuf;
            for (unsigned eltNum = 0; eltNum < VectorLength; ++eltNum) {
                const CosineSim::ValueType value = CosineSim::ValueType(std::rand() % MaxValue - (MaxValue / 2));
                *p++ = value;

                // If we've reached the index we've chosen as the test vector, save the test vector values
                if ((int)vecNum == testVectorIndex)
                    testVector.push_back(value);
            }
            cosineSim.finishCurrentPopulationVector(pBuf);
        }
        cosineSim.finishLoadPopulation();

        // Run the match in the FPGA

        std::cout << "INFO: Running match for test vector #" << testVectorIndex << "..." << std::endl;
        results = cosineSim.matchTargetVector(10, testVector.data());
        for (unsigned runCount = 0; runCount < Iterations; ++runCount) {
   	        std::cout << "-------- Run " << runCount << std::endl;
            results.clear();
            results = cosineSim.matchTargetVector(10, testVector.data());
        }
    } 
    catch (const xilinx_apps::cosinesim::Exception &ex) {
        std::cout << "Error during Cosinesim Running:" << ex.what() << std::endl;
        return -1;
    }
    // Display the results
    
    std::cout << "INFO: Results:" << std::endl;
    std::cout << "INFO: Similarity   Vector #" << std::endl;
    std::cout << "----------   --------" << std::endl;
    for (xilinx_apps::cosinesim::Result &result : results)
        std::cout << result.similarity << "       " << result.index << std::endl;
    
    return 0;
}
