
// Compile with:
// g++ cppdemo.cpp --std=c++11

// Use a temporary implementation of CosineSim until real API is ready
#define USE_LOCAL_CLASS

#ifndef USE_LOCAL_CLASS
#include "cosinesim.hpp"
#endif
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <iostream>

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
        RowIndex index_ = -1L;
        double similarity_ = 0.0;
        
        Result(RowIndex index, double similarity) {
            index_ = index;
            similarity_ = similarity;
        }
    };
    
    struct Options {
        
    };
    
    CosineSim(ColIndex vecLength, const Options &options)
    : vecLength_(vecLength), options_(options) {}

    ColIndex getVectorLength() const { return vecLength_; }
    
    void openFpga() {}
    void startLoadOldVectors() { numRows_ = 0; }
    Value *getOldVectorBuffer(RowIndex &rowIndex) {
        static std::vector<Value> s_buf;
        s_buf.resize(vecLength_, 0);
        rowIndex = numRows_++;
        return s_buf.data();
    }
    void finishCurrentOldVector() {}
    void finishLoadOldVectors() {}
    
    std::vector<Result> matchNewVector(unsigned numResults, const Value *elements) { return std::vector<Result>(); }
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
    
    CosineSim::Options options;
    // options.readJson("options.json");  // Just an idea, if the team thinks we should keep JSON support
    xilinx_apps::cosinesim::CosineSim<std::int32_t> cosineSim(VectorLength, options);
    
    // Pick an index at random out of all the old vectors to use as the test vector to match
    
    const int testVectorIndex = std::rand() % NumVectors;
    
    // Generate random vectors, writing each into the Alveo card
    
    std::cout << "Loading old vectors into Alveo card..." << std::endl;
    cosineSim.openFpga();
    cosineSim.startLoadOldVectors();
    for (unsigned vecNum = 0; vecNum < NumVectors; ++vecNum) {
        CosineSim::RowIndex rowIndex = 0;
        CosineSim::ValueType *pBuf = cosineSim.getOldVectorBuffer(rowIndex);
        for (unsigned eltNum = 0; eltNum < VectorLength; ++eltNum) {
            const CosineSim::ValueType value = CosineSim::ValueType(std::rand() % MaxValue - (MaxValue / 2));
            *pBuf++ = value;
            
            // If we've reached the index we've chosen as the test vector, save the test vector values
            if (vecNum == testVectorIndex)
                testVector.push_back(value);
        }
        cosineSim.finishCurrentOldVector();
    }
    cosineSim.finishLoadOldVectors();
    
    // Run the match in the FPGA
    
    std::cout << "Running match for test vector #" << testVectorIndex << "..." << std::endl;
    std::vector<CosineSim::Result> results = cosineSim.matchNewVector(10, testVector.data());
    
    // Display the results
    
    std::cout << "Results:" << std::endl;
    std::cout << "Similarity   Vector #" << std::endl;
    std::cout << "----------   --------" << std::endl;
    for (CosineSim::Result &result : results)
        std::cout << result.similarity_ << "       " << result.index_;
    
    return 0;
}
