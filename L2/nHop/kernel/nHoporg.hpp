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

#ifndef __XF_GRAPH_nHoporg_HPP_
#define __XF_GRAPH_nHoporg_HPP_

#include <ap_int.h>
#include <hls_stream.h>
#include "xf_database/hash_lookup3.hpp"

namespace xf {
namespace graph {
namespace internal {
namespace nHop {

inline void load_pair(ap_uint<32> start, ap_uint<32> numPairs, ap_uint<64>* pair, hls::stream<ap_uint<128> >& pairStream) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "========================loadPair===================" << std::endl;
    std::cout << " start=" << start << " numPairs=" << numPairs << std::endl;
#endif

    for (unsigned i = 0; i < numPairs.range(31, 9); i++) {
        ap_uint<32> addr = i * 512 + start;
        for (unsigned j = 0; j < 512; j++) {
#pragma HLS PIPELINE II = 1

            ap_uint<64> pair_in = pair[addr + j];
            ap_uint<32> src, des;
            src = pair_in(31, 0);
            des = pair_in(63, 32);

            ap_uint<128> tmp;
            tmp(63, 0) = pair_in;
            tmp(95, 64) = src;
            tmp(127, 96) = 0;
            pairStream.write(tmp);
        }
    }

    for (unsigned i = 0; i < numPairs.range(8, 0); i++) {
#pragma HLS PIPELINE II = 1
        ap_uint<64> pair_in = pair[numPairs.range(31, 9) * 512 + start + i];
        ap_uint<32> src, des;
        src = pair_in(31, 0);
        des = pair_in(63, 32);

#ifndef __SYNTHESIS__
        if ((numPairs.range(31, 9) * 512 + start + i) < 100)
            std::cout << "addr=" << numPairs.range(31, 9) * 512 + start + i << " src=" << src << " des=" << des
                      << std::endl;
#endif

        ap_uint<128> tmp;
        tmp(63, 0) = pair_in;
        tmp(95, 64) = src;
        tmp(127, 96) = 0;
        pairStream.write(tmp);
    }
}

inline void load_residual(ap_uint<32> numResidual, ap_uint<128>* buffer, hls::stream<ap_uint<128> >& pairStream) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "========================loadPair===================" << std::endl;
    std::cout << " numResidual=" << numResidual << std::endl;
#endif

    for (unsigned i = 0; i < numResidual.range(31, 9); i++) {
        for (unsigned j = 0; j < 512; j++) {
#pragma HLS PIPELINE II = 1
            ap_uint<128> tmp = buffer[i * 512 + j];
            pairStream.write(tmp);
        }
    }

    for (unsigned i = 0; i < numResidual.range(8, 0); i++) {
#pragma HLS PIPELINE II = 1
        ap_uint<128> tmp = buffer[numResidual.range(31, 9) * 512 + i];
        pairStream.write(tmp);

#ifndef __SYNTHESIS__
        ap_uint<32> src, des, idx, hop;
        src = tmp(31, 0);
        des = tmp(63, 32);
        idx = tmp(95, 64);
        hop = tmp(127, 96);
        if ((numResidual.range(31, 9) * 512 + i) < 100)
            std::cout << "addr=" << numResidual.range(31, 9) * 512 + i << " src=" << src << " des=" << des
                      << " idx=" << idx << " hop=" << hop << std::endl;
#endif
    }
}

inline void load(ap_uint<32> loopCount,
          ap_uint<32> start,
          ap_uint<32> numPairs,
          ap_uint<64>* pair,
          ap_uint<128>* buffer,
          hls::stream<ap_uint<128> >& pairStream) {
#pragma HLS INLINE off

    if (loopCount == 0) {
        load_pair(start, numPairs, pair, pairStream);
    } else {
        load_residual(numPairs, buffer, pairStream);
    }
}

template <int dispatchNum>
inline void dispatchOffset(ap_uint<32> numPairs,
                    hls::stream<ap_uint<128> >& pairStream,
                    ap_uint<32> offsetTable[dispatchNum + 1],
                    hls::stream<ap_uint<128> > dispatchStream[dispatchNum],
                    hls::stream<bool> dispatchStreamEnd[dispatchNum]) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "========================dispatchOffset===================" << std::endl;
#endif

