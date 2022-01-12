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

#include "nHop_kernel.hpp"
///#include "xf_graph_L2.hpp"
#include "nHop.hpp"

#ifndef __SYNTHESIS__
#include <iostream>
#endif

void nHopKernelWrapper(unsigned numHop,
                       unsigned intermediate,
                       unsigned numPairs,
                       unsigned batchSize,
                       unsigned hashSize,
                       unsigned aggrThreshold,
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

                       ap_uint<512>* bufferPing,
                       ap_uint<512>* bufferPong,

                       unsigned* numOut,
                       ap_uint<512>* bufferLocal,
                       ap_uint<512>* bufferSwitch) {
#pragma HLS INLINE off
#pragma HLS DATAFLOW

    hls::stream<ap_uint<512> > pairStream;
#pragma HLS stream variable = pairStream depth = 512
#pragma HLS resource variable = pairStream core = FIFO_BRAM
    hls::stream<bool> pairStreamEnd;
#pragma HLS stream variable = pairStreamEnd depth = 512
#pragma HLS resource variable = pairStreamEnd core = FIFO_LUTRAM
    hls::stream<ap_uint<512> > switchStream;
#pragma HLS stream variable = switchStream depth = 512
#pragma HLS resource variable = switchStream core = FIFO_BRAM
    hls::stream<bool> switchStreamEnd;
#pragma HLS stream variable = switchStreamEnd depth = 512
#pragma HLS resource variable = switchStreamEnd core = FIFO_LUTRAM

    xf::graph::internal::Hop::loadBuffer<true>(0, (numPairs + 3) / 4, pair, pairStream, pairStreamEnd);

    xf::graph::nHop(numHop, intermediate, numPairs, batchSize, hashSize, aggrThreshold, byPass, duplicate, pairStream, pairStreamEnd,
                    offsetTable, indexTable, cardTable, offset0, index0, offset1, index1, offset2, index2, offset3,
                    index3,  bufferPing, bufferPong,
                    numOut, bufferLocal, switchStream, switchStreamEnd);

    xf::graph::internal::Hop::writeOut(switchStream, switchStreamEnd, bufferSwitch);
}

