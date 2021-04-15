#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include <cosinesim.hpp>

using namespace xilinx_apps::cosinesim;

namespace py = pybind11;

namespace xilinx_apps {
namespace cosinesim {

// Binding fixed to following datatype
using Datatype = std::int32_t;

// Pointer container class for passing to python
template<typename PtrType>
class PtrWrapper {
public:
    PtrWrapper(void *p, int ptrLength) : ptr_(p), ptrLength_(ptrLength), currIdx_(0) {}
    PtrWrapper() : ptr_(nullptr), ptrLength_(0), currIdx_(0) {}

    void *get_ptr() { return ptr_; } // This is not right!

    void fill(PtrType value) {
        if(currIdx_ >= ptrLength_) {
            std::cout << "C_POINTER_ERROR: Tried writing to position " << currIdx_ << " for " << ptrLength_ << " length array" << std::endl;
        }
        else {
            *((PtrType *)ptr_ + currIdx_) = value;
            currIdx_++;
        }
    }

private:
    void * ptr_;
    int ptrLength_;
    int currIdx_;
};

// Wrapper child class to extend base class with PtrWrapper
class PyCSWrapper : public CosineSimBase {
public:
    PyCSWrapper(const Options &options, unsigned valueSize) : CosineSimBase(options, valueSize), opt_(options) {}

    PtrWrapper<Datatype> getPopulationVectorBuffer(RowIndex &rowIndex) {
        void *populationVec = CosineSimBase::getPopulationVectorBuffer(rowIndex);
        return PtrWrapper<Datatype>(populationVec, opt_.vecLength);
    }

    void finishCurrentPopulationVector(PtrWrapper<Datatype> populationVecWrap) {
        CosineSimBase::finishCurrentPopulationVector(populationVecWrap.get_ptr());
    }

    std::vector<Result> matchTargetVector(unsigned numResults, std::vector<Datatype> elementsVec) {
        return CosineSimBase::matchTargetVector(numResults, elementsVec.data());
    }

private:
    Options opt_;
};
}
}

PYBIND11_MODULE (xilCosineSim, pc) {
  pc.doc() = "Python bindings for the Xilinx Cosine Similarity library";

  py::class_<Options>(pc, "options")
    .def(py::init())
    .def_readwrite("vecLength", &Options::vecLength)
    .def_readwrite("numDevices", &Options::numDevices)
    .def_readwrite("xclbinPath", &Options::xclbinPath);

  py::class_<Result>(pc, "result")
    .def(py::init<RowIndex, double>())
    .def_readonly("index", &Result::index)
    .def_readonly("similarity", &Result::similarity);

  py::class_<PtrWrapper<Datatype>>(pc, "cpppointer")
    .def(py::init<void*, int>())
    .def(py::init<>())
    .def("fill", &PtrWrapper<Datatype>::fill);

  py::class_<PyCSWrapper>(pc, "cosinesim")
    .def(py::init<const Options &, unsigned>())
    .def("startLoadPopulation", &PyCSWrapper::startLoadPopulation,
        "load population vectors initialization API.,should be called once before user loads population vectors")
    .def("getPopulationVectorBuffer", &PyCSWrapper::getPopulationVectorBuffer,
        "return pointer of weightDense buffer. user can use the pointer to write into population vector")
    .def("finishCurrentPopulationVector", &PyCSWrapper::finishCurrentPopulationVector,
        "should be called when each population vector loading finishes")
    .def("finishLoadPopulationVectors", &PyCSWrapper::finishLoadPopulationVectors,
        "should be called when the whole population vectors loading finishes")
    .def("matchTargetVector", &PyCSWrapper::matchTargetVector, "Match API");
}