    for (unsigned i = 0; i < numPairs; i++) {
#pragma HLS PIPELINE II = 1
        ap_uint<32> src, des, idx, hop;
        ap_uint<128> tmp0 = pairStream.read();
        src = tmp0(31, 0);
        des = tmp0(63, 32);
        idx = tmp0(95, 64);
        hop = tmp0(127, 96);

        for (int j = dispatchNum - 1; j >= 0; j--) {
            if ((idx >= offsetTable[j]) && (idx < offsetTable[j + 1])) {
                ap_uint<128> tmp1;
                tmp1(31, 0) = src;
                tmp1(63, 32) = des;
                tmp1(95, 64) = idx - offsetTable[j];
                tmp1(127, 96) = hop;

                dispatchStream[j].write(tmp1);
                dispatchStreamEnd[j].write(false);

#ifndef __SYNTHESIS__
                if (i < 100)
                    std::cout << "dispatchID=" << j << " src=" << src << " des=" << des
                              << " idx=" << idx - offsetTable[j] << " offsetTable=" << offsetTable[j] << std::endl;
#endif
            }
        }
    }
    for (int i = 0; i < dispatchNum; i++) {
#pragma HLS UNROLL
        dispatchStreamEnd[i].write(true);
    }
}

inline void loadHopOffest(ap_uint<32> numHop,
                   hls::stream<ap_uint<128> >& pairStream,
                   hls::stream<bool>& pairStreamEnd,
                   ap_uint<32> indexShift,
                   unsigned* offset,
                   hls::stream<ap_uint<160> >& offsetStream,
                   hls::stream<bool>& offsetStreamEnd) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "========================loadHopOffset===================" << std::endl;
    std::cout << "indexShift=" << indexShift << std::endl;
#endif

    bool end = pairStreamEnd.read();
    while (!end) {
#pragma HLS PIPELINE II = 2
        ap_uint<32> src, des, idx, hop, offset_start, offset_end;
        ap_uint<128> tmp0 = pairStream.read();
        end = pairStreamEnd.read();

        src = tmp0(31, 0);
        des = tmp0(63, 32);
        idx = tmp0(95, 64);
        hop = tmp0(127, 96);

        ap_uint<160> tmp1;
        ap_uint<32> addr;
        offset_start = offset[idx];
        offset_end = offset[idx + 1];

#ifndef __SYNTHESIS__
        std::cout << "src=" << src << " des=" << des << " idx=" << idx << " hop=" << hop
                  << " offset_start=" << offset[idx] << " offset_end=" << offset[idx + 1] << std::endl;
#endif
        tmp1(31, 0) = src;
        tmp1(63, 32) = des;
        tmp1(95, 64) = offset_start - indexShift;
        tmp1(127, 96) = offset_end - offset_start;
        tmp1(159, 128) = hop + 1;

        offsetStream.write(tmp1);
        offsetStreamEnd.write(false);
    }
    offsetStreamEnd.write(true);
}

inline void loadHopIndex(ap_uint<32> numHop,
                  hls::stream<ap_uint<160> >& offsetStream,
                  hls::stream<bool>& offsetStreamEnd,
                  unsigned* index,
                  hls::stream<ap_uint<128> >& indexStream,
                  hls::stream<bool>& indexStreamEnd) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "========================loadHopIndex===================" << std::endl;
#endif

    bool end = offsetStreamEnd.read();
    ap_int<32> nm = 0;
    while (!end || nm > 0) {
#pragma HLS PIPELINE II = 1
        ap_uint<32> src, des, idx, hop, offset;
        ap_uint<160> tmp0;
        ap_uint<128> tmp1;
        if (nm == 0) {
            tmp0 = offsetStream.read();
            end = offsetStreamEnd.read();
            src = tmp0(31, 0);
            des = tmp0(63, 32);
            offset = tmp0(95, 64);
            nm = tmp0(127, 96);
            hop = tmp0(159, 128);

#ifndef __SYNTHESIS__
            std::cout << "src=" << src << " des=" << des << " offset=" << offset << " nm=" << nm << " hop=" << hop
                      << std::endl;
#endif
        } else {
            idx = index[offset];
            offset++;
            nm--;

            tmp1(31, 0) = src;
            tmp1(63, 32) = des;
            tmp1(95, 64) = idx;
            tmp1(127, 96) = hop;

            if ((idx == des) || (hop < numHop)) {
                indexStream.write(tmp1);
                indexStreamEnd.write(false);

#ifndef __SYNTHESIS__
                std::cout << "src=" << src << " des=" << des << " idx=" << idx << " hop=" << hop << std::endl;
#endif
            }
        }
    }
    indexStream.write(0);
    indexStreamEnd.write(true);
}

