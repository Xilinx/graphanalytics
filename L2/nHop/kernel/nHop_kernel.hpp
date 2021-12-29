/*
 * Copyright 2019 Xilinx, Inc.
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

#ifndef _XF_GRAPH_nHop_KERNEL_HPP_
#define _XF_GRAPH_nHop_KERNEL_HPP_

#include <ap_int.h>

/*
8pu kernel
commit 0f8d0faa315de033adfa3d2e8ebfe94c7f88ca28
Author: siyangw <siyangw@xilinx.com>
Date:   Thu Dec 23 02:06:43 2021 -0800

    add template for PU
*/
    extern "C" void nHop_kernel(unsigned numHop,
                            unsigned intermediate,
                            unsigned numPairs,
                            unsigned batchSize,
                            unsigned hashSize,
                            unsigned byPass,
                            unsigned duplicate,
                            ap_uint<512>* pair,

                            unsigned* offsetTable,
                            unsigned* indexTable,
                            ap_uint<64>* cardTable,
                            unsigned* offset0,
                            ap_uint<128>* index0,
                            unsigned* offset1,
                            ap_uint<128>* index1,
                            unsigned* offset2,
                            ap_uint<128>* index2,
                            unsigned* offset3,
                            ap_uint<128>* index3,
                            unsigned* offset4,
                            ap_uint<128>* index4,
                            unsigned* offset5,
                            ap_uint<128>* index5,
                            unsigned* offset6,
                            ap_uint<128>* index6,
                            unsigned* offset7,
                            ap_uint<128>* index7,

                            ap_uint<512>* bufferPing,
                            ap_uint<512>* bufferPong,

                            unsigned* numOut,
                            ap_uint<512>* bufferLocal,
                            ap_uint<512>* bufferSwitch);

/*
4pu kernel
*/
// extern "C" void nHop_kernel(unsigned numHop,
//                             unsigned intermediate,
//                             unsigned numPairs,
//                             unsigned batchSize,
//                             unsigned hashSize,
//                             unsigned byPass,
//                             unsigned duplicate,
//                             ap_uint<512>* pair,

//                             unsigned* offsetTable,
//                             unsigned* indexTable,
//                             ap_uint<64>* cardTable,
//                             unsigned* offset0,
//                             ap_uint<128>* index0,
//                             unsigned* offset1,
//                             ap_uint<128>* index1,
//                             unsigned* offset2,
//                             ap_uint<128>* index2,
//                             unsigned* offset3,
//                             ap_uint<128>* index3,

//                             ap_uint<512>* bufferPing,
//                             ap_uint<512>* bufferPong,

//                             unsigned* numOut,
//                             ap_uint<512>* bufferLocal,
//                             ap_uint<512>* bufferSwitch);


#endif