extern "C" void nHop_kernel(unsigned numHop,
                            unsigned intermediate,
                            unsigned numPairs,
                            unsigned batchSize,
                            unsigned hashSize,
                            unsigned aggrThreshold,
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

                            ap_uint<512>* bufferPing,
                            ap_uint<512>* bufferPong,

                            unsigned* numOut,
                            ap_uint<512>* bufferLocal,
                            ap_uint<512>* bufferSwitch) {
    const int ext_mem_size = 16 << 20;

// clang-format off
#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 2 num_read_outstanding = \
    64 max_write_burst_length = 1 max_read_burst_length = 32 bundle = gmem0 port = pair depth = ext_mem_size 
#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 64 num_read_outstanding = \
    2 max_write_burst_length = 32 max_read_burst_length = 8 bundle = gmem11 port = offsetTable depth = 4096
#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 64 num_read_outstanding = \
    2 max_write_burst_length = 32 max_read_burst_length = 8 bundle = gmem11 port = indexTable depth = 4096
#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 64 num_read_outstanding = \
    2 max_write_burst_length = 32 max_read_burst_length = 8 bundle = gmem11 port = cardTable depth = 4096

#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 2 num_read_outstanding = \
    64 max_write_burst_length = 1 max_read_burst_length = 32 bundle = gmem1 port = offset0 depth = ext_mem_size
#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 2 num_read_outstanding = \
    64 max_write_burst_length = 1 max_read_burst_length = 32 bundle = gmem2 port = index0 depth = ext_mem_size
#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 2 num_read_outstanding = \
    64 max_write_burst_length = 1 max_read_burst_length = 32 bundle = gmem3 port = offset1 depth = ext_mem_size
#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 2 num_read_outstanding = \
    64 max_write_burst_length = 1 max_read_burst_length = 32 bundle = gmem4 port = index1 depth = ext_mem_size
#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 2 num_read_outstanding = \
    64 max_write_burst_length = 1 max_read_burst_length = 32 bundle = gmem5 port = offset2 depth = ext_mem_size
#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 2 num_read_outstanding = \
    64 max_write_burst_length = 1 max_read_burst_length = 32 bundle = gmem6 port = index2 depth = ext_mem_size
#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 2 num_read_outstanding = \
    64 max_write_burst_length = 1 max_read_burst_length = 32 bundle = gmem7 port = offset3 depth = ext_mem_size
#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 2 num_read_outstanding = \
    64 max_write_burst_length = 1 max_read_burst_length = 32 bundle = gmem8 port = index3 depth = ext_mem_size

#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 64 num_read_outstanding = \
    64 max_write_burst_length = 32 max_read_burst_length = 32 bundle = gmem9 port = bufferPing depth = ext_mem_size
#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 64 num_read_outstanding = \
    64 max_write_burst_length = 32 max_read_burst_length = 32 bundle = gmem10 port = bufferPong depth = ext_mem_size

#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 64 num_read_outstanding = \
    2 max_write_burst_length = 32 max_read_burst_length = 8 bundle = gmem11 port = numOut depth = ext_mem_size
#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 64 num_read_outstanding = \
    2 max_write_burst_length = 32 max_read_burst_length = 8 bundle = gmem11 port = bufferLocal depth = ext_mem_size
#pragma HLS INTERFACE m_axi offset = slave latency = 32 num_write_outstanding = 64 num_read_outstanding = \
    64 max_write_burst_length = 32 max_read_burst_length = 32 bundle = gmem12 port = bufferSwitch depth = ext_mem_size


#pragma HLS INTERFACE s_axilite port = numHop bundle = control
#pragma HLS INTERFACE s_axilite port = intermediate bundle = control
#pragma HLS INTERFACE s_axilite port = numPairs bundle = control
#pragma HLS INTERFACE s_axilite port = batchSize bundle = control
#pragma HLS INTERFACE s_axilite port = hashSize bundle = control
#pragma HLS INTERFACE s_axilite port = aggrThreshold bundle = control
#pragma HLS INTERFACE s_axilite port = byPass bundle = control
#pragma HLS INTERFACE s_axilite port = duplicate bundle = control
#pragma HLS INTERFACE s_axilite port = pair bundle = control

#pragma HLS INTERFACE s_axilite port = offsetTable bundle = control
#pragma HLS INTERFACE s_axilite port = indexTable bundle = control
#pragma HLS INTERFACE s_axilite port = cardTable bundle = control

#pragma HLS INTERFACE s_axilite port = offset0 bundle = control
#pragma HLS INTERFACE s_axilite port = index0 bundle = control
#pragma HLS INTERFACE s_axilite port = offset1 bundle = control
#pragma HLS INTERFACE s_axilite port = index1 bundle = control
#pragma HLS INTERFACE s_axilite port = offset2 bundle = control
#pragma HLS INTERFACE s_axilite port = index2 bundle = control
#pragma HLS INTERFACE s_axilite port = offset3 bundle = control
#pragma HLS INTERFACE s_axilite port = index3 bundle = control

#pragma HLS INTERFACE s_axilite port = bufferPing bundle = control
#pragma HLS INTERFACE s_axilite port = bufferPong bundle = control

#pragma HLS INTERFACE s_axilite port = numOut bundle = control
#pragma HLS INTERFACE s_axilite port = bufferLocal bundle = control
#pragma HLS INTERFACE s_axilite port = bufferSwitch bundle = control

#pragma HLS INTERFACE s_axilite port = return bundle = control
// clang-format on

#ifndef __SYNTHESIS__
    std::cout << "kernel call success" << std::endl;
#endif

    nHopKernelWrapper(numHop, intermediate, numPairs, batchSize, hashSize, aggrThreshold, byPass, duplicate, pair, offsetTable,
                      indexTable, cardTable, offset0, index0, offset1, index1, offset2, index2, offset3, index3,
                       bufferPing, bufferPong,
                      numOut, bufferLocal, bufferSwitch);

#ifndef __SYNTHESIS__
    std::cout << "kernel call finish" << std::endl;
#endif
}