template <int IN_NM>
ap_uint<3> mux(ap_uint<IN_NM> rd) {
#pragma HLS inline
    ap_uint<3> o = 0;
    if (IN_NM == 8) {
        o[0] = rd[1] | rd[3] | rd[5] | rd[7];
        o[1] = rd[2] | rd[3] | rd[6] | rd[7];
        o[2] = rd[4] | rd[5] | rd[6] | rd[7];
    } else if (IN_NM == 4) {
        o[0] = rd[1] | rd[3];
        o[1] = rd[2] | rd[3];
    } else if (IN_NM == 2) {
        o[0] = rd[1];
    } else {
        o = 0;
    }
    return o;
}

template <int CH_NM>
ap_uint<CH_NM> mul_ch_read(ap_uint<CH_NM> empty) {
    ap_uint<CH_NM> rd = 0;
#pragma HLS inline
    for (int i = 0; i < CH_NM; i++) {
#pragma HLS unroll
        ap_uint<CH_NM> t_e = 0;
        if (i > 0) t_e = empty(i - 1, 0);
        rd[i] = t_e > 0 ? (bool)0 : (bool)empty[i];
    }
    return rd;
}

template <int KEYW>
inline void merge8to1(hls::stream<ap_uint<KEYW> >& i0_key_strm,
               hls::stream<bool>& i0_e_strm,
               hls::stream<ap_uint<KEYW> >& i1_key_strm,
               hls::stream<bool>& i1_e_strm,
               hls::stream<ap_uint<KEYW> >& i2_key_strm,
               hls::stream<bool>& i2_e_strm,
               hls::stream<ap_uint<KEYW> >& i3_key_strm,
               hls::stream<bool>& i3_e_strm,
               hls::stream<ap_uint<KEYW> >& i4_key_strm,
               hls::stream<bool>& i4_e_strm,
               hls::stream<ap_uint<KEYW> >& i5_key_strm,
               hls::stream<bool>& i5_e_strm,
               hls::stream<ap_uint<KEYW> >& i6_key_strm,
               hls::stream<bool>& i6_e_strm,
               hls::stream<ap_uint<KEYW> >& i7_key_strm,
               hls::stream<bool>& i7_e_strm,

               hls::stream<ap_uint<KEYW> >& o_key_strm,
               hls::stream<bool>& o_e_strm) {
    ap_uint<KEYW> key_arry[8];
#pragma HLS array_partition variable = key_arry dim = 1
    ap_uint<8> empty_e = 0;
    ap_uint<8> rd_e = 0;
    ap_uint<8> last = 0;
#ifndef __SYNTHESIS__
    unsigned int cnt = 0;
#endif
LOOP_MERGE8_1:
    do {
#pragma HLS loop_tripcount min = 1 max = 5000
#pragma HLS PIPELINE II = 1
        empty_e[0] = !i0_e_strm.empty() && !last[0];
        empty_e[1] = !i1_e_strm.empty() && !last[1];
        empty_e[2] = !i2_e_strm.empty() && !last[2];
        empty_e[3] = !i3_e_strm.empty() && !last[3];
        empty_e[4] = !i4_e_strm.empty() && !last[4];
        empty_e[5] = !i5_e_strm.empty() && !last[5];
        empty_e[6] = !i6_e_strm.empty() && !last[6];
        empty_e[7] = !i7_e_strm.empty() && !last[7];
        rd_e = mul_ch_read(empty_e);
        if (rd_e[0]) {
            key_arry[0] = i0_key_strm.read();
            last[0] = i0_e_strm.read();
        }
        if (rd_e[1]) {
            key_arry[1] = i1_key_strm.read();
            last[1] = i1_e_strm.read();
        }
        if (rd_e[2]) {
            key_arry[2] = i2_key_strm.read();
            last[2] = i2_e_strm.read();
        }
        if (rd_e[3]) {
            key_arry[3] = i3_key_strm.read();
            last[3] = i3_e_strm.read();
        }
        if (rd_e[4]) {
            key_arry[4] = i4_key_strm.read();
            last[4] = i4_e_strm.read();
        }
        if (rd_e[5]) {
            key_arry[5] = i5_key_strm.read();
            last[5] = i5_e_strm.read();
        }
        if (rd_e[6]) {
            key_arry[6] = i6_key_strm.read();
            last[6] = i6_e_strm.read();
        }
        if (rd_e[7]) {
            key_arry[7] = i7_key_strm.read();
            last[7] = i7_e_strm.read();
        }
        ap_uint<3> id = mux<8>(rd_e);
        ap_uint<KEYW> key = key_arry[id];
        bool valid_n = last[id];
        if (!valid_n && rd_e != 0) {
#ifndef __SYNTHESIS__
            cnt++;
#endif
            o_key_strm.write(key);
            o_e_strm.write(false);
        }
    } while (last != 255);
    o_e_strm.write(true);
#ifndef __SYNTHESIS__
    std::cout << "merge number=" << cnt << std::endl;
#endif
}

