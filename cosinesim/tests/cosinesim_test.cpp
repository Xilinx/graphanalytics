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
#include <cstdio>
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>

// Enable this macro to include tests under development (positive test cases that we know fail)
#define DEV_TESTS 1

using Element = std::int32_t;
using Vector = std::vector<Element>;
using CosineSim = xilinx_apps::cosinesim::CosineSim<Element>;
using Result = xilinx_apps::cosinesim::Result;
using ResultVector = std::vector<Result>;
using RowIndex = xilinx_apps::cosinesim::RowIndex;

const double DoubleEqualityResolution = 0.001; // 0.000001;

static bool g_isVerbose = false;

struct TestParams {
    Element m_minValue = 0;
    Element m_maxValue = 0;
    unsigned m_vectorLength = 0;
    unsigned m_numVectors = 0;
    
    TestParams(Element minVal, Element maxVal, unsigned vecLen, unsigned numVecs)
        : m_minValue(minVal), m_maxValue(maxVal), m_vectorLength(vecLen), m_numVectors(numVecs)
    {}
};


static const TestParams s_testParamSet[] = {
//   min      max  vec len  num pop vecs
    {-8192,   8192,  200,     100},  // 0
    {-8192,   8192,  200,     200},  
    {-8192,   8192,  200,     500},  
    {-8192,   8192,  200,    1000},  
    {-8192,   8192,  200,    2000},  
    {-8192,   8192,  200,    5000},  // 5
    {100,    10000,  200,     100},
    {100,    10000,   50,     500},
    {100,    10000,   20,    1000},
    {100,    10000,   10,    2000},
    {100,    10000,    5,    5000},  // 10
    {100,    10000,  400,    5000},
    {100,    10000,  400,   20000},
    {100,    10000,  500,  100000},
    
#ifdef DEV_TESTS
    {1000,  100000,  200,     100},
#endif
};


// Number of results to request for each set of test parameters
static const unsigned s_numResultsList[] = {1, 5, 10, 20, 50, 100};


inline Element generateRandomElement(Element minValue, Element maxValue) {
    return (minValue == maxValue) ? minValue : std::rand() % (maxValue - minValue) + minValue;
}


inline double roundDouble(double d) {
    double rounded = std::round(d / DoubleEqualityResolution) * DoubleEqualityResolution;
//    std::printf("roundDouble: in=%.12g, out=%.12g\n", d, rounded);
    return rounded;
}


struct CosineSimVector {
    std::vector<Element> m_elements;
    double m_normal = 0.0;
    
    void setNormal() {
        std::int64_t inormal = 0;
        for (auto eltInt : m_elements) {
//            const double elt = eltInt;
            inormal += eltInt * eltInt;
        }
        m_normal = inormal;
        m_normal = std::sqrt(m_normal);
    }
    
    double cosineSimilarity(const CosineSimVector &other) const {
        std::int64_t dotProduct = 0.0;
        for (unsigned i = 0, end = m_elements.size(); i < end; ++i)
            dotProduct = dotProduct + m_elements[i] * other.m_elements[i];
        double result = double(dotProduct) / (m_normal * other.m_normal);
        return result;
    }
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
        //
        // Additionally, the amount that the range scales down doesn't reduce to less than 20% of the full range.
        // This restriction ensures that large data sets don't produce results that have scores that are all 1.
//        double rangeRatio = (testParams.m_numVectors - 1 - i) / (testParams.m_numVectors - 1);
//        if (rangeRatio < 0.2)
//            rangeRatio = 0.2;
        double rangeRatio = std::log10(testParams.m_numVectors - i);
        const Element range = Element((testParams.m_maxValue - testParams.m_minValue) * rangeRatio);
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


ResultVector runSwCosineSim(const TestParams &testParams, unsigned numResults, const CosineSimVector &targetVec,
    const std::vector<CosineSimVector> &populationVecs)
{
    std::cout << "#### Running SW cosine similarity" << std::endl;
    MatchHeap heap(numResults);
    for (RowIndex rowNum = 0, end = populationVecs.size(); rowNum < end; ++rowNum) {
        double score = targetVec.cosineSimilarity(populationVecs[rowNum]);
        heap.addMatch(score, rowNum);
    }
    return heap.toMatchVector();
}


void printResultVector(const ResultVector &results) {
    std::cout << "Index      Similarity      Vector #" << std::endl;
    std::cout << "-----   ----------------   ---------" << std::endl;
    unsigned index = 0;
    for (const Result &result : results)
        std::printf("%5d  %16.12g  %9ld\n", index++, result.similarity, result.index);
}


bool compareResultStructs(const Result &res1, const Result &res2) {
//    std::cout << "(" << res1.similarity << ", " << res1.index << ") < (" << res2.similarity << ", " << res2.index
//        << ") = ";
    if (res1.similarity > res2.similarity) {
//        std::cout << "true (test 1)" << std::endl;
        return true;
    }
    if (res1.similarity < res2.similarity) {
//        std::cout << "false (test 2)" << std::endl;
        return false;
    }
//    std::cout << (res1.index < res2.index ? "true" : "false") << " (test 3)" << std::endl;
    return res1.index < res2.index;
}


Result roundResult(const Result &r) {
    return Result(r.index, roundDouble(r.similarity));
}


bool areMatchesEqual(const ResultVector &refVec, const ResultVector &testVec) {
    bool isSuccess = true;
    
    // Check that we actually have results from testVec
    if (testVec.empty()) {
        std::cout << "#### FAIL: Test vector is empty." << std::endl;
        return false;
    }

    // Copy the vectors, round their elements, and sort them in place
    ResultVector refVecSorted = refVec;
    ResultVector::iterator dummyIter = refVecSorted.begin();
    std::transform(refVecSorted.begin(), refVecSorted.end(), dummyIter, roundResult);
    std::sort(refVecSorted.begin(), refVecSorted.end(), compareResultStructs);
    ResultVector testVecSorted = testVec;
    dummyIter = testVecSorted.begin();
    std::transform(testVecSorted.begin(), testVecSorted.end(), dummyIter, roundResult);
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
        unsigned tailStart = refVecSorted.size();
        double tailScore = refVecSorted[tailStart - 1].similarity;
        while (tailStart > 0 && refVecSorted[tailStart - 1].similarity == tailScore
                && testVecSorted[tailStart - 1].similarity == tailScore)
            --tailStart;
        std::cout << "INFO: Matching tails start at result # " << tailStart << std::endl;
        
        // Compare the non-tail results strictly (results must match exactly)
        for (unsigned i = 0; i < tailStart; ++i) {
//            if (refVecSorted[i].index != testVecSorted[i].index
//                    || std::fabs(refVecSorted[i].similarity - testVecSorted[i].similarity) > DoubleEqualityResolution)
            if (refVecSorted[i].index != testVecSorted[i].index
                    || refVecSorted[i].similarity != testVecSorted[i].similarity)
            {
                std::cout << "#### FAIL: Result vectors don't match at result # " << i << std::endl;
                std::cout << "     refVec.index=" << refVecSorted[i].index
                    << ", testVec.index=" << testVecSorted[i].index
                    << ", refVec.similarity=" << refVecSorted[i].similarity
                    << ", testVec.similarity=" << testVecSorted[i].similarity << std::endl;
                isSuccess = false;
                break;
            }
        }
    }
    
