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

#ifndef XILINX_APPS_COSINESIM_HPP
#define XILINX_APPS_COSINESIM_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <exception>

namespace xilinx_apps
{
namespace cosinesim
{
struct Options;
class ImplBase;
}

}

// Define this macro to make functions in cosinesim_loader.cpp inline instead of extern.  You would use this macro
// when including cosinesim_loader.cpp in a header file, as opposed to linking with libXilinxCosineSim_loader.a.
#ifdef XILINX_COSINESIM_INLINE_IMPL
#define XILINX_COSINESIM_IMPL_DECL inline
#else
#define XILINX_COSINESIM_IMPL_DECL extern
#endif

extern "C" {
XILINX_COSINESIM_IMPL_DECL
xilinx_apps::cosinesim::ImplBase *xilinx_cosinesim_createImpl(const xilinx_apps::cosinesim::Options& options, unsigned valueSize);

XILINX_COSINESIM_IMPL_DECL
void xilinx_cosinesim_destroyImpl(xilinx_apps::cosinesim::ImplBase *pImpl);
}

namespace xilinx_apps {
namespace cosinesim {

using RowIndex = std::int64_t;
using ColIndex = std::int32_t;


class Exception : public std::exception {
    std::string message;
public:
    Exception(const std::string &msg) : message(msg) {}
    virtual const char* what() const noexcept override { return message.c_str(); }
};


struct Result {
    RowIndex index = -1L;
    double similarity = 0.0;

    Result(RowIndex index, double similarity) {
        this->index = index;
        this->similarity = similarity;
    }
};

struct Options {
    // vector length, default is 200.
    ColIndex vecLength;
    // number of devices, default is 1.
    std::int32_t numDevices;
    // FPGA binary file Path. default is the package installation path
    std::string xclbinPath;
    
    // Work around g++ version compatibility problems (use this instead of xclbinPath)
    const char *xclbinPathCStr = nullptr;
};


template <typename Value>
class CosineSim;

class ImplBase {
public:
    virtual ~ImplBase(){};
    virtual void startLoadPopulation(std::int64_t numVertices) = 0;
    virtual void *getPopulationVectorBuffer(RowIndex &rowIndex) = 0;
    virtual void finishCurrentPopulationVector(void * pbuf) = 0;
    virtual void finishLoadPopulationVectors() =0;
    virtual std::vector<Result> matchTargetVector(unsigned numResults, void *elements) = 0;
    virtual void cleanGraph() =0;
};

class CosineSimBase {
public:
    CosineSimBase(const Options &options, unsigned valueSize) : pImpl_(::xilinx_cosinesim_createImpl(options, valueSize)) {
        // Copy C string to std::string, as C string may go away
        if (options_.xclbinPathCStr != nullptr) {
            options_.xclbinPath = options_.xclbinPathCStr;
            options_.xclbinPathCStr = nullptr;
        }
    }

    ~CosineSimBase() {
        pImpl_->cleanGraph();
        ::xilinx_cosinesim_destroyImpl(pImpl_);
    }

    // load population vectors initialization API. should be called once before user loads population vectors
    void startLoadPopulation(std::int64_t numVectors){pImpl_->startLoadPopulation(numVectors);}  //

    // return pointer of weightDense buffer. user can use the pointer to write into population vector
    void *getPopulationVectorBuffer(RowIndex &rowIndex) {
        return pImpl_->getPopulationVectorBuffer(rowIndex);
    }

    // should be called when each population vector loading finishes
    void finishCurrentPopulationVector(void *pbuf){pImpl_->finishCurrentPopulationVector(pbuf);}

    // should be called when the whole population vectors loading finishes
    void finishLoadPopulationVectors(){pImpl_->finishLoadPopulationVectors();}

    // Match API
    std::vector<Result> matchTargetVector(unsigned numResults, void *elements) {
        return pImpl_->matchTargetVector(numResults, elements);
    }

private:
    Options options_;
    ImplBase *pImpl_ = nullptr;
};

template <typename Value>
class CosineSim : public CosineSimBase {
public:
    using ValueType = Value;
    const Options& getOptions() {return options_;};

   // CosineSim( const Options &options) :options_(options), pImpl_(::xilinx_cosinesim_createImpl(options, sizeof(Value)))
    CosineSim(const Options &options) : CosineSimBase(options, sizeof(Value)) {};


    // return pointer of weightDense buffer. user can use the pointer to write into population vector
    Value *getPopulationVectorBuffer(RowIndex &rowIndex) {
        return reinterpret_cast<Value *>(CosineSimBase::getPopulationVectorBuffer(rowIndex));
    }


    // should be called when each population vector loading finishes
    void finishCurrentPopulationVector(Value *pbuf){CosineSimBase::finishCurrentPopulationVector(pbuf);}

private:
    Options options_;
    ImplBase *pImpl_ = nullptr;

};

} // namespace cosinesim
} // namespace xilinx_apps


#endif /* XILINX_APPS_COSINESIM_HPP */
