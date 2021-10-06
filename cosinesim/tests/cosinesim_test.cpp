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
#include <map>
#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>

using Element = std::int32_t;
using Vector = std::vector<Element>;
using CosineSim = xilinx_apps::cosinesim::CosineSim<Element>;
using Result = xilinx_apps::cosinesim::Result;
using ResultVector = std::vector<Result>;
using RowIndex = xilinx_apps::cosinesim::RowIndex;


inline Element generateRandomElement(Element minValue, Element maxValue) {
    return (minValue == maxValue) ? minValue : std::rand() % (maxValue - minValue) + minValue;
}


struct CosineSimVector {
    std::vector<Element> m_elements;
    double m_normal = 0.0;
    
    void setNormal() {
        m_normal = 0.0;
        for (auto eltInt : m_elements) {
            const double elt = eltInt;
            m_normal += elt * elt;
        }
        m_normal = std::sqrt(m_normal);
    }
    
    double cosineSimilarity(const CosineSimVector &other) const {
        double dotProduct = 0.0;
        for (unsigned i = 0, end = m_elements.size(); i < end; ++i)
            dotProduct = dotProduct + m_elements[i] * other.m_elements[i];
        double result = dotProduct / (m_normal * other.m_normal);
        return result;
    }
};


struct TestParams {
    Element m_minValue = 0;
    Element m_maxValue = 0;
    unsigned m_vectorLength = 0;
    unsigned m_numVectors = 0;
    unsigned m_numResults = 0;
    
    TestParams(Element minVal, Element maxVal, unsigned vecLen, unsigned numVecs, unsigned numResults)
        : m_minValue(minVal), m_maxValue(maxVal), m_vectorLength(vecLen), m_numVectors(numVecs),
          m_numResults(numResults)
    {}
};



void generateVectors(const TestParams &testParams, CosineSimVector &targetVec,
    std::vector<CosineSimVector> &populationVecs)
{
    // Create the target vector with random numbers whose range is limited only by the test parameter min and max values
    
    std::cout << "#### Generating target and population vectors" << std::endl;
    targetVec.m_elements.clear();
    for (unsigned i = 0; i < testParams.m_vectorLength; ++i)
        targetVec.m_elements.push_back(generateRandomElement(testParams.m_minValue, testParams.m_maxValue));
    targetVec.setNormal();
    
    // Create population vectors with random numbers constrained by an envelope which starts with the full range of
    // possible values (test param minValue to maxValue) for population vector 0, but linearly narrows the range
    // until the last population vector has exactly the same values as the target vector.  The envelope should cause
    // population vectors at the end of the population vector list to match more closely that the ones before them.
    
    populationVecs.clear();
    populationVecs.resize(testParams.m_numVectors);
    for (unsigned i = 0; i < testParams.m_numVectors; ++i) {
        CosineSimVector &curCosVec = populationVecs[i];
        Vector &vec = curCosVec.m_elements;
        // The allowable range for a population vector element is the full range, scaled down by how far down the
        // population vector list we are, centered on the corresponding target vector element.
        //
        // For example, if there are 10 population vectors, then the random range for population vector 0 will be
        // the full range, the range for population vector 1 will be 8/9 of the full range, and for
        // population vector 9, 0/9 of the full range.
        const Element range = (testParams.m_maxValue - testParams.m_minValue) * (testParams.m_numVectors - 1 - i)
            / (testParams.m_numVectors - 1);
        const Element halfRange = range / 2;
        
        // Fill the population vector
        for (Element targetElt : targetVec.m_elements) {
            // Determine lower and upper bounds to be the calculated range centered on the target element, not to
            // exceed the absolute lower and upper values set by testParams
            Element localMin = (targetElt - testParams.m_minValue < halfRange) ? testParams.m_minValue
                : targetElt - halfRange;
            Element localMax = (testParams.m_maxValue - targetElt < halfRange) ? testParams.m_maxValue
                : targetElt + halfRange;
            vec.push_back(generateRandomElement(localMin, localMax));
        }
        curCosVec.setNormal();
    }
}


class MatchHeap {
public:
    using Map = std::multimap<double, RowIndex>;
    
private:
    const unsigned m_maxSize;
    Map m_map;
    
public:
    MatchHeap(unsigned maxSize) : m_maxSize(maxSize) {}
    
    void addMatch(double score, RowIndex rowIndex) {
        if (m_map.size() >= m_maxSize) {
            if (score > m_map.begin()->first)
                m_map.erase(m_map.begin());
            else
                return;
        }
        
        m_map.insert(Map::value_type(score, rowIndex));
    }
    