inline void nHopMultiProcessingUnit(ap_uint<32> numHop,
                             ap_uint<32> numPairs,
                             hls::stream<ap_uint<128> >& pairStream,
                             ap_uint<32> offestTable[9],
                             ap_uint<32> indexTable[9],

                             unsigned* offset0,
                             unsigned* offset1,
                             unsigned* offset2,
                             unsigned* offset3,
                             unsigned* offset4,
                             unsigned* offset5,
                             unsigned* offset6,
                             unsigned* offset7,

                             unsigned* index0,
                             unsigned* index1,
                             unsigned* index2,
                             unsigned* index3,
                             unsigned* index4,
                             unsigned* index5,
                             unsigned* index6,
                             unsigned* index7,

                             hls::stream<ap_uint<128> >& hopStream,
                             hls::stream<bool>& hopStreamEnd) {
#pragma HLS INLINE off
#pragma HLS DATAFLOW

    hls::stream<ap_uint<128> > dispatchStream[8];
#pragma HLS stream variable = dispatchStream depth = 512
#pragma HLS resource variable = dispatchStream core = FIFO_BRAM
    hls::stream<bool> dispatchStreamEnd[8];
#pragma HLS stream variable = dispatchStreamEnd depth = 512
#pragma HLS resource variable = dispatchStreamEnd core = FIFO_LUTRAM

    hls::stream<ap_uint<160> > offsetStream[8];
#pragma HLS stream variable = offsetStream depth = 512
#pragma HLS resource variable = offsetStream core = FIFO_BRAM
    hls::stream<bool> offsetStreamEnd[8];
#pragma HLS stream variable = offsetStreamEnd depth = 512
#pragma HLS resource variable = offsetStreamEnd core = FIFO_LUTRAM

    hls::stream<ap_uint<128> > indexStream[8];
#pragma HLS stream variable = indexStream depth = 512
#pragma HLS resource variable = indexStream core = FIFO_BRAM
    hls::stream<bool> indexStreamEnd[8];
#pragma HLS stream variable = indexStreamEnd depth = 512
#pragma HLS resource variable = indexStreamEnd core = FIFO_LUTRAM

    dispatchOffset<8>(numPairs, pairStream, offestTable, dispatchStream, dispatchStreamEnd);

    loadHopOffest(numHop, dispatchStream[0], dispatchStreamEnd[0], indexTable[0], offset0, offsetStream[0],
                  offsetStreamEnd[0]);

    loadHopOffest(numHop, dispatchStream[1], dispatchStreamEnd[1], indexTable[1], offset1, offsetStream[1],
                  offsetStreamEnd[1]);

    loadHopOffest(numHop, dispatchStream[2], dispatchStreamEnd[2], indexTable[2], offset2, offsetStream[2],
                  offsetStreamEnd[2]);

    loadHopOffest(numHop, dispatchStream[3], dispatchStreamEnd[3], indexTable[3], offset3, offsetStream[3],
                  offsetStreamEnd[3]);

    loadHopOffest(numHop, dispatchStream[4], dispatchStreamEnd[4], indexTable[4], offset4, offsetStream[4],
                  offsetStreamEnd[4]);

    loadHopOffest(numHop, dispatchStream[5], dispatchStreamEnd[5], indexTable[5], offset5, offsetStream[5],
                  offsetStreamEnd[5]);

    loadHopOffest(numHop, dispatchStream[6], dispatchStreamEnd[6], indexTable[6], offset6, offsetStream[6],
                  offsetStreamEnd[6]);

    loadHopOffest(numHop, dispatchStream[7], dispatchStreamEnd[7], indexTable[7], offset7, offsetStream[7],
                  offsetStreamEnd[7]);

    loadHopIndex(numHop, offsetStream[0], offsetStreamEnd[0], index0, indexStream[0], indexStreamEnd[0]);

    loadHopIndex(numHop, offsetStream[1], offsetStreamEnd[1], index1, indexStream[1], indexStreamEnd[1]);

    loadHopIndex(numHop, offsetStream[2], offsetStreamEnd[2], index2, indexStream[2], indexStreamEnd[2]);

    loadHopIndex(numHop, offsetStream[3], offsetStreamEnd[3], index3, indexStream[3], indexStreamEnd[3]);

    loadHopIndex(numHop, offsetStream[4], offsetStreamEnd[4], index4, indexStream[4], indexStreamEnd[4]);

    loadHopIndex(numHop, offsetStream[5], offsetStreamEnd[5], index5, indexStream[5], indexStreamEnd[5]);

    loadHopIndex(numHop, offsetStream[6], offsetStreamEnd[6], index6, indexStream[6], indexStreamEnd[6]);

    loadHopIndex(numHop, offsetStream[7], offsetStreamEnd[7], index7, indexStream[7], indexStreamEnd[7]);

    merge8to1<128>(indexStream[0], indexStreamEnd[0], indexStream[1], indexStreamEnd[1], indexStream[2],
                   indexStreamEnd[2], indexStream[3], indexStreamEnd[3], indexStream[4], indexStreamEnd[4],
                   indexStream[5], indexStreamEnd[5], indexStream[6], indexStreamEnd[6], indexStream[7],
                   indexStreamEnd[7], hopStream, hopStreamEnd);
}

