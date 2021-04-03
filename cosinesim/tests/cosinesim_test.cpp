
// Compile with:
// g++ cppdemo.cpp --std=c++11

// Use a temporary implementation of CosineSim until real API is ready
//#define USE_LOCAL_CLASS

#ifndef USE_LOCAL_CLASS
#include "cosinesim.hpp"
#endif

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <iostream>
#include <string>

#ifdef USE_LOCAL_CLASS
namespace xilinx_apps {
namespace cosinesim {
template <typename Value>
class CosineSim {
public:
    using RowIndex = std::int64_t;
    using ColIndex = std::int32_t;
    using ValueType = Value;
    
    struct Result {
        RowIndex index = -1L;
        double similarity = 0.0;
        
        Result(RowIndex index, double similarity) {
            index = index;
            similarity = similarity;
        }
    };
    
    struct Options {
        
    };
    
    CosineSim(ColIndex vecLength, const Options &options)
    : vecLength_(vecLength), options_(options) {}

    ColIndex getVectorLength() const { return vecLength_; }
    
    void openFpga() {}
    void startLoadPopulation() { numRows_ = 0; }
    Value *getPopulationVectorBuffer(RowIndex &rowIndex) {
        static std::vector<Value> s_buf;
        s_buf.resize(vecLength_, 0);
        rowIndex = numRows_++;
        return s_buf.data();
    }
    void finishCurrentPopulationVector() {}
    void finishLoadPopulation() {}
    
    std::vector<Result> matchTargetVector(unsigned numResults, const Value *elements) { return std::vector<Result>(); }
    void closeFpga() {}
    
private:
    Options options_;
    ColIndex vecLength_ = 0;
    RowIndex numRows_ = 0;
};

}  // namespace cosinesim
}  // namespace xilinx_apps

#endif

const unsigned VectorLength = 200;
const unsigned NumVectors = 5000;
const int MaxValue = 16383;

using CosineSim = xilinx_apps::cosinesim::CosineSim<std::int32_t>;

int main(int argc, char **argv) {
    std::srand(0x12345);
    std::vector<CosineSim::ValueType> testVector;  // "new vector" to match
    
    // Create the CosineSim object

    xilinx_apps::cosinesim::Options options;
    options.vecLength = VectorLength;
    if (argc > 1)
        options.numDevices = std::stoi(argv[1]);
    else
        options.numDevices = 1;

    std::cout << "-------- START COSINESIME TEST numDevices=" << options.numDevices << "----------" << std::endl;

    //user can set xclbinPath through jsonPath
    //options.jsonPath = "Debug/config_cosinesim_ss_dense_fpga.json";

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
    cosineSim.finishLoadPopulationVectors();
    
    // Run the match in the FPGA
    
    std::cout << "INFO: Running match for test vector #" << testVectorIndex << "..." << std::endl;
    std::vector<xilinx_apps::cosinesim::Result> results = cosineSim.matchTargetVector(10, testVector.data());
    results.clear();
    results = cosineSim.matchTargetVector(10, testVector.data());
    
    // Display the results
    
    std::cout << "INFO: Results:" << std::endl;
    std::cout << "INFO: Similarity   Vector #" << std::endl;
    std::cout << "----------   --------" << std::endl;
    for (xilinx_apps::cosinesim::Result &result : results)
        std::cout << result.similarity << "       " << result.index << std::endl;
    
    return 0;
}