    ResultVector toMatchVector() const {
        ResultVector mv(m_map.size(), Result(0.0, 0));
        unsigned vecIndex = mv.size() - 1;
        for (auto entry : m_map)
            mv[vecIndex--] = Result(entry.second, entry.first);
        return mv;
    }
};


ResultVector runSwCosineSim(const TestParams &testParams, const CosineSimVector &targetVec,
    const std::vector<CosineSimVector> &populationVecs)
{
    std::cout << "#### Running SW cosine similarity" << std::endl;
    MatchHeap heap(testParams.m_numResults);
    for (RowIndex rowNum = 0, end = populationVecs.size(); rowNum < end; ++rowNum) {
        double score = targetVec.cosineSimilarity(populationVecs[rowNum]);
        heap.addMatch(score, rowNum);
    }
    return heap.toMatchVector();
}


ResultVector runHwCosineSim(const TestParams &testParams, const CosineSimVector &targetVec,
    const std::vector<CosineSimVector> &populationVecs, unsigned numDevices, const std::string &deviceTypes)
{
    std::cout << "#### Running HW cosine similarity, numDevices=" << numDevices << std::endl;
    xilinx_apps::cosinesim::Options options;
    options.vecLength = testParams.m_vectorLength;
    options.numDevices = numDevices;
    options.deviceNames = deviceTypes;

    ResultVector results;
    try {
        CosineSim cosineSim(options);

        // Generate random vectors, writing each into the Alveo card

        std::cout << "======== Loading population vectors into Alveo card..." << std::endl;
        //cosineSim.openFpga();
        cosineSim.startLoadPopulation(testParams.m_numVectors);
        for (RowIndex vecNum = 0; vecNum < testParams.m_numVectors; ++vecNum) {
            CosineSim::ValueType *pBuf = cosineSim.getPopulationVectorBuffer(vecNum);
            CosineSim::ValueType *p = pBuf;
            for (Element value : populationVecs[vecNum].m_elements)
                *p++ = CosineSim::ValueType(value);
            cosineSim.finishCurrentPopulationVector(pBuf);
        }
        cosineSim.finishLoadPopulation();

        // Run the match in the FPGA

        std::cout << "======== Running match..." << std::endl;
        results = cosineSim.matchTargetVector(testParams.m_numResults, targetVec.m_elements.data());
    } 
    catch (const xilinx_apps::cosinesim::Exception &ex) {
        std::cout << "#### ERROR during Cosinesim Running:" << ex.what() << std::endl;
    }
    return results;
}


void printResultVector(const ResultVector &results) {
    std::cout << "Index   Similarity   Vector #" << std::endl;
    std::cout << "-----   ----------   --------" << std::endl;
    unsigned index = 0;
    for (const Result &result : results)
        std::cout << index++ << "       " << result.similarity << "       " << result.index << std::endl;
}


bool compareResultStructs(const Result &res1, const Result &res2) {
    if (res1.similarity > res2.similarity)
        return true;
    if (res1.similarity < res2.similarity)
        return false;
    return res1.index < res2.index;
}


bool areMatchesEqual(const ResultVector &refVec, const ResultVector &testVec) {
    bool isSuccess = true;
    // Copy the vectors and sort them in place
    ResultVector refVecSorted = refVec;
    std::sort(refVecSorted.begin(), refVecSorted.end(), compareResultStructs);
    ResultVector testVecSorted = testVec;
    std::sort(testVecSorted.begin(), testVecSorted.end(), compareResultStructs);

    if (refVec.size() != testVec.size()) {
        std::cout << "#### FAIL: Result vector sizes are different.  Reference:" << refVec.size() << ", Test:"
            << testVec.size() << std::endl;
        isSuccess = false;
    }
    else {
        // Find the "tail" of the vectors, which is the last m results, all of which have the same score.
        // The tails of refVec and testVec may not match if the tail would have been longer if not truncated
        // by numResults.  If the tails of refVec and testVec are of different lengths, we'll keep the shorter tail,
        // which also implies that the vectors won't match.
        unsigned tailStart = refVecSorted.size() - 1;
        double tailScore = refVecSorted[tailStart].similarity;
        while (tailStart > 0 && refVecSorted[tailStart].similarity == tailScore
                && testVecSorted[tailStart].similarity == tailScore)
            --tailStart;
        
        // Compare the non-tail results strictly (results must match exactly)
        for (unsigned i = 0; i < tailStart; ++i) {
            if (refVecSorted[i].index != testVecSorted[i].index
                    || std::fabs(refVecSorted[i].similarity - testVecSorted[i].similarity) > 0.000001)
//            if (refVecSorted[i].index != testVecSorted[i].index
//                    || refVecSorted[i].similarity != testVecSorted[i].similarity)
            {
                std::cout << "#### FAIL: Result vectors don't match at result # " << i << std::endl;
                isSuccess = false;
                break;
            }
        }
    }
    
    if (isSuccess)
        std::cout << "PASS" << std::endl;
    else {
        std::cout << "==========================================" << std::endl;
        std::cout << "Reference (SW model)" << std::endl;
        printResultVector(refVecSorted);
        std::cout << "------------------------------------------" << std::endl;
        std::cout << "Test (Alveo)" << std::endl;
        printResultVector(testVecSorted);
        std::cout << "==========================================" << std::endl;
    }
    return isSuccess;
}