inline void writeOut(hls::stream<ap_uint<128> >& streamIn,
              hls::stream<bool>& streamInEnd,
              ap_uint<32>& count,
              ap_uint<128>* buffer) {
#pragma HLS INLINE off
    count = 0;
    bool end = streamInEnd.read();
    while (!end) {
#pragma HLS PIPELINE II = 1
        buffer[count] = streamIn.read();
        end = streamInEnd.read();
        count++;
    }
}

inline void hashProcess(ap_uint<32> numHop,
                 hls::stream<ap_uint<128> >& hopStream,
                 hls::stream<bool>& hopStreamEnd,
                 hls::stream<ap_uint<96> >& aggrStream,
                 hls::stream<bool>& aggrStreamEnd,
                 hls::stream<ap_uint<128> >& resStream,
                 hls::stream<bool>& resStreamEnd) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "=======================hashProcess===================" << std::endl;
#endif
    bool end = hopStreamEnd.read();
    while (!end) {
#pragma HLS PIPELINE II = 1

        ap_uint<32> src, des, idx, hop;
        ap_uint<128> tmp0 = hopStream.read();
        end = hopStreamEnd.read();

        src = tmp0(31, 0);
        des = tmp0(63, 32);
        idx = tmp0(95, 64);
        hop = tmp0(127, 96);

        ap_uint<96> tmp1;
        ap_uint<64> key = (des, src);
        ap_uint<64> hash;
        database::details::hashlookup3_seed_core<64>(key, 0xbeef, hash);
        tmp1(63, 0) = tmp0(63, 0);
        tmp1(95, 64) = hash(31, 0);
        if (hop < numHop) {
#ifndef __SYNTHESIS__
            std::cout << "residualStream: src=" << src << " des=" << des << " idx=" << idx << " hop=" << hop
                      << std::endl;
#endif
            resStream.write(tmp0);
            resStreamEnd.write(false);
        }
        if (des == idx) {
#ifndef __SYNTHESIS__
            std::cout << "aggrStream: src=" << src << " des=" << des << std::endl;
#endif
            aggrStream.write(tmp1);
            aggrStreamEnd.write(false);
        }
    }
    resStreamEnd.write(true);
    aggrStreamEnd.write(true);
}

inline void aggrCounter(hls::stream<ap_uint<96> >& aggrStream,
                 hls::stream<bool>& aggrStreamEnd,
#ifndef __SYNTHESIS__
                 ap_uint<96>* aggrURAM[64]
#else
                 ap_uint<96> aggrURAM[64][4096]
#endif
                 ) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "=======================aggCounter===================" << std::endl;
#endif

    ap_uint<96> elem = 0;
    ap_uint<96> elem_temp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    ap_uint<18> hash_temp[8] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
                                0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