    if (!isSuccess || g_isVerbose) {
        std::cout << "= Original ===============================" << std::endl;
        std::cout << "Reference (SW model)" << std::endl;
        printResultVector(refVec);
        std::cout << "------------------------------------------" << std::endl;
        std::cout << "Test (Alveo)" << std::endl;
        printResultVector(testVec);
        std::cout << "= Final ==================================" << std::endl;
        std::cout << "Reference (SW model)" << std::endl;
        printResultVector(refVecSorted);
        std::cout << "------------------------------------------" << std::endl;
        std::cout << "Test (Alveo)" << std::endl;
        printResultVector(testVecSorted);
        std::cout << "==========================================" << std::endl;
    }
    if (isSuccess)
        std::cout << "#### PASS" << std::endl;
    return isSuccess;
}


void printUsage(const char *progName) {
    std::cout << progName << " [options]" << std::endl
        << "where 'options' is one or more of:" << std::endl
        << "  -d <numDevices>     the number of Alveo cards to use (default = 1)" << std::endl
        << "  -t <deviceTypes>    a space-separated list of shell names (default = xilinx_u50_gen3x16_xdma_201920_3)" << std::endl
        << "  -1 <testNum>        run one test of the given index (default = run all tests)" << std::endl
        << "  -n <numResults>     run each test with only the given numResults (default = run all numResults)" << std::endl
        << "  -v                  verbose: display extra info, such as results for passing tests" << std::endl
        << "  -h                  prints this help message" << std::endl;
}