void printUsage(const char *progName) {
    std::cout << progName << " [options]" << std::endl
        << "where 'options' is one or more of:" << std::endl
        << "  -d <numDevices>     the number of Alveo cards to use (default = 1)" << std::endl
        << "  -i <numIterations>  the number of times to run each hardware run (default = 1)" << std::endl
        << "  -t <deviceTypes>    a space-separated list of shell names (default = xilinx_u50_gen3x16_xdma_201920_3)" << std::endl
        << "  -h                  prints this help message" << std::endl;
}

static const TestParams s_testParamSet[] = {
    // min, max, vec len, num pop vecs, num results
//    {-8192, 8192, 200, 100, 15},  
    {-8192, 8192, 200, 200, 15},  
    {-8192, 8192, 200, 500, 15},  
    {-8192, 8192, 200, 1000, 15},  
    {-8192, 8192, 200, 2000, 15},  
    {-8192, 8192, 200, 5000, 15},  
};

// argv
// 1: NumDevices
// 2: Iterations
// 3: device names
int main(int argc, char **argv) {
    unsigned numDevices = 1;
    unsigned numIterations = 1;
    (void) numIterations;  // TODO: implement multiple runs on hardware
    std::string deviceTypes = "xilinx_u50_gen3x16_xdma_201920_3";

    int curArgNum = 1;
    while (curArgNum < argc) {
        std::string curArg(argv[curArgNum++]);
        if (curArg[0] == '-') {
            if (curArg == "-d") {
                if (curArgNum < argc) {
                    std::cout << "-d argument: " << argv[curArgNum] << std::endl;
                    numDevices = std::stoi(argv[curArgNum++]);
                }
                else {
                    std::cout << "ERROR: option -d requires an argument." << std::endl;
                    printUsage(argv[0]);
                    return 1;
                }
            }
            else if (curArg == "-i") {
                if (curArgNum < argc) {
                    std::cout << "-i argument: " << argv[curArgNum] << std::endl;
                    numIterations = std::stoi(argv[curArgNum++]);
                }
                else {
                    std::cout << "ERROR: option -i requires an argument." << std::endl;
                    printUsage(argv[0]);
                    return 1;
                }
            }
            else if (curArg == "-t") {
                if (curArgNum < argc)
                    deviceTypes = argv[curArgNum++];
                else {
                    std::cout << "ERROR: option -d requires an argument." << std::endl;
                    printUsage(argv[0]);
                    return 1;
                }
            }
            else if (curArg == "-h" || curArg == "-help" || curArg == "--help") {
                printUsage(argv[0]);
                return 0;
            }
        }
        
        else {
            std::cout << "ERROR: Unrecognized argument '" << curArg << "'." << std::endl
                << "This program takes no positional arguments." << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }


    std::srand(0x12345);
    const unsigned NumTests = sizeof(s_testParamSet)/sizeof(TestParams);
    unsigned numPassingTests = 0;
    for (unsigned testNum = 0; testNum < NumTests; ++testNum) {
        std::cout << "# TEST " << testNum << " ##############################################" << std::endl;
        CosineSimVector targetVec;
        std::vector<CosineSimVector> populationVecs;
        const TestParams &testParams = s_testParamSet[testNum];
        generateVectors(testParams, targetVec, populationVecs);
        ResultVector swResults = runSwCosineSim(testParams, targetVec, populationVecs);
        ResultVector hwResults = runHwCosineSim(testParams, targetVec, populationVecs, numDevices, deviceTypes);
        if (hwResults.size() == 0) {
            std::cout << "# TEST " << testNum << " failed: empty results returned ############" << std::endl;
            continue;
        }

        bool doResultsMatch = areMatchesEqual(swResults, hwResults);
        if (doResultsMatch)
            ++numPassingTests;
    }
    std::cout << std::endl << numPassingTests << '/' << NumTests << " tests passed" << std::endl;
    if (numPassingTests < NumTests) {
        std::cout << "FAIL" << std::endl;
        return 1;
    }

    std::cout << "PASS" << std::endl;
    return 0;
}