#pragma HLS array_partition variable = elem_temp complete
#pragma HLS array_partition variable = hash_temp complete

    bool end = aggrStreamEnd.read();
    bool write_success = true;

    ap_uint<32> src, des;
    ap_uint<18> hash;
    ap_uint<6> chip_idx;
    ap_uint<12> arry_idx;
    while (!end) {
#pragma HLS PIPELINE II = 1
#pragma HLS DEPENDENCE variable = aggrURAM pointer inter false
#pragma HLS LATENCY min = 8

        if (write_success) {
            ap_uint<160> tmp0 = aggrStream.read();
            end = aggrStreamEnd.read();

            src = tmp0(31, 0);
            des = tmp0(63, 32);
            hash = tmp0(81, 64);
            chip_idx = hash(17, 12);
            arry_idx = hash(11, 0);

            if (hash == hash_temp[0]) {
                elem = elem_temp[0];
            } else if (hash == hash_temp[1]) {
                elem = elem_temp[1];
            } else if (hash == hash_temp[2]) {
                elem = elem_temp[2];
            } else if (hash == hash_temp[3]) {
                elem = elem_temp[3];
            } else if (hash == hash_temp[4]) {
                elem = elem_temp[4];
            } else if (hash == hash_temp[5]) {
                elem = elem_temp[5];
            } else if (hash == hash_temp[6]) {
                elem = elem_temp[6];
            } else if (hash == hash_temp[7]) {
                elem = elem_temp[7];
            } else {
                elem = aggrURAM[chip_idx][arry_idx];
            }
        } else {
            elem = aggrURAM[chip_idx][arry_idx];
        }

        ap_uint<64> key = elem(63, 0);
        ap_uint<32> pld = elem(95, 64);
        ap_uint<96> new_elem;

#ifndef __SYNTHESIS__
        std::cout << "write_success=" << write_success << " src=" << src << " des=" << des << " chip_idx=" << chip_idx
                  << " arry_idx=" << arry_idx << " (des, src)=" << (des, src) << " key=" << key << " pld=" << pld
                  << std::endl;
#endif

        if (((key(63, 32) == des) && (key(31, 0) == src)) || (key == 0)) {
            new_elem(95, 64) = pld + 1;
            new_elem(63, 32) = des;
            new_elem(31, 0) = src;
            aggrURAM[chip_idx][arry_idx] = new_elem;

            for (int i = 7; i > 0; i--) {
#pragma HLS unroll
                hash_temp[i] = hash_temp[i - 1];
                elem_temp[i] = elem_temp[i - 1];
            }
            hash_temp[0] = (chip_idx, arry_idx);
            elem_temp[0] = new_elem;

            write_success = true;
        } else {
            if (arry_idx < 4095)
                arry_idx++;
            else {
                chip_idx++;
                arry_idx = 0;
            }

            write_success = false;
        }
    }
}

inline void initializeURAM(
#ifndef __SYNTHESIS__
              ap_uint<96>* aggrURAM[64]
#else
              ap_uint<96> aggrURAM[64][4096]
#endif
) {
#pragma HLS INLINE off

    for (int i = 0; i < 64; i++) {
#pragma HLS UNROLL
        for (int j = 0; j < 4096; j++) {
#pragma HLS PIPELINE II = 1
            aggrURAM[i][j] = 0;
        }
    }
}

inline void outputURAM(
#ifndef __SYNTHESIS__
    ap_uint<96>* aggrURAM[64],
#else
    ap_uint<96> aggrURAM[64][4096],
#endif
    ap_uint<32>& numResult,
    ap_uint<128>* result) {
#pragma HLS INLINE off

    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 4096; j++) {
#pragma HLS PIPELINE II = 1
            ap_uint<128> tmp = aggrURAM[i][j];
            if (tmp != 0) {
                numResult++;
                result[numResult] = tmp;

#ifndef __SYNTHESIS__
                ap_uint<32> src, des, count;
                src = tmp(31, 0);
                des = tmp(63, 32);
                count = tmp(95, 64);

                std::cout << "i=" << i << " j=" << j << " src=" << src << " des=" << des << " count=" << count
                          << std::endl;
#endif
            }
        }
    }
    result[0] = numResult;
}

