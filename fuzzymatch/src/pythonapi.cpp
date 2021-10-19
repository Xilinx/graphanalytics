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

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include <xilinxFuzzyMatch.h>

using namespace xilinx_apps::fuzzymatch;

namespace py = pybind11;


PYBIND11_MODULE (fuzzymatch, pc) {
  pc.doc() = "Python bindings for the Xilinx FuzzyMatch library";

  py::class_<XString>(pc, "xString")
    .def(py::init<>())
    .def(py::init<const char *>())
    .def(py::init<const std::string &>());
    
  py::class_<Options>(pc, "options")
    .def(py::init())
    .def_readwrite("xclbinPath", &Options::xclbinPath)
    .def_readwrite("deviceNames", &Options::deviceNames);
    
    
  py::class_<FuzzyMatch>(pc, "FuzzyMatch")
    .def(py::init<const Options &>())
    .def("startFuzzyMatch", &FuzzyMatch::startFuzzyMatch)
    .def("fuzzyMatchLoadVec", &FuzzyMatch::fuzzyMatchLoadVec)
    .def("executefuzzyMatch", &FuzzyMatch::executefuzzyMatch);

}