int main(int argc, char **argv) {
    const unsigned NumTests = sizeof(s_testParamSet)/sizeof(TestParams);  // how many tests defined
    const unsigned NumNumResults = sizeof(s_numResultsList)/sizeof(unsigned);  // how many variations of numResults

    unsigned numDevices = 1;
    unsigned numIterations = 1;
    int singleTestNum = -1;  // < 0 means run all tests
    (void) numIterations;  // TODO: implement multiple runs on hardware
    std::string deviceTypes = "xilinx_u50_gen3x16_xdma_201920_3";
    int userNumResults = -1;  // < 0 means all values of numResults

    int curArgNum = 1;
    while (curArgNum < argc) {
        std::string curArg(argv[curArgNum++]);
        if (curArg[0] == '-') {
            if (curArg == "-d") {
                if (curArgNum < argc) {
//                    std::cout << "-d argument: " << argv[curArgNum] << std::endl;
                    numDevices = std::stoi(argv[curArgNum++]);
                }
                else {
                    std::cout << "ERROR: option -d requires an argument." << std::endl;
                    printUsage(argv[0]);
                    return 2;
                }
            }
            else if (curArg == "-t") {
                if (curArgNum < argc)
                    deviceTypes = argv[curArgNum++];
                else {
                    std::cout << "ERROR: option -d requires an argument." << std::endl;
                    printUsage(argv[0]);
                    return 2;
                }
            }
            else if (curArg == "-1") {
                if (curArgNum < argc) {
//                    std::cout << "-1 argument: " << argv[curArgNum] << std::endl;
                    singleTestNum = std::stoi(argv[curArgNum++]);
                    if (singleTestNum >= int(NumTests)) {
                        std::cout << "ERROR: test number " << singleTestNum << " doesn't exist." << std::endl;
                        std::cout << "Valid range is 0 to " << NumTests - 1 << std::endl;
                        return 2;
                    }
                }
                else {
                    std::cout << "ERROR: option -1 requires an argument." << std::endl;
                    printUsage(argv[0]);
                    return 2;
                }
            }
            else if (curArg == "-n") {
                if (curArgNum < argc) {
//                    std::cout << "-n argument: " << argv[curArgNum] << std::endl;
                    userNumResults = std::stoi(argv[curArgNum++]);
                    if (userNumResults > 100 || userNumResults == 0) {
                        std::cout << "ERROR: numResults " << singleTestNum << " is invalid." << std::endl;
                        std::cout << "Valid range is 1 to 100" << std::endl;
                        return 2;
                    }
                }
                else {
                    std::cout << "ERROR: option -1 requires an argument." << std::endl;
                    printUsage(argv[0]);
                    return 2;
                }
            }
            else if (curArg == "-v") {
                g_isVerbose = true;
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
            return 2;
        }
    }


    std::srand(0x12345);
    unsigned numPassingTests = 0;
    std::vector<bool> testResults(NumTests, false);  // whether each test passed (true) or failed (false)
    const unsigned startTestNum = (singleTestNum >= 0) ? unsigned(singleTestNum) : 0;
    const unsigned endTestNum = (singleTestNum >= 0) ? unsigned(singleTestNum) : NumTests - 1;
    const unsigned NumRuns = (userNumResults > 0) ? 1 : NumNumResults;
    for (unsigned testNum = startTestNum; testNum <= endTestNum ; ++testNum) {
        std::cout << "#####################################################" << std::endl;
        std::cout << "# TEST " << testNum << std::endl;

        CosineSimVector targetVec;
        std::vector<CosineSimVector> populationVecs;
        const TestParams &testParams = s_testParamSet[testNum];
        std::cout << "minValue = " << testParams.m_minValue
            << ", maxValue = " << testParams.m_maxValue
            << ", vectorLength = " << testParams.m_vectorLength
            << ", numVectors = " << testParams.m_numVectors
            << std::endl;
        std::cout << "#####################################################" << std::endl;
        
        generateVectors(testParams, targetVec, populationVecs);
        
        try {
            std::cout << "#### Running HW cosine similarity, numDevices=" << numDevices << std::endl;
            xilinx_apps::cosinesim::Options options;
            options.vecLength = testParams.m_vectorLength;
            options.numDevices = numDevices;
            options.deviceNames = deviceTypes;

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

            // Run the matches in the SW model and FPGA, once per numResults value
            
            unsigned numPassingRuns = 0;
            for (unsigned numResultsIndex = 0; numResultsIndex < NumRuns; ++numResultsIndex) {
                const unsigned numResults = (userNumResults > 0) ? userNumResults : s_numResultsList[numResultsIndex];
                std::cout << "#### RUN " << numResultsIndex << ": numResults = " << numResults
                        << " #############################" << std::endl;
                
                // Run the match in the SW model
                ResultVector swResults = runSwCosineSim(testParams, numResults, targetVec, populationVecs);

                // Run the match in the FPGA
                std::cout << "======== Running match..." << std::endl;
                ResultVector hwResults = cosineSim.matchTargetVector(numResults, targetVec.m_elements.data());

                // Check whether the SW and HW results match and record the test results
                bool doResultsMatch = areMatchesEqual(swResults, hwResults);
                if (doResultsMatch)
                    ++numPassingRuns;
            }
            
            // The whole test passes if all the runs pass
            testResults[testNum] = (numPassingRuns == NumRuns);
            if (numPassingRuns == NumRuns)
                ++numPassingTests;
        }
        catch (const xilinx_apps::cosinesim::Exception &ex) {
            std::cout << "#### FAIL: Error during HW cosinesim execution: " << ex.what() << std::endl;
            testResults.push_back(false);
        }
    }

    std::cout << std::endl;
    std::cout << "#####################################################" << std::endl;
    std::cout << "# SUMMARY" << std::endl;
    std::cout << "#####################################################" << std::endl;
    for (unsigned i = startTestNum; i <= endTestNum; ++i)
        std::cout << "Test " << i << ": " << (testResults[i] ? "PASS" : "FAIL") << std::endl;
    const unsigned NumTestsRan = (singleTestNum >= 0 ? 1 : NumTests);
    std::cout << std::endl << numPassingTests << '/' << NumTestsRan << " tests passed" << std::endl;
    if (numPassingTests < NumTestsRan) {
        std::cout << "FAIL" << std::endl;
        return 1;
    }

    std::cout << "PASS" << std::endl;
    return 0;
}