inline void hashAggrCounter(ap_uint<32> numHop,
                     hls::stream<ap_uint<128> >& hopStream,
                     hls::stream<bool>& hopStreamEnd,
#ifndef __SYNTHESIS__
                     ap_uint<96>* aggrURAM[64],
#else
                     ap_uint<96> aggrURAM[64][4096],
#endif
                     ap_uint<32>& numResidual,
                     ap_uint<128>* buffer) {
#pragma HLS DATAFLOW
#pragma HLS INLINE off

    hls::stream<ap_uint<96> > aggrStream("aggrStream");
#pragma HLS stream variable = aggrStream depth = 512
#pragma HLS resource variable = aggrStream core = FIFO_BRAM
    hls::stream<bool> aggrStreamEnd("aggrStreamEnd");
#pragma HLS stream variable = aggrStreamEnd depth = 512
#pragma HLS resource variable = aggrStreamEnd core = FIFO_LUTRAM

    hls::stream<ap_uint<128> > resStream("resStream");
#pragma HLS stream variable = resStream depth = 512
#pragma HLS resource variable = resStream core = FIFO_BRAM
    hls::stream<bool> resStreamEnd("resStreamEnd");
#pragma HLS stream variable = resStreamEnd depth = 512
#pragma HLS resource variable = resStreamEnd core = FIFO_LUTRAM

    hashProcess(numHop, hopStream, hopStreamEnd, aggrStream, aggrStreamEnd, resStream, resStreamEnd);

    aggrCounter(aggrStream, aggrStreamEnd, aggrURAM);

    writeOut(resStream, resStreamEnd, numResidual, buffer);
}

inline void nHopCore(ap_uint<32> loopCount,
              ap_uint<32> numHop,
              ap_uint<32> pairStart,
              ap_uint<32> numPairs,
              ap_uint<64>* pair,

              ap_uint<32> offsetTable[9],
              ap_uint<32> indexTable[9],

              unsigned* offset0,
              unsigned* index0,
              unsigned* offset1,
              unsigned* index1,
              unsigned* offset2,
              unsigned* index2,
              unsigned* offset3,
              unsigned* index3,
              unsigned* offset4,
              unsigned* index4,
              unsigned* offset5,
              unsigned* index5,
              unsigned* offset6,
              unsigned* index6,
              unsigned* offset7,
              unsigned* index7,
#ifndef __SYNTHESIS__
              ap_uint<96>* aggrURAM[64],
#else
              ap_uint<96> aggrURAM[64][4096],
#endif
              ap_uint<32>& numResidual,
              ap_uint<128>* bufferPing,
              ap_uint<128>* bufferPong) {
#pragma HLS DATAFLOW

    hls::stream<ap_uint<128> > pairStream("pairStream");
#pragma HLS stream variable = pairStream depth = 512
#pragma HLS resource variable = pairStream core = FIFO_BRAM

    load(loopCount, pairStart, numPairs, pair, bufferPing, pairStream);

    hls::stream<ap_uint<128> > hopStream("hopStream");
#pragma HLS stream variable = hopStream depth = 512
#pragma HLS resource variable = hopStream core = FIFO_BRAM
    hls::stream<bool> hopStreamEnd("hopStreamEnd");
#pragma HLS stream variable = hopStreamEnd depth = 512
#pragma HLS resource variable = hopStreamEnd core = FIFO_LUTRAM

    nHopMultiProcessingUnit(numHop, numPairs, pairStream, offsetTable, indexTable, offset0, offset1, offset2, offset3,
                            offset4, offset5, offset6, offset7, index0, index1, index2, index3, index4, index5, index6,
                            index7, hopStream, hopStreamEnd);

    hashAggrCounter(numHop, hopStream, hopStreamEnd, aggrURAM, numResidual, bufferPong);
}

} // namespace nHop
} // namespace internal

/**
 * @brief nHop this API can find the how many 2-hop pathes between two vertices. The input graph is the matrix in
 * CSR format. And a list of src and destination pairs whose 2-hop pathes will be counted.
 *
 * @param numPairs  How many pairs of source and destination vertices to be counted.
 * @param pair  The source and destination of pairs are stored in this pointer.
 * @param offsetOneHop The CSR offset is stored in this pointer.
 * @param indexOneHop The CSR index is stored in this pointer.
 * @param offsetnHop The CSR offset is stored in this pointer. The graph should be replicated and stored here. This
 * pointer is for an independent AXI port to increase performance.
 * @param indexTwoop The CSR index is stored in this pointer. The graph should be replicated and stored here. This
 * pointer is for an independent AXI port to increase performance.
 * @param cnt_res The result of the nHop API. The order of the result matches the order of the input source and
 * destination pairs.
 *
 */

