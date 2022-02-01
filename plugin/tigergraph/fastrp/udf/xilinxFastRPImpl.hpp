/*
 * Copyright 2019-2021 Xilinx, Inc.
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

#ifndef XILINX_FASTRP_IMPL_HPP
#define XILINX_FASTRP_IMPL_HPP

// Use inline definitions for dynamic loading functions
#define XILINX_FASTRP_INLINE_IMPL
#include "xilinxfastrp.hpp"

// Enable this to turn on debug output
#define XILINX_FASTRP_DEBUG_ON

// Enable this to dump context vertices and edges
//#define XILINX_FASTRP_DUMP_context

// Enable this to dump an .mtx file of the context
//#define XILINX_FASTRP_DUMP_MTX

#include <vector>
#include <map>
#include <fstream>

namespace xilFastRP {

using Mutex = std::mutex;

//#define XILINX_FASTRP_DEBUG_MUTEX


} /* namespace xilFastRP */

#include "fastrp_loader.cpp"

#endif /* XILINX_FASTRP_IMPL_HPP */