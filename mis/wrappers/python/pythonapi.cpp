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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include <xilinxmis.hpp>

using namespace xilinx_apps::mis;

namespace py = pybind11;

PYBIND11_MODULE (xilMisPython, pc) {
  pc.doc() = "Python bindings for the Xilinx Maximal Independent Set library";

  py::class_<XString>(pc, "xString")
    .def(py::init<>())
    .def(py::init<const char *>())
    .def(py::init<const std::string &>());

  py::class_<GraphCSR>(pc, "GraphCSR")
    .def(py::init<std::vector<int> &, std::vector<int> &>());
    
  py::class_<Options>(pc, "options")
    .def(py::init())
    .def_readwrite("xclbinPath", &Options::xclbinPath)
    .def_readwrite("deviceNames", &Options::deviceNames);
    
  py::class_<MIS>(pc, "MIS")
    .def(py::init<const Options &>())
    .def("startMis", &MIS::startMis)
    .def("setGraph", &MIS::setGraph)
    .def("executeMIS", &MIS::executeMIS)
    .def("count", &MIS::count);

}