inline void nHop_org(ap_uint<32> numHop,
          ap_uint<32> numPairs,
          ap_uint<32> batchSize,
          ap_uint<64>* pair,

          ap_uint<32>* offestTable,
          ap_uint<32>* indexTable,
          unsigned* offset0,
          unsigned* index0,
          unsigned* offset1,
          unsigned* index1,
          unsigned* offset2,
          unsigned* index2,
          unsigned* offset3,
          unsigned* index3,
          unsigned* offset4,
          unsigned* index4,
          unsigned* offset5,
          unsigned* index5,
          unsigned* offset6,
          unsigned* index6,
          unsigned* offset7,
          unsigned* index7,

          ap_uint<128>* bufferPing,
          ap_uint<128>* bufferPong,

          ap_uint<128>* result) {
#pragma HLS INLINE

#ifndef __SYNTHESIS__
        std::cout << "===" << std::endl;
#endif

#ifndef __SYNTHESIS__
    ap_uint<96>* aggrURAM[64];
    for (int i = 0; i < 64; i++) aggrURAM[i] = (ap_uint<96>*)malloc(4096 * sizeof(ap_uint<96>));
#else
    ap_uint<96> aggrURAM[64][4096];
#endif

#pragma HLS bind_storage variable = aggrURAM type = ram_2p impl = uram
    ap_uint<32> table0[9];
#pragma HLS ARRAY_PARTITION variable = table0 complete
    ap_uint<32> table1[9];
#pragma HLS ARRAY_PARTITION variable = table1 complete

    ap_uint<32> numResult = 0;

#ifndef __SYNTHESIS__
        std::cout << "========================LoadLUT===================" << std::endl;
#endif

LoadTable:
    for (int i = 0; i < 9; i++) {
#pragma HLS PIPELINE II = 1
        table0[i] = offestTable[i];
        table1[i] = indexTable[i];
    }

BatchProcess:
    for (ap_uint<32> pairStart = 0; pairStart < numPairs; pairStart += batchSize) {
        ap_uint<32> loop_count = 0;
        ap_uint<32> numResidual = 0;
        bool ping_pong_flag = false;

#ifndef __SYNTHESIS__
        std::cout << "========================InitializeURAM===================" << std::endl;
#endif

        xf::graph::internal::nHop::initializeURAM(aggrURAM);

    HopLoop:
        while ((loop_count == 0) || (numResidual > 0)) {
#ifndef __SYNTHESIS__
            std::cout << "========================nHopCoreStart===================" << std::endl;
#endif

            ap_uint<32> numPair = loop_count == 0 ? batchSize : numResidual;

            if (ping_pong_flag) {
                xf::graph::internal::nHop::nHopCore(loop_count, numHop, pairStart, numPair, pair, table0, table1,
                                                    offset0, index0, offset1, index1, offset2, index2, offset3, index3,
                                                    offset4, index4, offset5, index5, offset6, index6, offset7, index7,
                                                    aggrURAM, numResidual, bufferPing, bufferPong);
            } else {
                xf::graph::internal::nHop::nHopCore(loop_count, numHop, pairStart, numPair, pair, table0, table1,
                                                    offset0, index0, offset1, index1, offset2, index2, offset3, index3,
                                                    offset4, index4, offset5, index5, offset6, index6, offset7, index7,
                                                    aggrURAM, numResidual, bufferPong, bufferPing);
            }
            ping_pong_flag = !ping_pong_flag;
            loop_count++;

#ifndef __SYNTHESIS__
            std::cout << "numResidual=" << numResidual << " loopC_cunt=" << loop_count << std::endl;
            std::cout << "========================nHopCoreEnd===================" << std::endl;
#endif
        }

#ifndef __SYNTHESIS__
        std::cout << "========================OutputResults===================" << std::endl;
#endif
        xf::graph::internal::nHop::outputURAM(aggrURAM, numResult, result);
#ifndef __SYNTHESIS__
        std::cout << "numResult=" << numResult << std::endl;
#endif
    }

#ifndef __SYNTHESIS__
    for (int i = 0; i < 64; i++) free(aggrURAM[i]);
#endif
}

} // namespace graph
} // namespace xf

#endif
