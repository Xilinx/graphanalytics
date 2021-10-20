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
#include <xilinxlouvain.h>

using namespace xilinx_apps::louvainmod;

namespace py = pybind11;

namespace xilinx_apps {
namespace louvainmod {
}
}

PYBIND11_MODULE (xilLouvainmod, pc) {
  pc.doc() = "Python bindings for the Xilinx Louvainmod library";

  py::class_<XString>(pc, "xString")
    .def(py::init<>())
    .def(py::init<const char *>())
    .def(py::init<const std::string &>());
    
  py::class_<Options>(pc, "options")
    .def(py::init())
    .def_readwrite("modeAlveo", &Options::modeAlveo)
    .def_readwrite("xclbinPath", &Options::xclbinPath)
    .def_readwrite("kernelMode", &Options::kernelMode)
    .def_readwrite("nameProj", &Options::nameProj)
    .def_readwrite("alveoProject", &Options::alveoProject)
    .def_readwrite("numDevices", &Options::numDevices)
    .def_readwrite("deviceNames", &Options::deviceNames)
    .def_readwrite("nodeId", &Options::nodeId)
    .def_readwrite("hostName", &Options::hostName)
    .def_readwrite("hostIpAddress", &Options::hostIpAddress)
    .def_readwrite("clusterIpAddresses", &Options::clusterIpAddresses);
    
  py::class_<LouvainMod::PartitionOptions>(pc, "partitionOptions")
    .def(py::init())
    .def_readwrite("numPars", &LouvainMod::PartitionOptions::numPars)
    .def_readwrite("par_prune", &LouvainMod::PartitionOptions::par_prune)
    .def_readwrite("totalNumVertices", &LouvainMod::PartitionOptions::totalNumVertices);
    
  py::class_<LouvainMod::ComputeOptions>(pc, "computeOptions")
    .def(py::init())
    .def_readwrite("outputFile", &LouvainMod::ComputeOptions::outputFile)
    .def_readwrite("max_iter", &LouvainMod::ComputeOptions::max_iter)
    .def_readwrite("max_level", &LouvainMod::ComputeOptions::max_level)
    .def_readwrite("tolerance", &LouvainMod::ComputeOptions::tolerance)
    .def_readwrite("intermediateResult", &LouvainMod::ComputeOptions::intermediateResult)
    .def_readwrite("final_Q", &LouvainMod::ComputeOptions::final_Q)
    .def_readwrite("all_Q", &LouvainMod::ComputeOptions::all_Q);
    
  py::class_<LouvainMod>(pc, "louvainMod")
    .def(py::init<const Options &>())
    .def("partitionDataFile", &LouvainMod::partitionDataFile)
    .def("loadAlveoAndComputeLouvain", &LouvainMod::loadAlveoAndComputeLouvain);

}
