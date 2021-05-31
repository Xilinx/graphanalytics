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
 * WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include <cosinesim.hpp>

using namespace xilinx_apps::cosinesim;

namespace py = pybind11;

namespace xilinx_apps {
namespace cosinesim {

// Binding fixed to following datatype
using DataType = std::int32_t;

// C++ random number generator, because native python is too slow
using VecDataType = std::vector<DataType>;
VecDataType buildRandData(int max, int min, int vecSize) {
    VecDataType vec(vecSize);
    for(auto elem = 0; elem < vecSize; elem++) {
      vec[elem] = DataType(std::rand() * 1.0*(max-min)/RAND_MAX + min);
    }
    return vec;
}


// Pointer container class for passing to python
template<typename PtrType>
class PtrWrapper {
public:
    PtrWrapper(void *p, int ptrLength) : ptr_(p), ptrLength_(ptrLength), currIdx_(0) {}
    PtrWrapper() : ptr_(nullptr), ptrLength_(0), currIdx_(0) {}

    void *get_ptr() { return ptr_; } // This is not right!

    void append(py::list valueList) {
        unsigned vecSize = valueList.size();
        if(currIdx_ + vecSize > ptrLength_) {
            std::cout << "CPP_POINTER_ERROR: Tried to store " << currIdx_ + vecSize << " elements in a C++ array of size " << ptrLength_ << std::endl;
        }
        else {
            for(unsigned i = 0; i < vecSize; i++) {
                *((PtrType *)ptr_ + currIdx_) = valueList[i].cast<DataType>();
                currIdx_++;
            }
        }
    }

private:
    void * ptr_;
    unsigned ptrLength_;
    unsigned currIdx_;
};

// Wrapper child class to extend base class with PtrWrapper
class PyCSWrapper : public CosineSimBase {
public:
    PyCSWrapper(const Options &options, unsigned valueSize) : CosineSimBase(options, valueSize), opt_(options) {}

    PtrWrapper<DataType> getPopulationVectorBuffer(RowIndex &rowIndex) {
        void *populationVec = CosineSimBase::getPopulationVectorBuffer(rowIndex);
        return PtrWrapper<DataType>(populationVec, opt_.vecLength);
    }

    void finishCurrentPopulationVector(PtrWrapper<DataType> populationVecWrap) {
        CosineSimBase::finishCurrentPopulationVector(populationVecWrap.get_ptr());
    }

    std::vector<Result> matchTargetVector(unsigned numResults, std::vector<DataType> elementsVec) {
        return CosineSimBase::matchTargetVector(numResults, elementsVec.data());
    }

private:
    Options opt_;
};
}
}

PYBIND11_MODULE (xilCosineSim, pc) {
  pc.doc() = "Python bindings for the Xilinx Cosine Similarity library";

  pc.def("buildRandData", &buildRandData);

  py::class_<Options>(pc, "options")
    .def(py::init())
    .def_readwrite("vecLength", &Options::vecLength)
    .def_readwrite("numDevices", &Options::numDevices)
    .def_readwrite("xclbinPath", &Options::xclbinPath);

  py::class_<Result>(pc, "result")
    .def(py::init<RowIndex, double>())
    .def_readonly("index", &Result::index)
    .def_readonly("similarity", &Result::similarity);

  py::class_<PtrWrapper<DataType>>(pc, "cpppointer")
    .def(py::init<void*, int>())
    .def(py::init<>())
    .def("append", &PtrWrapper<DataType>::append);

  py::class_<PyCSWrapper>(pc, "cosinesim")
    .def(py::init<const Options &, unsigned>())
    .def("startLoadPopulation", &PyCSWrapper::startLoadPopulation,
        "load population vectors initialization API.,should be called once before user loads population vectors")
    .def("getPopulationVectorBuffer", &PyCSWrapper::getPopulationVectorBuffer,
        "return pointer of weightDense buffer. user can use the pointer to write into population vector")
    .def("finishCurrentPopulationVector", &PyCSWrapper::finishCurrentPopulationVector,
        "should be called when each population vector loading finishes")
    .def("finishLoadPopulation", &PyCSWrapper::finishLoadPopulation,
        "should be called when the whole population vectors loading finishes")
    .def("matchTargetVector", &PyCSWrapper::matchTargetVector, "Match API");
}
