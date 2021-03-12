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


namespace xilinx_apps {
namespace cosinesim {



using RowIndex = std::int64_t;
using ColIndex = std::int32_t;

struct Result {
    RowIndex index_ = -1L;
    double similarity_ = 0.0;

    Result(RowIndex index, double similarity) {
        index_ = index;
        similarity_ = similarity;
    }
};
struct Options {
	ColIndex vecLength;
	//std::int64_t numVertices;
	std::int64_t devicesNeeded;
};

enum ErrorCode{
	NoError =0,
	ErrorGraphPartition,
	ErrorUnsupportedValueType,
	ErrorConfigFileNotExist,
	ErrorXclbinNotExist,
	ErrorXclbin2NotExist,
	ErrorFailFPGASetup
};

template <typename Value>
class CosineSim;

class ImplBase {
public:
	virtual ~ImplBase(){};
	virtual void startLoadPopulation(std::int64_t numVertices) = 0;
	virtual void *getPopulationVectorBuffer(RowIndex &rowIndex) = 0;
	virtual void finishCurrentPopulationVector() = 0;
	virtual void finishLoadPopulationVectors() =0;
	virtual std::vector<Result> matchTargetVector(unsigned numResults, void *elements) = 0;
};

extern "C" {
    ImplBase *createImpl(const Options& options, unsigned valueSize);
    void destroyImpl(ImplBase *pImpl);
}



template <typename Value>
class CosineSim {
public:
	using ValueType = Value;
	const Options& getOptions() {return options_;};

    //CosineSim( const Options &options) :CosineSimBase(options), pImpl_(createImpl(this, sizeof(Value))) {};
    
	CosineSim( const Options &options) :options_(options), pImpl_(createImpl(options, sizeof(Value))) {};


    //oid openFpga(...);
    void startLoadPopulation(std::int64_t numVertices){pImpl_->startLoadPopulation(numVertices);}  //

    Value *getPopulationVectorBuffer(RowIndex &rowIndex) {
        // figure out where in weightDense to start writing
        // memset vector padding (8 bytes for example) to 0
        // return pointer into weightDense
         return reinterpret_cast<Value *>(pImpl_->getPopulationVectorBuffer(rowIndex));
    }
    void finishCurrentPopulationVector(){pImpl_->finishCurrentPopulationVector();}
    void finishLoadPopulationVectors(){pImpl_->finishLoadPopulationVectors();}

    
    std::vector<Result> matchTargetVector(unsigned numResults, void *elements) {
    	return pImpl_->matchTargetVector(numResults, elements);
    }
    //void closeFpga();
    
private:
    //std::unique_ptr<ImplBase> pImpl_;
    ImplBase *pImpl_ = nullptr;
    Options options_;
    //ColIndex vecLength_ = 0;
    //RowIndex numRows_ = 0;

};

} // namespace cosinesim
} // namespace xilinx_apps


#endif /* XILINX_APPS_COSINESIM_HPP */
