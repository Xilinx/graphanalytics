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

#ifndef __XF_GRAPH_nHop_HPP_
#define __XF_GRAPH_nHop_HPP_

#define DEBUG true
#ifdef DEBUG

#define DEBUG_LOAD true
#define DEBUG_HOP true
#define DEBUG_AGGR true
#define DEBUG_OUTPUT true

#endif

#include <ap_int.h>
#include <hls_stream.h>
#include "xf_database/hash_lookup3.hpp"

namespace xf {
namespace graph {
namespace internal {
namespace Hop {

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

template <int KEYW, bool PADD>
void merge1to1(hls::stream<ap_uint<KEYW> >& i0_key_strm,
               hls::stream<bool>& i0_e_strm,

               ap_uint<32>& mergedCnt,
               hls::stream<ap_uint<KEYW> >& o_key_strm,
               hls::stream<bool>& o_e_strm) {
    ap_uint<KEYW> key_arry;
    bool rd_e = 0;
    bool last = 0;
    ap_uint<32> cnt = 0;
LOOP_MERGE1_1:
    do {
#pragma HLS loop_tripcount min = 1 max = 5000
#pragma HLS PIPELINE II = 1
        rd_e = !i0_e_strm.empty() && !last;
        if (rd_e) {
            key_arry = i0_key_strm.read();
            last = i0_e_strm.read();
        }
        ap_uint<KEYW> key = key_arry;
        bool valid_n = last;
        if (!valid_n && rd_e != 0) {
            cnt++;
            o_key_strm.write(key);
            o_e_strm.write(false);
        }
    } while (last != 1);
    o_e_strm.write(true);
    if (PADD) o_key_strm.write(0);
    mergedCnt = cnt;
#ifndef __SYNTHESIS__
    std::cout << "merge number=" << cnt << std::endl;
#endif
}

template <int KEYW, bool PADD>
void merge2to1(hls::stream<ap_uint<KEYW> >& i0_key_strm,
               hls::stream<bool>& i0_e_strm,
               hls::stream<ap_uint<KEYW> >& i1_key_strm,
               hls::stream<bool>& i1_e_strm,

               ap_uint<32>& mergedCnt,
               hls::stream<ap_uint<KEYW> >& o_key_strm,
               hls::stream<bool>& o_e_strm) {
    ap_uint<KEYW> key_arry[2];
#pragma HLS array_partition variable = key_arry dim = 1
    ap_uint<2> empty_e = 0;
    ap_uint<2> rd_e = 0;
    ap_uint<2> last = 0;
    ap_uint<32> cnt = 0;
LOOP_MERGE2_1:
    do {
#pragma HLS loop_tripcount min = 1 max = 5000
#pragma HLS PIPELINE II = 1
        empty_e[0] = !i0_e_strm.empty() && !last[0];
        empty_e[1] = !i1_e_strm.empty() && !last[1];
        rd_e = mul_ch_read(empty_e);
        if (rd_e[0]) {
            key_arry[0] = i0_key_strm.read();
            last[0] = i0_e_strm.read();
        }
        if (rd_e[1]) {
            key_arry[1] = i1_key_strm.read();
            last[1] = i1_e_strm.read();
        }
        ap_uint<1> id = mux<2>(rd_e);
        ap_uint<KEYW> key = key_arry[id];
        bool valid_n = last[id];
        if (!valid_n && rd_e != 0) {
            cnt++;
            o_key_strm.write(key);
            o_e_strm.write(false);
        }
    } while (last != 3);
    o_e_strm.write(true);
    if (PADD) o_key_strm.write(0);
    mergedCnt = cnt;
#ifndef __SYNTHESIS__
    std::cout << "merge number=" << cnt << std::endl;
#endif
}

template <int KEYW, bool PADD>
void merge4to1(hls::stream<ap_uint<KEYW> >& i0_key_strm,
               hls::stream<bool>& i0_e_strm,
               hls::stream<ap_uint<KEYW> >& i1_key_strm,
               hls::stream<bool>& i1_e_strm,
               hls::stream<ap_uint<KEYW> >& i2_key_strm,
               hls::stream<bool>& i2_e_strm,
               hls::stream<ap_uint<KEYW> >& i3_key_strm,
               hls::stream<bool>& i3_e_strm,

               ap_uint<32>& mergedCnt,
               hls::stream<ap_uint<KEYW> >& o_key_strm,
               hls::stream<bool>& o_e_strm) {
    ap_uint<KEYW> key_arry[4];
#pragma HLS array_partition variable = key_arry dim = 1
    ap_uint<4> empty_e = 0;
    ap_uint<4> rd_e = 0;
    ap_uint<4> last = 0;
    ap_uint<32> cnt = 0;
LOOP_MERGE4_1:
    do {
#pragma HLS loop_tripcount min = 1 max = 5000
#pragma HLS PIPELINE II = 1
        empty_e[0] = !i0_e_strm.empty() && !last[0];
        empty_e[1] = !i1_e_strm.empty() && !last[1];
        empty_e[2] = !i2_e_strm.empty() && !last[2];
        empty_e[3] = !i3_e_strm.empty() && !last[3];
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
        ap_uint<2> id = mux<4>(rd_e);
        ap_uint<KEYW> key = key_arry[id];
        bool valid_n = last[id];
        if (!valid_n && rd_e != 0) {
            cnt++;
            o_key_strm.write(key);
            o_e_strm.write(false);
        }
    } while (last != 15);
    o_e_strm.write(true);
    if (PADD) o_key_strm.write(0);
    mergedCnt = cnt;
#ifndef __SYNTHESIS__
    std::cout << "merge number=" << cnt << std::endl;
#endif
}

template <int KEYW, bool PADD>
void merge8to1(hls::stream<ap_uint<KEYW> >& i0_key_strm,
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

               ap_uint<32>& mergedCnt,
               hls::stream<ap_uint<KEYW> >& o_key_strm,
               hls::stream<bool>& o_e_strm) {
    ap_uint<KEYW> key_arry[8];
#pragma HLS array_partition variable = key_arry dim = 1
    ap_uint<8> empty_e = 0;
    ap_uint<8> rd_e = 0;
    ap_uint<8> last = 0;
    ap_uint<32> cnt = 0;
LOOP_MERGE8_1:
    do {
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
            cnt++;
            o_key_strm.write(key);
            o_e_strm.write(false);
        }
    } while (last != 255);
    o_e_strm.write(true);
    if (PADD) o_key_strm.write(0);
    mergedCnt = cnt;
#ifndef __SYNTHESIS__
    std::cout << "merge number=" << cnt << std::endl;
#endif
}

template <bool PADD>
void loadBuffer(ap_uint<32> start,
                ap_uint<32> numPairs,
                ap_uint<512>* pair,
                hls::stream<ap_uint<512> >& pairStream,
                hls::stream<bool>& pairStreamEnd) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "========================loadBuffer===================" << std::endl;
    std::cout << " start=" << start << " numPairs=" << numPairs << std::endl;
#endif

    for (unsigned i = 0; i < numPairs.range(31, 9); i++) {
        ap_uint<32> addr = i * 512 + start;
        for (unsigned j = 0; j < 512; j++) {
#pragma HLS PIPELINE II = 1

            ap_uint<512> tmpIn = pair[addr + j];
            pairStream.write(tmpIn);
            pairStreamEnd.write(false);

#ifndef __SYNTHESIS__
#ifdef DEBUG_LOAD
            for (unsigned k = 0; k < 4; k++) {
                ap_uint<128> pair_in = tmpIn(128 * k + 127, 128 * k);
                ap_uint<32> src, des, idx;
                ap_uint<16> hop, card;
                src = pair_in(31, 0);
                des = pair_in(63, 32);
                idx = pair_in(95, 64);
                hop = pair_in(111, 96);
                card = pair_in(127, 112);

                if ((src == 124311) && (des == 721))
                    std::cout << "loadBase: i=" << i << " j=" << j << " k=" << k << " src=" << src << " des=" << des
                              << " idx=" << idx << " hop=" << hop << " card=" << card << std::endl;
            }
#endif
#endif
        }
    }

    for (unsigned i = 0; i < numPairs.range(8, 0); i++) {
#pragma HLS PIPELINE II = 1
        ap_uint<512> tmpIn = pair[numPairs.range(31, 9) * 512 + start + i];
        pairStream.write(tmpIn);
        pairStreamEnd.write(false);

#ifndef __SYNTHESIS__
#ifdef DEBUG_LOAD
        for (unsigned k = 0; k < 4; k++) {
            ap_uint<128> pair_in = tmpIn(128 * k + 127, 128 * k);
            ap_uint<32> src, des, idx;
            ap_uint<16> hop, card;
            src = pair_in(31, 0);
            des = pair_in(63, 32);
            idx = pair_in(95, 64);
            hop = pair_in(111, 96);
            card = pair_in(127, 112);

            if ((src == 124311) && (des == 721))
                std::cout << "loadResidual: i=" << i << " k=" << k << " src=" << src << " des=" << des << " idx=" << idx
                          << " hop=" << hop << " card=" << card << std::endl;
        }
#endif
#endif
    }
    if (PADD) pairStream.write(0);
    pairStreamEnd.write(true);
}

template <bool PADD>
void loadStrm(ap_uint<32> numPairs,
              bool& batchEnd,
              hls::stream<ap_uint<512> >& pair,
              hls::stream<bool>& pairEnd,
              hls::stream<ap_uint<512> >& pairStream,
              hls::stream<bool>& pairStreamEnd) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "========================loadStrm===================" << std::endl;
    std::cout << "numPairs=" << numPairs << std::endl;
#endif

    bool end = batchEnd;
    ap_uint<32> cnt = 0;
    while (!end && (cnt < numPairs)) {
#pragma HLS PIPELINE II = 1
        ap_uint<512> tmpIn = pair.read();
        end = pairEnd.read();
        cnt++;

        pairStream.write(tmpIn);
        pairStreamEnd.write(false);

#ifndef __SYNTHESIS__
#ifdef DEBUG_LOAD
        for (unsigned k = 0; k < 4; k++) {
            ap_uint<128> pair_in = tmpIn(128 * k + 127, 128 * k);
            ap_uint<32> src, des, idx;
            ap_uint<16> hop, card;
            src = pair_in(31, 0);
            des = pair_in(63, 32);
            idx = pair_in(95, 64);
            hop = pair_in(111, 96);
            card = pair_in(127, 112);

            if ((src == 124311) && (des == 721))
                std::cout << "loadStrm: k=" << k << " src=" << src << " des=" << des << " idx=" << idx << " hop=" << hop
                          << " card=" << card << std::endl;
        }
#endif
#endif
    }
    batchEnd = end;
    if (PADD) pairStream.write(0);
    pairStreamEnd.write(true);
}

void splitPair(bool loadBatch,
               hls::stream<ap_uint<512> >& streamIn,
               hls::stream<bool>& streamInEnd,
               hls::stream<ap_uint<128> > pairStream[4],
               hls::stream<bool> pairStreamEnd[4]) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "========================splitPair===================" << std::endl;
#endif

    bool end = streamInEnd.read();
    while (!end) {
#pragma HLS PIPELINE II = 1

        ap_uint<512> tmpIn = streamIn.read();
        end = streamInEnd.read();
        for (unsigned k = 0; k < 4; k++) {
            ap_uint<128> pair_in = tmpIn(128 * k + 127, 128 * k);
            ap_uint<32> src, des, idx;
            ap_uint<16> hop, card;
            src = pair_in(31, 0);
            des = pair_in(63, 32);
            idx = pair_in(95, 64);
            hop = pair_in(111, 96);
            card = pair_in(127, 112);

            ap_uint<128> tmp;
            tmp(63, 0) = pair_in(63, 0);
            if (loadBatch && (hop == 0)) {
                tmp(95, 64) = src;
                tmp(127, 96) = 0;
            } else {
                tmp(95, 64) = idx;
                tmp(127, 96) = hop;
            }

            if (tmp != 0) {
#ifndef __SYNTHESIS__
#ifdef DEBUG_LOAD
                if ((src == 124311) && (des == 721))
                    std::cout << "src=" << src << " des=" << des << " idx=" << tmp(95, 64) << " hop=" << tmp(111, 96)
                              << " card=" << tmp(127, 112) << std::endl;
#endif
#endif

                pairStream[k].write(tmp);
                pairStreamEnd[k].write(false);
            }
        }
    }

    for (unsigned k = 0; k < 4; k++) {
#pragma HLS UNROLL
        pairStreamEnd[k].write(true);
    }
}

void loadPair(bool loadBatch,
              bool& batchEnd,
              ap_uint<32> numPairs,
              hls::stream<ap_uint<512> >& pair,
              hls::stream<bool>& pairEnd,
              ap_uint<512>* pingPongBuffer,
              hls::stream<ap_uint<512> >& pairStream,
              hls::stream<bool>& pairStreamEnd) {
#pragma HLS INLINE off

    if (loadBatch) {
        loadStrm<false>(numPairs, batchEnd, pair, pairEnd, pairStream, pairStreamEnd);
    } else {
        loadBuffer<false>(0, numPairs, pingPongBuffer, pairStream, pairStreamEnd);
    }
}

void load(bool loadBatch,
          bool& batchEnd,
          ap_uint<32> numPairs,
          hls::stream<ap_uint<512> >& pair,
          hls::stream<bool>& pairEnd,
          ap_uint<512>* buffer,
          hls::stream<ap_uint<128> > pairStream[4],
          hls::stream<bool> pairStreamEnd[4]) {
#pragma HLS INLINE off
#pragma HLS DATAFLOW

    hls::stream<ap_uint<512> > streamIn;
#pragma HLS stream variable = streamIn depth = 512
#pragma HLS resource variable = streamIn core = FIFO_BRAM
    hls::stream<bool> streamInEnd;
#pragma HLS stream variable = streamInEnd depth = 512
#pragma HLS resource variable = streamInEnd core = FIFO_SRL

    loadPair(loadBatch, batchEnd, numPairs, pair, pairEnd, buffer, streamIn, streamInEnd);

    splitPair(loadBatch, streamIn, streamInEnd, pairStream, pairStreamEnd);
}

template <int dispatchNum>
void dispatchSplitCSR(hls::stream<ap_uint<128> >& pairStream,
                      hls::stream<bool>& pairStreamEnd,
                      ap_uint<32> offsetTable[dispatchNum + 1],
                      hls::stream<ap_uint<128> > dispatchStream[dispatchNum],
                      hls::stream<bool> dispatchStreamEnd[dispatchNum]) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "========================dispatchOffset0===================" << std::endl;
#endif

    bool end = pairStreamEnd.read();
    while (!end) {
#pragma HLS PIPELINE II = 1
        ap_uint<32> src, des, idx, hop;
        ap_uint<128> tmp0 = pairStream.read();
        end = pairStreamEnd.read();
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
#ifdef DEBUG_LOAD
                if ((src == 124311) && (des == 721))
                    std::cout << "dispatchID=" << j << " src=" << src << " des=" << des
                              << " idx=" << idx - offsetTable[j] << " offsetTable=" << offsetTable[j] << " hop=" << hop
                              << std::endl;
#endif
#endif
            }
        }
    }
    for (int i = 0; i < dispatchNum; i++) {
#pragma HLS UNROLL
        dispatchStream[i].write(0);
        dispatchStreamEnd[i].write(true);
    }
}

template <int dispatchNum>
void dispatchDuplicateCSR(hls::stream<ap_uint<128> >& pairStream,
                          hls::stream<bool>& pairStreamEnd,
                          hls::stream<ap_uint<128> > dispatchStream[dispatchNum],
                          hls::stream<bool> dispatchStreamEnd[dispatchNum]) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "========================dispatchOffset1===================" << std::endl;
#endif

    ap_uint<2> j = 0;
    bool end = pairStreamEnd.read();
    while (!end) {
#pragma HLS PIPELINE II = 1
        ap_uint<128> tmp0 = pairStream.read();
        end = pairStreamEnd.read();

        dispatchStream[j].write(tmp0);
        dispatchStreamEnd[j].write(false);
        j++;
    }
    for (int i = 0; i < dispatchNum; i++) {
#pragma HLS UNROLL
        dispatchStream[i].write(0);
        dispatchStreamEnd[i].write(true);
    }
}

template <int PU>
void switchDispatch(ap_uint<32> duplicate,
                    ap_uint<32> offsetTable[PU + 1],
                    hls::stream<ap_uint<128> >& pairStream,
                    hls::stream<bool>& pairStreamEnd,
                    hls::stream<ap_uint<128> > dispatchStream[PU],
                    hls::stream<bool> dispatchStreamEnd[PU]) {
#pragma HLS INLINE off

    if (duplicate == 0) {
        dispatchSplitCSR<PU>(pairStream, pairStreamEnd, offsetTable, dispatchStream, dispatchStreamEnd);
    } else {
        dispatchDuplicateCSR<PU>(pairStream, pairStreamEnd, dispatchStream, dispatchStreamEnd);
    }
}

template <int PU>
void switchPairs(ap_uint<32> duplicate,
                 ap_uint<32> offsetTable[11],
                 hls::stream<ap_uint<128> > strmIn[4],
                 hls::stream<bool> strmInEnd[4],

                 hls::stream<ap_uint<128> > strmOut[PU],
                 hls::stream<bool> strmOutEnd[PU]) {
#pragma HLS INLINE off
#pragma HLS DATAFLOW

    hls::stream<ap_uint<128> > switchStrm[4][PU];
#pragma HLS stream variable = switchStrm depth = 8
#pragma HLS resource variable = switchStrm core = FIFO_SRL
    hls::stream<bool> switchStrmEnd[4][PU];
#pragma HLS stream variable = switchStrmEnd depth = 8
#pragma HLS resource variable = switchStrmEnd core = FIFO_SRL

    for (ap_uint<8> i = 0; i < 4; i++) {
#pragma HLS UNROLL

        switchDispatch<PU>(duplicate, offsetTable, strmIn[i], strmInEnd[i], switchStrm[i], switchStrmEnd[i]);
    }

    ap_uint<32> cnt[PU];
#pragma HLS ARRAY_PARTITION variable = cnt complete

    if (PU >= 1)
        merge4to1<128, false>(switchStrm[0][0], switchStrmEnd[0][0], switchStrm[1][0], switchStrmEnd[1][0],
                              switchStrm[2][0], switchStrmEnd[2][0], switchStrm[3][0], switchStrmEnd[3][0], cnt[0],
                              strmOut[0], strmOutEnd[0]);
    if (PU >= 2)
        merge4to1<128, false>(switchStrm[0][1], switchStrmEnd[0][1], switchStrm[1][1], switchStrmEnd[1][1],
                              switchStrm[2][1], switchStrmEnd[2][1], switchStrm[3][1], switchStrmEnd[3][1], cnt[1],
                              strmOut[1], strmOutEnd[1]);

    if (PU >= 4) {
        merge4to1<128, false>(switchStrm[0][2], switchStrmEnd[0][2], switchStrm[1][2], switchStrmEnd[1][2],
                              switchStrm[2][2], switchStrmEnd[2][2], switchStrm[3][2], switchStrmEnd[3][2], cnt[2],
                              strmOut[2], strmOutEnd[2]);

        merge4to1<128, false>(switchStrm[0][3], switchStrmEnd[0][3], switchStrm[1][3], switchStrmEnd[1][3],
                              switchStrm[2][3], switchStrmEnd[2][3], switchStrm[3][3], switchStrmEnd[3][3], cnt[3],
                              strmOut[3], strmOutEnd[3]);
    }
}

void loadHopOffset(ap_uint<32> numHop,
                   ap_uint<32> duplicate,
                   hls::stream<ap_uint<128> >& pairStream,
                   hls::stream<bool>& pairStreamEnd,
                   ap_uint<32> indexShift,
                   unsigned* offset,
                   hls::stream<ap_uint<128> >& offsetStream,
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

        ap_uint<128> tmp1;
        ap_uint<32> addr;
        offset_start = offset[idx];
        offset_end = offset[idx + 1];

        ap_uint<24> nm = offset_end - offset_start;
        tmp1(31, 0) = src;
        tmp1(63, 32) = des;
        if (duplicate == 0)
            tmp1(95, 64) = offset_start - indexShift;
        else
            tmp1(95, 64) = offset_start;
        tmp1(119, 96) = nm;
        tmp1(127, 120) = hop + 1;

#ifndef __SYNTHESIS__
#ifdef DEBUG_HOP
        if ((src == 124311) && (des == 721))
            std::cout << "src=" << src << " des=" << des << " idx=" << idx << " hop=" << hop
                      << " offset_start=" << offset[idx] << " offset_end=" << offset[idx + 1]
                      << " offset=" << tmp1(95, 64) << " nm=" << tmp1(119, 96) << std::endl;
#endif
#endif

        if (nm > 0) {
            offsetStream.write(tmp1);
            offsetStreamEnd.write(false);
        }
    }
    offsetStreamEnd.write(true);
}

void generateIndexAddr(hls::stream<ap_uint<128> >& offsetStream,
                       hls::stream<bool>& offsetStreamEnd,
                       hls::stream<ap_uint<128> >& addrStream,
                       hls::stream<bool>& addrStreamEnd) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "========================generateIndexAddr===================" << std::endl;
#endif

    ap_uint<32> src, des, hop, offset, addr;
    ap_uint<128> tmp0, tmp1;
    ap_uint<2> idx;

    bool end = offsetStreamEnd.read();
    ap_int<32> nm = 0;
    bool start = true;
    while (!end || nm > 0) {
#pragma HLS PIPELINE II = 1

        if (nm == 0) {
            tmp0 = offsetStream.read();
            end = offsetStreamEnd.read();

            src = tmp0(31, 0);
            des = tmp0(63, 32);
            offset = tmp0(95, 64);
            nm = tmp0(119, 96);
            hop = tmp0(127, 120);
            start = true;

            addr = offset(31, 2);
            idx = offset(1, 0);
        } else {
            ap_uint<4> enable;
            if (start) {
                for (int i = 0; i < 4; i++) {
                    if ((i >= idx) && (i < (nm + idx)))
                        enable[i] = 1;
                    else
                        enable[i] = 0;
                }
                if ((nm + idx) < 4) {
                    nm = 0;
                } else {
                    nm = nm + idx - 4;
                }
            } else {
                if (nm > 4) {
                    enable = 15;
                    nm = nm - 4;
                } else {
                    for (int i = 0; i < 4; i++) {
                        if (i < nm)
                            enable[i] = 1;
                        else
                            enable[i] = 0;
                    }
                    nm = 0;
                }
            }

            tmp1(31, 0) = src;
            tmp1(63, 32) = des;
            tmp1(95, 64) = addr;
            tmp1(99, 96) = enable;
            tmp1(127, 120) = hop;

            addr++;
            start = false;

#ifndef __SYNTHESIS__
#ifdef DEBUG_HOP
            if ((src == 124311) && (des == 721))
                std::cout << "IndexAddr src=" << src << " des=" << des << " offset=" << offset << " idx=" << idx
                          << " addr=" << addr << " enable=" << enable << " hop=" << hop << std::endl;
#endif
#endif

            addrStream.write(tmp1);
            addrStreamEnd.write(false);
        }
    }
    addrStreamEnd.write(true);
}

void loadIndex(ap_uint<32> numHop,
               ap_uint<32> intermediate,
               ap_uint<32> byPass,
               ap_uint<32> offsetStart,
               ap_uint<32> offsetEnd,

               hls::stream<ap_uint<128> >& offsetStream,
               hls::stream<bool>& offsetStreamEnd,
               ap_uint<128>* index,

               hls::stream<ap_uint<128> > localIndexStream[4],
               hls::stream<bool> localIndexStreamEnd[4],
               hls::stream<ap_uint<128> > netIndexStream[4],
               hls::stream<bool> netIndexStreamEnd[4],
               hls::stream<ap_uint<64> > aggrStream[4],
               hls::stream<bool> aggrStreamEnd[4],
               hls::stream<ap_uint<128> > outStream[4],
               hls::stream<bool> outStreamEnd[4]) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "========================loadHopIndex===================" << std::endl;
    std::cout << "offsetStart=" << offsetStart << " offsetEnd=" << offsetEnd << std::endl;
#endif

    bool end = offsetStreamEnd.read();
    while (!end) {
#pragma HLS PIPELINE II = 1
        ap_uint<32> src, des, hop, addr;
        ap_uint<128> idx_tmp;
        ap_uint<32> idx[4];
        ap_uint<4> enable;
        ap_uint<128> tmp0;
        ap_uint<128> tmp1;
        ap_uint<64> tmp2;
        ap_uint<128> tmp3;

        tmp0 = offsetStream.read();
        end = offsetStreamEnd.read();
        src = tmp0(31, 0);
        des = tmp0(63, 32);
        addr = tmp0(95, 64);
        enable = tmp0(99, 96);
        hop = tmp0(127, 120);

        idx_tmp = index[addr];
        for (int i = 0; i < 4; i++) {
            idx[i] = idx_tmp(32 * i + 31, 32 * i);

            tmp1(31, 0) = src;
            tmp1(63, 32) = des;
            tmp1(95, 64) = idx[i];
            tmp1(127, 96) = hop;

            tmp2(31, 0) = src;
            tmp2(63, 32) = des;

            if (enable[i]) {
                if ((hop < numHop) && (idx[i] >= offsetStart) && (idx[i] < offsetEnd)) {
                    // for local ping-pong hop
                    localIndexStream[i].write(tmp1);
                    localIndexStreamEnd[i].write(false);

#ifndef __SYNTHESIS__
#ifdef DEBUG_HOP
                    if ((src == 124311) && (des == 721))
                        std::cout << "localResidual src=" << src << " des=" << des << " idx=" << idx[i]
                                  << " hop=" << hop << std::endl;
#endif
#endif
                }

                if ((idx[i] < offsetStart) || (idx[i] >= offsetEnd)) {
                    if ((hop < numHop) || ((hop == numHop) && (intermediate != 0))) {
                        // for index need send to network
                        netIndexStream[i].write(tmp1);
                        netIndexStreamEnd[i].write(false);

#ifndef __SYNTHESIS__
#ifdef DEBUG_HOP
                        if ((src == 124311) && (des == 721))
                            std::cout << "netSwitch src=" << src << " des=" << des << " idx=" << idx[i]
                                      << " hop=" << hop << std::endl;
#endif
#endif
                    }
                }

                if ((idx[i] == des) && (byPass == 0)) {
                    // for internal aggr
                    aggrStream[i].write(tmp2);
                    aggrStreamEnd[i].write(false);

#ifndef __SYNTHESIS__
#ifdef DEBUG_HOP
                    if ((src == 124311) && (des == 721))
                        std::cout << "internalAggr src=" << src << " des=" << des << " idx=" << idx[i] << " hop=" << hop
                                  << std::endl;
#endif
#endif
                }

                if ((hop == numHop) && (intermediate != 0) && (idx < offsetEnd) && (idx >= offsetStart)) {
                    tmp3 = tmp1;
                } else if (((idx[i] == des) && (byPass != 0))) {
                    tmp3 = tmp2;
                } else {
                    tmp3 = 0;
                }

                if (((idx[i] == des) && (byPass != 0)) ||
                    ((hop == numHop) && (intermediate != 0) && (idx[i] < offsetEnd) && (idx[i] >= offsetStart))) {
                    // for output to local buffer
                    // (idx == des) && (byPass != 0) output aggr pair
                    // (hop == numHop) && (intermediate != 0) output intermediate hop
                    outStream[i].write(tmp3);
                    outStreamEnd[i].write(false);

#ifndef __SYNTHESIS__
#ifdef DEBUG_HOP
                    if ((src == 124311) && (des == 721))
                        std::cout << "localResult src=" << src << " des=" << des << " idx=" << tmp3(85, 64)
                                  << " hop=" << tmp3(127, 96) << std::endl;
#endif
#endif
                }
            }
        }
    }

    for (int i = 0; i < 4; i++) {
#pragma HLS UNROLL

        outStream[i].write(0);
        outStreamEnd[i].write(true);
        aggrStream[i].write(0);
        aggrStreamEnd[i].write(true);
        netIndexStream[i].write(0);
        netIndexStreamEnd[i].write(true);
        localIndexStream[i].write(0);
        localIndexStreamEnd[i].write(true);
    }
}

template <int WIDTH, bool PADD, bool END>
void transfer2to1(hls::stream<ap_uint<WIDTH> >& inStream0,
                  hls::stream<bool>& inStreamEnd0,
                  hls::stream<ap_uint<WIDTH> >& inStream1,
                  hls::stream<bool>& inStreamEnd1,
                  hls::stream<ap_uint<2 * WIDTH> >& outStream,
                  hls::stream<bool>& outStreamEnd) {
#pragma HLS INLINE off

    ap_uint<WIDTH> arry[2] = {0, 0};
#pragma HLS array_partition variable = arry dim = 1
    ap_uint<2> empty = 0;
    ap_uint<2> store = 0;
    ap_uint<2> last = 0;
    bool write = true;
LOOP_MERGE2_1:
    do {
#pragma HLS PIPELINE II = 1
        empty[0] = !inStreamEnd0.empty() && !last[0];
        empty[1] = !inStreamEnd1.empty() && !last[1];

        if (store == 3) {
            // wait for write
        } else if (store == 2) {
            // read for arry[0]
            if (empty == 3) {
                arry[0] = inStream0.read();
                last[0] = inStreamEnd0.read();
                store = 3;
            } else if (empty == 2) {
                arry[0] = inStream1.read();
                last[1] = inStreamEnd1.read();
                store = 3;
            } else if (empty == 1) {
                arry[0] = inStream0.read();
                last[0] = inStreamEnd0.read();
                store = 3;
            } else {
                store = 2;
            }
        } else if (store == 1) {
            // read for arry[1]
            if (empty == 3) {
                arry[1] = inStream1.read();
                last[1] = inStreamEnd1.read();
                store = 3;
            } else if (empty == 2) {
                arry[1] = inStream1.read();
                last[1] = inStreamEnd1.read();
                store = 3;
            } else if (empty == 1) {
                arry[1] = inStream0.read();
                last[0] = inStreamEnd0.read();
                store = 3;
            } else {
                store = 1;
            }
        } else {
            // store == 0, read for arry[0] and arry[1]
            if (empty == 3) {
                arry[0] = inStream0.read();
                last[0] = inStreamEnd0.read();
                arry[1] = inStream1.read();
                last[1] = inStreamEnd1.read();
                store = 3;
            } else if (empty == 2) {
                arry[1] = inStream1.read();
                last[1] = inStreamEnd1.read();
                store = 2;
            } else if (empty == 1) {
                arry[0] = inStream0.read();
                last[0] = inStreamEnd0.read();
                store = 1;
            } else {
                store = 0;
            }
        }

        ap_uint<2 * WIDTH> key = (arry[1], arry[0]);
        if ((store == 3) && !outStream.full() && !outStreamEnd.full()) {
            // write out
            if (key != 0) {
                outStream.write(key);
                outStreamEnd.write(false);
#ifndef __SYNTHESIS__
#ifdef DEBUG_HOP
// std::cout << "Merge arry[0]=" << arry[0] << " arry[1]=" << arry[1] << std::endl;
#endif
#endif
            }
            arry[0] = 0;
            arry[1] = 0;
            store = 0;
        }
    } while (last != 3);
    if (store != 0) {
        // write out residual
        ap_uint<2 * WIDTH> key = (arry[1], arry[0]);
        outStream.write(key);
        outStreamEnd.write(false);
    }
    if (PADD) outStream.write(0);
    if (END) outStreamEnd.write(true);
}

template <bool PADD, bool END>
void merge128to512(hls::stream<ap_uint<128> > inStream[4],
                   hls::stream<bool> inStreamEnd[4],
                   hls::stream<ap_uint<512> >& outStream,
                   hls::stream<bool>& outStreamEnd) {
#pragma HLS INLINE off
#pragma HLS DATAFLOW

    hls::stream<ap_uint<256> > mergeStream[2];
#pragma HLS stream variable = mergeStream depth = 8
#pragma HLS resource variable = mergeStream core = FIFO_SRL
    hls::stream<bool> mergeStreamEnd[2];
#pragma HLS stream variable = mergeStreamEnd depth = 8
#pragma HLS resource variable = mergeStreamEnd core = FIFO_SRL

    transfer2to1<128, true, true>(inStream[0], inStreamEnd[0], inStream[1], inStreamEnd[1], mergeStream[0],
                                  mergeStreamEnd[0]);

    transfer2to1<128, true, true>(inStream[2], inStreamEnd[2], inStream[3], inStreamEnd[3], mergeStream[1],
                                  mergeStreamEnd[1]);

    transfer2to1<256, PADD, END>(mergeStream[0], mergeStreamEnd[0], mergeStream[1], mergeStreamEnd[1], outStream,
                                 outStreamEnd);
}

void loadHopMultipleIndex(ap_uint<32> numHop,
                          ap_uint<32> intermediate,
                          ap_uint<32> byPass,
                          ap_uint<32> offsetStart,
                          ap_uint<32> offsetEnd,
                          hls::stream<ap_uint<128> >& offsetStream,
                          hls::stream<bool>& offsetStreamEnd,
                          ap_uint<128>* index,
                          hls::stream<ap_uint<512> >& localIndexStream,
                          hls::stream<bool>& localIndexStreamEnd,
                          hls::stream<ap_uint<512> >& netIndexStream,
                          hls::stream<bool>& netIndexStreamEnd,
                          hls::stream<ap_uint<64> >& aggrOutStream,
                          hls::stream<bool>& aggrOutStreamEnd,
                          hls::stream<ap_uint<512> >& localResultStream,
                          hls::stream<bool>& localResultStreamEnd) {
#pragma HLS INLINE off
#pragma HLS DATAFLOW

#ifndef __SYNTHESIS__
    std::cout << "================Load Multiple Hop Index==============" << std::endl;
#endif

    hls::stream<ap_uint<128> > addrStream("addrStream");
#pragma HLS stream variable = addrStream depth = 8
#pragma HLS resource variable = addrStream core = FIFO_SRL
    hls::stream<bool> addrStreamEnd("addrStreamEnd");
#pragma HLS stream variable = addrStreamEnd depth = 8
#pragma HLS resource variable = addrStreamEnd core = FIFO_SRL

    generateIndexAddr(offsetStream, offsetStreamEnd, addrStream, addrStreamEnd);

    hls::stream<ap_uint<128> > localStream[4];
#pragma HLS stream variable = localStream depth = 8
#pragma HLS resource variable = localStream core = FIFO_SRL
    hls::stream<bool> localStreamEnd[4];
#pragma HLS stream variable = localStreamEnd depth = 8
#pragma HLS resource variable = localStreamEnd core = FIFO_SRL
    hls::stream<ap_uint<128> > netStream[4];
#pragma HLS stream variable = netStream depth = 8
#pragma HLS resource variable = netStream core = FIFO_SRL
    hls::stream<bool> netStreamEnd[4];
#pragma HLS stream variable = netStreamEnd depth = 8
#pragma HLS resource variable = netStreamEnd core = FIFO_SRL
    hls::stream<ap_uint<64> > aggrStream[4];
#pragma HLS stream variable = aggrStream depth = 8
#pragma HLS resource variable = aggrStream core = FIFO_SRL
    hls::stream<bool> aggrStreamEnd[4];
#pragma HLS stream variable = aggrStreamEnd depth = 8
#pragma HLS resource variable = aggrStreamEnd core = FIFO_SRL
    hls::stream<ap_uint<128> > resultStream[4];
#pragma HLS stream variable = resultStream depth = 8
#pragma HLS resource variable = resultStream core = FIFO_SRL
    hls::stream<bool> resultStreamEnd[4];
#pragma HLS stream variable = resultStreamEnd depth = 8
#pragma HLS resource variable = resultStreamEnd core = FIFO_SRL

    loadIndex(numHop, intermediate, byPass, offsetStart, offsetEnd, addrStream, addrStreamEnd, index, localStream,
              localStreamEnd, netStream, netStreamEnd, aggrStream, aggrStreamEnd, resultStream, resultStreamEnd);

#ifndef __SYNTHESIS__
    std::cout << "================Merge Local Index=============" << std::endl;
#endif

    merge128to512<true, true>(localStream, localStreamEnd, localIndexStream, localIndexStreamEnd);

#ifndef __SYNTHESIS__
    std::cout << "================Merge Nex Stream==============" << std::endl;
#endif

    merge128to512<false, true>(netStream, netStreamEnd, netIndexStream, netIndexStreamEnd);

#ifndef __SYNTHESIS__
    std::cout << "===============Merge Result Stream============" << std::endl;
#endif

    merge128to512<false, false>(resultStream, resultStreamEnd, localResultStream, localResultStreamEnd);

#ifndef __SYNTHESIS__
    std::cout << "================Merge aggr Stream=============" << std::endl;
#endif

    ap_uint<32> tmp;
    merge4to1<64, true>(aggrStream[0], aggrStreamEnd[0], aggrStream[1], aggrStreamEnd[1], aggrStream[2],
                        aggrStreamEnd[2], aggrStream[3], aggrStreamEnd[3], tmp, aggrOutStream, aggrOutStreamEnd);
}

template <int maxDevice>
void networkSender(hls::stream<ap_uint<512> >& indexStreamIn,
                   hls::stream<bool>& indexStreamInEnd,
                   ap_uint<64> cardTable[maxDevice],
                   hls::stream<ap_uint<512> >& indexStreamOut,
                   hls::stream<bool>& indexStreamOutEnd) {
#pragma HLS INLINE off

    bool end = indexStreamInEnd.read();
    while (!end) {
#pragma HLS PIPELINE II = 1

        ap_uint<32> src, des, idx;
        ap_uint<16> hop, id;
        ap_uint<512> tmp0, tmp1;
        tmp0 = indexStreamIn.read();
        end = indexStreamInEnd.read();

        for (int j = 0; j < 4; j++) {
            src = tmp0(31 + 128 * j, 128 * j);
            des = tmp0(63 + 128 * j, 32 + 128 * j);
            idx = tmp0(95 + 128 * j, 64 + 128 * j);
            hop = tmp0(111 + 128 * j, 96 + 128 * j);

            for (int i = 0; i < maxDevice; i++) {
                ap_uint<64> table = cardTable[i];
                ap_uint<32> offsetEnd = table(31, 0);
                ap_uint<16> cardId = table(47, 32);
                if (idx < offsetEnd) {
                    id = cardId;
                }
            }

            tmp1(31 + 128 * j, 128 * j) = src;
            tmp1(63 + 128 * j, 32 + 128 * j) = des;
            tmp1(95 + 128 * j, 64 + 128 * j) = idx;
            tmp1(111 + 128 * j, 96 + 128 * j) = hop;
            tmp1(127 + 128 * j, 112 + 128 * j) = id;
        }
        indexStreamOut.write(tmp1);
        indexStreamOutEnd.write(false);
    }
}

template <int PU>
void hashProcess(ap_uint<32> byPass,
                 ap_uint<32> hashSize,
                 hls::stream<ap_uint<64> >& hopStream,
                 hls::stream<bool>& hopStreamEnd,
                 hls::stream<ap_uint<96> >& aggrStream,
                 hls::stream<bool>& aggrStreamEnd) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "=======================hashProcess===================" << std::endl;
#endif
    bool end = hopStreamEnd.read();
    while (!end) {
#pragma HLS PIPELINE II = 1

        ap_uint<32> src, des;
        ap_uint<64> tmp0 = hopStream.read();
        end = hopStreamEnd.read();

        src = tmp0(31, 0);
        des = tmp0(63, 32);

        ap_uint<96> tmp1;
        ap_uint<64> key = (des, src);
        ap_uint<64> hash;
        database::details::hashlookup3_seed_core<64>(key, 0xbeef, hash);
        tmp1(63, 0) = tmp0(63, 0);
        if (hashSize <= 4096) {
            tmp1(75, 64) = hash(11 + PU, 0);
            tmp1(95, 76) = 0;
        } else if (hashSize <= 65536) {
            tmp1(79, 64) = hash(15 + PU, 0);
            tmp1(95, 80) = 0;
        } else
            tmp1(95, 64) = hash(31, 0);

#ifndef __SYNTHESIS__
#ifdef DEBUG_AGGR
        if ((src == 124311) && (des == 721))
            std::cout << "aggrStream: src=" << src << " des=" << des << " hash=" << tmp1(95, 64) << std::endl;
#endif
#endif
        if (byPass == 0) {
            aggrStream.write(tmp1);
            aggrStreamEnd.write(false);
        }
    }
    aggrStreamEnd.write(true);
}

template <int depth, int PU>
void aggrCounter(hls::stream<ap_uint<96> >& aggrStream,
                 hls::stream<bool>& aggrStreamEnd,
                 ap_uint<32>& numAggr,
#ifndef __SYNTHESIS__
                 ap_uint<96>* aggrURAM[1 << PU],
#else
                 ap_uint<96> aggrURAM[1 << PU][1 << depth],
#endif
                 hls::stream<ap_uint<64> >& overflowStream,
                 hls::stream<bool>& overflowStreamEnd) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "=======================aggrCounter===================" << std::endl;
#endif
    const int maxBatch = 1 << depth;
    ap_uint<96> elem = 0;
    ap_uint<96> elem_temp[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    ap_uint<depth + PU> idx_temp[12] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
                                        0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
#pragma HLS array_partition variable = elem_temp complete
#pragma HLS array_partition variable = idx_temp complete

    bool end = aggrStreamEnd.read();

    ap_uint<32> src, des, hash;
    ap_uint<depth + PU> arry_idx;
aggrCountLoop:
    while (!end) {
#pragma HLS PIPELINE II = 1
#pragma HLS DEPENDENCE variable = aggrURAM pointer inter false

        ap_uint<96> tmp0 = aggrStream.read();
        end = aggrStreamEnd.read();

        src = tmp0(31, 0);
        des = tmp0(63, 32);
        hash = tmp0(95, 64);
        arry_idx = hash(depth + PU - 1, 0);

        if (arry_idx == idx_temp[0]) {
            elem = elem_temp[0];
        } else if (arry_idx == idx_temp[1]) {
            elem = elem_temp[1];
        } else if (arry_idx == idx_temp[2]) {
            elem = elem_temp[2];
        } else if (arry_idx == idx_temp[3]) {
            elem = elem_temp[3];
        } else if (arry_idx == idx_temp[4]) {
            elem = elem_temp[4];
        } else if (arry_idx == idx_temp[5]) {
            elem = elem_temp[5];
        } else if (arry_idx == idx_temp[6]) {
            elem = elem_temp[6];
        } else if (arry_idx == idx_temp[7]) {
            elem = elem_temp[7];
        } else if (arry_idx == idx_temp[8]) {
            elem = elem_temp[8];
        } else if (arry_idx == idx_temp[9]) {
            elem = elem_temp[9];
        } else if (arry_idx == idx_temp[10]) {
            elem = elem_temp[10];
        } else if (arry_idx == idx_temp[11]) {
            elem = elem_temp[11];
        } else {
            elem = aggrURAM[arry_idx(PU - 1, 0)][arry_idx(depth + PU - 1, PU)];
        }

        ap_uint<64> key = elem(63, 0);
        ap_uint<32> pld = elem(95, 64);
        ap_uint<96> new_elem;

#ifndef __SYNTHESIS__
#ifdef DEBUG_AGGR
        if ((src == 124311) && (des == 721))
            std::cout << "base: src=" << src << " des=" << des << " chip_idx=" << arry_idx(PU - 1, 0)
                      << " arry_idx=" << arry_idx(depth + PU - 1, PU) << " (des, src)=" << (des, src) << " key=" << key
                      << " pld=" << pld << std::endl;
#endif
#endif

        if ((key == (des, src)) || (elem == 0)) {
            new_elem(95, 64) = pld + 1;
            new_elem(63, 32) = des;
            new_elem(31, 0) = src;
            aggrURAM[arry_idx(PU - 1, 0)][arry_idx(depth + PU - 1, PU)] = new_elem;
            numAggr++;
        } else {
            overflowStream.write((des, src));
            overflowStreamEnd.write(false);
        }

        for (int i = 11; i > 0; i--) {
#pragma HLS unroll
            idx_temp[i] = idx_temp[i - 1];
            elem_temp[i] = elem_temp[i - 1];
        }
        idx_temp[0] = arry_idx;
        elem_temp[0] = new_elem;
    }
    overflowStreamEnd.write(true);
}

template <int PU>
void overflowCounter(hls::stream<ap_uint<64> >& overflowStream,
                     hls::stream<bool>& overflowStreamEnd,
#ifndef __SYNTHESIS__
                     ap_uint<96>* overflowURAM[1 << PU]
#else
                     ap_uint<96> overflowURAM[1 << PU][2048]
#endif
                     ) {
#pragma HLS INLINE off

#ifndef __SYNTHESIS__
    std::cout << "=======================overflowCounter===================" << std::endl;
#endif

    bool end = overflowStreamEnd.read();
    ap_uint<32> max_cnt = 0;
    bool write_success = true;
    ap_uint<64> key_in = 0;
    ap_uint<13> idx = 1;
    ap_uint<3> chip = 0;

overflowCountLoop:
    while (!end || !write_success) {
#pragma HLS PIPELINE II = 1

        if (write_success) {
            key_in = overflowStream.read();
            end = overflowStreamEnd.read();

            chip = 0;
            idx = 1;
        } else {
            if (PU == 3) {
                if (chip == 7) {
                    idx++;
                    chip = 0;
                } else {
                    chip++;
                }
            } else if (PU == 2) {
                if (chip == 3) {
                    idx++;
                    chip = 0;
                } else {
                    chip++;
                }
            } else if (PU == 1) {
                if (chip == 1) {
                    idx++;
                    chip = 0;
                } else {
                    chip++;
                }
            } else {
                idx++;
                chip = 0;
            }
        }

        ap_uint<96> elem = overflowURAM[chip][idx];
        ap_uint<64> key = elem(63, 0);
        ap_uint<32> pld = elem(95, 64);

        if ((key == key_in) || (elem == 0)) {
            write_success = true;

            ap_uint<96> new_elem;
            new_elem(63, 0) = key_in;
            new_elem(95, 64) = pld + 1;
            overflowURAM[chip][idx] = new_elem;
        } else {
            write_success = false;
        }
        if (idx > max_cnt) max_cnt = idx;

#ifndef __SYNTHESIS__
#ifdef DEBUG_AGGR
        ap_uint<32> src, des;
        src = key_in(31, 0);
        des = key_in(63, 32);

        if ((src == 124311) && (des == 721))
            std::cout << "overflow: src=" << src << " des=" << des << " chip_idx=" << chip << " arry_idx=" << idx
                      << " (des, src)=" << key_in << " key=" << key << " pld=" << pld << std::endl;
#endif
#endif
    }
    for (int i = 0; i < PU; i++) overflowURAM[i][0] = max_cnt;
}

template <int depth, int PU>
void hashAggrCounter(ap_uint<32> byPass,
                     ap_uint<32> hashSize,
                     hls::stream<ap_uint<64> >& hopStream,
                     hls::stream<bool>& hopStreamEnd,
                     ap_uint<32>& numAggr,
#ifndef __SYNTHESIS__
                     ap_uint<96>* aggrURAM[1 << PU],
                     ap_uint<96>* overflowURAM[1 << PU]
#else
                     ap_uint<96> aggrURAM[1 << PU][1 << depth],
                     ap_uint<96> overflowURAM[1 << PU][2048]
#endif

                     ) {
#pragma HLS DATAFLOW
#pragma HLS INLINE off

    hls::stream<ap_uint<96> > aggrStream("aggrStream");
#pragma HLS stream variable = aggrStream depth = 512
#pragma HLS resource variable = aggrStream core = FIFO_BRAM
    hls::stream<bool> aggrStreamEnd("aggrStreamEnd");
#pragma HLS stream variable = aggrStreamEnd depth = 512
#pragma HLS resource variable = aggrStreamEnd core = FIFO_SRL

    hls::stream<ap_uint<64> > overflowStream("overflowStream");
#pragma HLS stream variable = overflowStream depth = 512
#pragma HLS resource variable = overflowStream core = FIFO_BRAM
    hls::stream<bool> overflowStreamEnd("overflowStreamEnd");
#pragma HLS stream variable = overflowStreamEnd depth = 512
#pragma HLS resource variable = overflowStreamEnd core = FIFO_SRL

    hashProcess<PU>(byPass, hashSize, hopStream, hopStreamEnd, aggrStream, aggrStreamEnd);

    aggrCounter<depth, PU>(aggrStream, aggrStreamEnd, numAggr, aggrURAM, overflowStream, overflowStreamEnd);

    overflowCounter<PU>(overflowStream, overflowStreamEnd, overflowURAM);
}

template <bool PADD, bool END>
void streamTransfer128to512(hls::stream<ap_uint<128> >& strmIn,
                            hls::stream<bool>& strmInEnd,
                            hls::stream<ap_uint<512> >& strmOut,
                            hls::stream<bool>& strmOutEnd) {
#pragma HLS INLINE off

    bool end = strmInEnd.read();
    ap_uint<512> tmpOut = 0;
    ap_uint<2> j = 0;
    while (!end) {
#pragma HLS PIPELINE II = 1

        ap_uint<128> tmpIn = strmIn.read();
        end = strmInEnd.read();

        if (j == 0) {
            tmpOut(127, 0) = tmpIn;
            tmpOut(511, 128) = 0;
        } else if (j == 1) {
            tmpOut(255, 128) = tmpIn;
            tmpOut(511, 256) = 0;
        } else if (j == 2) {
            tmpOut(383, 256) = tmpIn;
            tmpOut(511, 384) = 0;
        } else {
            tmpOut(511, 384) = tmpIn;
            strmOut.write(tmpOut);
            strmOutEnd.write(false);
        }
        j++;
    }
    if (j != 0) {
        strmOut.write(tmpOut);
        strmOutEnd.write(false);
    }
    if (PADD) strmOut.write(0);
    if (END) strmOutEnd.write(true);
}

template <bool PADD, bool END>
void streamTransfer64to512(hls::stream<ap_uint<128> >& strmIn,
                           hls::stream<bool>& strmInEnd,
                           hls::stream<ap_uint<512> >& strmOut,
                           hls::stream<bool>& strmOutEnd) {
#pragma HLS INLINE off

    bool end = strmInEnd.read();
    ap_uint<512> tmpOut = 0;
    ap_uint<3> j = 0;
    while (!end) {
#pragma HLS PIPELINE II = 1

        ap_uint<128> tmp = strmIn.read();
        ap_uint<64> tmpIn = tmp(63, 0);
        end = strmInEnd.read();

        if (j == 0) {
            tmpOut(63, 0) = tmpIn;
            tmpOut(511, 64) = 0;
        } else if (j == 1) {
            tmpOut(127, 64) = tmpIn;
            tmpOut(511, 127) = 0;
        } else if (j == 2) {
            tmpOut(195, 128) = tmpIn;
            tmpOut(511, 196) = 0;
        } else if (j == 3) {
            tmpOut(255, 192) = tmpIn;
            tmpOut(511, 256) = 0;
        } else if (j == 4) {
            tmpOut(319, 256) = tmpIn;
            tmpOut(511, 320) = 0;
        } else if (j == 5) {
            tmpOut(383, 320) = tmpIn;
            tmpOut(511, 384) = 0;
        } else if (j == 6) {
            tmpOut(447, 384) = tmpIn;
            tmpOut(511, 448) = 0;
        } else {
            tmpOut(511, 448) = tmpIn;
            strmOut.write(tmpOut);
            strmOutEnd.write(false);
        }
        j++;
    }
    if (j != 0) {
        strmOut.write(tmpOut);
        strmOutEnd.write(false);
    }
    if (PADD) strmOut.write(0);
    if (END) strmOutEnd.write(true);
}

void nHopStreamTransfer(ap_uint<32> intermediate,
                        ap_uint<32> byPass,
                        hls::stream<ap_uint<128> >& strmIn,
                        hls::stream<bool>& strmInEnd,
                        hls::stream<ap_uint<512> >& strmOut,
                        hls::stream<bool>& strmOutEnd) {
#pragma HLS INLINE off

    if ((byPass != 0) && (intermediate == 0)) {
#ifndef __SYNTHESIS__
        std::cout << "transfer 64bit stream to 512bit" << std::endl;
#endif
        streamTransfer64to512<true, true>(strmIn, strmInEnd, strmOut, strmOutEnd);
    } else {
#ifndef __SYNTHESIS__
        std::cout << "transfer 128bit stream to 512bit" << std::endl;
#endif
        streamTransfer128to512<true, true>(strmIn, strmInEnd, strmOut, strmOutEnd);
    }
}

template <int maxDevice>
void hopProcessingUnit(ap_uint<32> numHop,
                       ap_uint<32> intermediate,
                       ap_uint<32> byPass,
                       ap_uint<32> duplicate,
                       hls::stream<ap_uint<128> >& dispatchStream,
                       hls::stream<bool>& dispatchStreamEnd,

                       ap_uint<32> offsetStart,
                       ap_uint<32> offsetEnd,
                       ap_uint<32> indexTable,
                       ap_uint<64> cardTable[maxDevice],
                       unsigned* offset,
                       ap_uint<128>* index,

                       hls::stream<ap_uint<512> >& localStream,
                       hls::stream<bool>& localStreamEnd,
                       hls::stream<ap_uint<512> >& networkStream,
                       hls::stream<bool>& networkStreamEnd,
                       hls::stream<ap_uint<64> >& aggrStream,
                       hls::stream<bool>& aggrStreamEnd,
                       hls::stream<ap_uint<512> >& outStream,
                       hls::stream<bool>& outStreamEnd) {
#pragma HLS INLINE off
#pragma HLS DATAFLOW

    hls::stream<ap_uint<128> > offsetStream;
#pragma HLS stream variable = offsetStream depth = 512
#pragma HLS resource variable = offsetStream core = FIFO_BRAM
    hls::stream<bool> offsetStreamEnd;
#pragma HLS stream variable = offsetStreamEnd depth = 512
#pragma HLS resource variable = offsetStreamEnd core = FIFO_SRL

    hls::stream<ap_uint<512> > netStream;
#pragma HLS stream variable = netStream depth = 512
#pragma HLS resource variable = netStream core = FIFO_BRAM
    hls::stream<bool> netStreamEnd;
#pragma HLS stream variable = netStreamEnd depth = 512
#pragma HLS resource variable = netStreamEnd core = FIFO_SRL

    loadHopOffset(numHop, duplicate, dispatchStream, dispatchStreamEnd, indexTable, offset, offsetStream,
                  offsetStreamEnd);

    loadHopMultipleIndex(numHop, intermediate, byPass, offsetStart, offsetEnd, offsetStream, offsetStreamEnd, index,
                         localStream, localStreamEnd, netStream, netStreamEnd, aggrStream, aggrStreamEnd, outStream,
                         outStreamEnd);

    networkSender<maxDevice>(netStream, netStreamEnd, cardTable, networkStream, networkStreamEnd);
}

template <int _WAxi, int _WStrm, int _BurstLen>
void countForBurst(hls::stream<ap_uint<_WStrm> >& istrm,
                   hls::stream<bool>& e_istrm,
                   hls::stream<ap_uint<_WAxi> >& axi_strm,
                   hls::stream<ap_uint<8> >& nb_strm) {
    const int N = _WAxi / _WStrm;
    ap_uint<_WAxi> tmp;
    bool isLast;
    int nb = 0;
    int bs = 0;

    isLast = e_istrm.read();
doing_loop:
    while (!isLast) {
#pragma HLS pipeline II = 1
        isLast = e_istrm.read();
        int offset = bs * _WStrm;
        ap_uint<_WStrm> t = istrm.read();
        tmp.range(offset + _WStrm - 1, offset) = t(_WStrm - 1, 0);
        if (bs == (N - 1)) {
            axi_strm.write(tmp);
            if (nb == (_BurstLen - 1)) {
                nb_strm.write(_BurstLen);
                nb = 0;
            } else
                ++nb;
            bs = 0;
        } else
            ++bs;
    }
    // not enough one axi
    if (bs != 0) {
    doing_not_enough:
        for (; bs < N; ++bs) {
#pragma HLS unroll
            int offset = bs * _WStrm;
            tmp.range(offset + _WStrm - 1, offset) = 0;
        }
        axi_strm.write(tmp);
        ++nb;
    }
    if (nb != 0) {
        nb_strm.write(nb);
    }
    nb_strm.write(0);
}

template <int _WAxi, int _WStrm, int _BurstLen>
void burstWrite(ap_uint<_WAxi>* wbuf, hls::stream<ap_uint<_WAxi> >& axi_strm, hls::stream<ap_uint<8> >& nb_strm) {
    int total = 0;
    ap_uint<_WAxi> tmp;
    int n = nb_strm.read();
doing_burst:
    while (n) {
    doing_one_burst:
        for (int i = 0; i < n; i++) {
#pragma HLS pipeline II = 1
            tmp = axi_strm.read();
            wbuf[total * _BurstLen + i] = tmp;

#ifndef __SYNTHESIS__
#ifdef DEBUG_OUTPUT
            for (int j = 0; j < 4; j++) {
                ap_uint<32> src, des, idx, hop;
                src = tmp(128 * j + 31, 128 * j);
                des = tmp(128 * j + 63, 128 * j + 32);
                idx = tmp(128 * j + 95, 128 * j + 64);
                hop = tmp(128 * j + 127, 128 * j + 96);

                if ((src == 38) && (des == 45))
                std::cout << "writeOut: addr=" << total * _BurstLen + i << " src=" << src << " des=" << des
                          << " idx=" << idx << " hop=" << hop << std::endl;
            }
#endif
#endif
        }
        n = nb_strm.read();
        total++;
    }
}

void writeOut(hls::stream<ap_uint<512> >& streamIn, hls::stream<bool>& streamInEnd, ap_uint<512>* buffer) {
#pragma HLS INLINE off
    const int fifo_buf = 512;

#pragma HLS dataflow

    hls::stream<ap_uint<512> > axi_strm;
    hls::stream<ap_uint<8> > nb_strm;
#pragma HLS stream variable = nb_strm depth = 2
#pragma HLS stream variable = axi_strm depth = fifo_buf

    countForBurst<512, 512, 64>(streamIn, streamInEnd, axi_strm, nb_strm);

    burstWrite<512, 512, 64>(buffer, axi_strm, nb_strm);
}

template <int depth>
void initSingleURAM(ap_uint<32> maxUpdateSize, ap_uint<96> aggrURAM[1 << depth], ap_uint<96> overflowURAM[2048]) {
#pragma HLS INLINE off

    for (int j = 0; j < maxUpdateSize; j++) {
#pragma HLS PIPELINE II = 1
        aggrURAM[j] = 0;
    }
    for (int j = 0; j < 2048; j++) {
#pragma HLS PIPELINE II = 1
        overflowURAM[j] = 0;
    }
}

template <int depth, int PU>
void initURAM(ap_uint<32> byPass,
              ap_uint<32> hashSize,
#ifndef __SYNTHESIS__
              ap_uint<96>* aggrURAM[PU],
              ap_uint<96>* overflowURAM[PU]
#else
              ap_uint<96> aggrURAM[PU][1 << depth],
              ap_uint<96> overflowURAM[PU][2048]
#endif
              ) {
#pragma HLS INLINE off

    const int maxBatch = 1 << depth;

    ap_uint<32> maxUpdateSize;
    if (hashSize <= 4096)
        maxUpdateSize = 4096;
    else if (hashSize <= 65536)
        maxUpdateSize = 65536 > maxBatch ? (ap_uint<32>)maxBatch : (ap_uint<32>)65536;
    else
        maxUpdateSize = maxBatch;

    if (byPass == 0) {
        for (int i = 0; i < PU; i++) {
#pragma HLS UNROLL

            initSingleURAM<depth>(maxUpdateSize, aggrURAM[i], overflowURAM[i]);
        }
    }
}

template <int depth>
void readURAM(ap_uint<32> maxUpdateSize,
              ap_uint<96> aggrURAM[1 << depth],
              ap_uint<96> overflowURAM[2048],
              hls::stream<ap_uint<128> >& aggrOutStream,
              hls::stream<bool>& aggrOutStreamEnd) {
#pragma HLS INLINE off

    for (int j = 0; j < maxUpdateSize; j++) {
#pragma HLS PIPELINE II = 1
#pragma HLS DEPENDENCE variable = aggrURAM pointer inter false
        ap_uint<96> tmp = aggrURAM[j];
        aggrURAM[j] = 0;

        if (tmp != 0) {
            aggrOutStream.write(tmp);
            aggrOutStreamEnd.write(false);

#ifndef __SYNTHESIS__
#ifdef DEBUG_AGGR
            ap_uint<32> src, des, count;
            src = tmp(31, 0);
            des = tmp(63, 32);
            count = tmp(95, 64);
            if ((src == 124311) && (des == 721))
                std::cout << "j=" << j << " src=" << src << " des=" << des << " count=" << count << std::endl;
#endif
#endif
        }
    }

    ap_uint<32> overflow_cnt = overflowURAM[0];
    overflowURAM[0] = 0;
#ifndef __SYNTHESIS__
    std::cout << "overflowCNT=" << overflow_cnt << std::endl;
#endif

    for (int j = 0; j <= overflow_cnt / 8; j++) {
#pragma HLS PIPELINE II = 1
#pragma HLS DEPENDENCE variable = overflowURAM pointer inter false
        ap_uint<96> tmp = overflowURAM[j + 1];
        overflowURAM[j + 1] = 0;

        if (tmp != 0) {
            aggrOutStream.write(tmp);
            aggrOutStreamEnd.write(false);

#ifndef __SYNTHESIS__
#ifdef DEBUG_AGGR
            ap_uint<32> src, des, count;
            src = tmp(31, 0);
            des = tmp(63, 32);
            count = tmp(95, 64);
            if ((src == 124311) && (des == 721))
                std::cout << "j=" << j + 1 << " src=" << src << " des=" << des << " count=" << count << std::endl;
#endif
#endif
        }
    }
    aggrOutStreamEnd.write(true);
}

template <int depth>
void outputSingleURAM(ap_uint<32> maxUpdateSize,
                      ap_uint<96> aggrURAM[1 << depth],
                      ap_uint<96> overflowURAM[2048],
                      hls::stream<ap_uint<512> >& aggrOutStream,
                      hls::stream<bool>& aggrOutStreamEnd) {
#pragma HLS INLINE off
#pragma HLS DATAFLOW

    hls::stream<ap_uint<128> > URAMStream;
#pragma HLS stream variable = URAMStream depth = 8
#pragma HLS resource variable = URAMStream core = FIFO_SRL
    hls::stream<bool> URAMStreamEnd;
#pragma HLS stream variable = URAMStreamEnd depth = 8
#pragma HLS resource variable = URAMStreamEnd core = FIFO_SRL

    readURAM<depth>(maxUpdateSize, aggrURAM, overflowURAM, URAMStream, URAMStreamEnd);

    streamTransfer128to512<false, false>(URAMStream, URAMStreamEnd, aggrOutStream, aggrOutStreamEnd);
}

template <int depth, int PU>
void outputURAM(ap_uint<32> byPass,
                ap_uint<32> hashSize,
                ap_uint<32> numAggr,
                ap_uint<32> numResidual,
#ifndef __SYNTHESIS__
                ap_uint<96>* aggrURAM[PU],
                ap_uint<96>* overflowURAM[PU],
#else
                ap_uint<96> aggrURAM[PU][1 << depth],
                ap_uint<96> overflowURAM[PU][2048],
#endif
                hls::stream<ap_uint<512> > aggrOutStream[8],
                hls::stream<bool> aggrOutStreamEnd[8]) {
#pragma HLS INLINE off

    const int maxBatch = 1 << depth;
    ap_uint<32> maxUpdateSize;
    if (hashSize <= 4096)
        maxUpdateSize = 4096;
    else if (hashSize <= 65536)
        maxUpdateSize = 65536 > maxBatch ? (ap_uint<32>)maxBatch : (ap_uint<32>)65536;
    else
        maxUpdateSize = maxBatch;

#ifndef __SYNTHESIS__
    std::cout << "numAggr=" << numAggr << " maxUpdateSize=" << maxUpdateSize << std::endl;
#endif

    if ((byPass == 0) && (numAggr != 0) && (numResidual == 0)) {
        for (int i = 0; i < PU; i++) {
#pragma HLS UNROLL

            outputSingleURAM<depth>(maxUpdateSize, aggrURAM[i], overflowURAM[i], aggrOutStream[i], aggrOutStreamEnd[i]);
        }
    }
}

template <int depth, int maxDevice, int PU>
void nHopCore(bool loadBatch,
              bool& batchEnd,
              ap_uint<32> numHop,
              ap_uint<32> intermediate,
              ap_uint<32> byPass,
              ap_uint<32> duplicate,
              ap_uint<32> hashSize,
              ap_uint<32> numPairs,
              hls::stream<ap_uint<512> >& pair,
              hls::stream<bool>& pairEnd,

              ap_uint<32> offsetStart,
              ap_uint<32> offsetEnd,
              ap_uint<32> offsetTable[PU + 3],
              ap_uint<32> indexTable[PU + 1],
              ap_uint<64> cardTable[PU][maxDevice],

              unsigned* offset0,
              ap_uint<128>* index0,
              unsigned* offset1,
              ap_uint<128>* index1,
              unsigned* offset2,
              ap_uint<128>* index2,
              unsigned* offset3,
              ap_uint<128>* index3,
              
#ifndef __SYNTHESIS__
              ap_uint<96>* aggrURAM[PU],
              ap_uint<96>* overflowURAM[PU],
#else
              ap_uint<96> aggrURAM[PU][1 << depth],
              ap_uint<96> overflowURAM[PU][2048],
#endif
              ap_uint<32>& numAggr,
              ap_uint<32>& numResidual,
              ap_uint<512>* bufferPing,
              ap_uint<512>* bufferPong,
              hls::stream<ap_uint<512> > netStream[8],
              hls::stream<bool> netStreamEnd[8],
              hls::stream<ap_uint<512> > outStream[8],
              hls::stream<bool> outStreamEnd[8]) {
#pragma HLS INLINE off
#pragma HLS DATAFLOW

    hls::stream<ap_uint<128> > pairStream[4];
#pragma HLS stream variable = pairStream depth = 512
#pragma HLS resource variable = pairStream core = FIFO_BRAM
    hls::stream<bool> pairStreamEnd[4];
#pragma HLS stream variable = pairStreamEnd depth = 512
#pragma HLS resource variable = pairStreamEnd core = FIFO_SRL

    load(loadBatch, batchEnd, numPairs, pair, pairEnd, bufferPing, pairStream, pairStreamEnd);

    hls::stream<ap_uint<128> > switchStream[8];
#pragma HLS stream variable = switchStream depth = 512
#pragma HLS resource variable = switchStream core = FIFO_BRAM
    hls::stream<bool> switchStreamEnd[8];
#pragma HLS stream variable = switchStreamEnd depth = 512
#pragma HLS resource variable = switchStreamEnd core = FIFO_SRL

    switchPairs<PU>(duplicate, offsetTable, pairStream, pairStreamEnd, switchStream, switchStreamEnd);

    hls::stream<ap_uint<512> > indexStream[8];
#pragma HLS stream variable = indexStream depth = 8
#pragma HLS resource variable = indexStream core = FIFO_SRL
    hls::stream<bool> indexStreamEnd[8];
#pragma HLS stream variable = indexStreamEnd depth = 8
#pragma HLS resource variable = indexStreamEnd core = FIFO_SRL

    hls::stream<ap_uint<64> > aggrStream[8];
#pragma HLS stream variable = aggrStream depth = 512
#pragma HLS resource variable = aggrStream core = FIFO_BRAM
    hls::stream<bool> aggrStreamEnd[8];
#pragma HLS stream variable = aggrStreamEnd depth = 512
#pragma HLS resource variable = aggrStreamEnd core = FIFO_SRL

    if (PU >= 1)
        hopProcessingUnit<maxDevice>(numHop, intermediate, byPass, duplicate, switchStream[0], switchStreamEnd[0],
                                     offsetStart, offsetEnd, indexTable[0], cardTable[0], offset0, index0,
                                     indexStream[0], indexStreamEnd[0], netStream[0], netStreamEnd[0], aggrStream[0],
                                     aggrStreamEnd[0], outStream[0], outStreamEnd[0]);

    if (PU >= 2)
        hopProcessingUnit<maxDevice>(numHop, intermediate, byPass, duplicate, switchStream[1], switchStreamEnd[1],
                                     offsetStart, offsetEnd, indexTable[1], cardTable[1], offset1, index1,
                                     indexStream[1], indexStreamEnd[1], netStream[1], netStreamEnd[1], aggrStream[1],
                                     aggrStreamEnd[1], outStream[1], outStreamEnd[1]);

    if (PU >= 4) {
        hopProcessingUnit<maxDevice>(numHop, intermediate, byPass, duplicate, switchStream[2], switchStreamEnd[2],
                                     offsetStart, offsetEnd, indexTable[2], cardTable[2], offset2, index2,
                                     indexStream[2], indexStreamEnd[2], netStream[2], netStreamEnd[2], aggrStream[2],
                                     aggrStreamEnd[2], outStream[2], outStreamEnd[2]);

        hopProcessingUnit<maxDevice>(numHop, intermediate, byPass, duplicate, switchStream[3], switchStreamEnd[3],
                                     offsetStart, offsetEnd, indexTable[3], cardTable[3], offset3, index3,
                                     indexStream[3], indexStreamEnd[3], netStream[3], netStreamEnd[3], aggrStream[3],
                                     aggrStreamEnd[3], outStream[3], outStreamEnd[3]);
    }

    hls::stream<ap_uint<512> > localStream;
#pragma HLS stream variable = localStream depth = 512
#pragma HLS resource variable = localStream core = FIFO_BRAM
    hls::stream<bool> localStreamEnd;
#pragma HLS stream variable = localStreamEnd depth = 512
#pragma HLS resource variable = localStreamEnd core = FIFO_SRL

    hls::stream<ap_uint<64> > aggrInternalStream;
#pragma HLS stream variable = aggrInternalStream depth = 512
#pragma HLS resource variable = aggrInternalStream core = FIFO_BRAM
    hls::stream<bool> aggrInternalStreamEnd;
#pragma HLS stream variable = aggrInternalStreamEnd depth = 512
#pragma HLS resource variable = aggrInternalStreamEnd core = FIFO_SRL
    ap_uint<32> tmp;

     if (PU == 4) {
        merge4to1<512, false>(indexStream[0], indexStreamEnd[0], indexStream[1], indexStreamEnd[1], indexStream[2],
                              indexStreamEnd[2], indexStream[3], indexStreamEnd[3], numResidual, localStream,
                              localStreamEnd);

        merge4to1<64, false>(aggrStream[0], aggrStreamEnd[0], aggrStream[1], aggrStreamEnd[1], aggrStream[2],
                             aggrStreamEnd[2], aggrStream[3], aggrStreamEnd[3], tmp, aggrInternalStream,
                             aggrInternalStreamEnd);

        hashAggrCounter<depth, 2>(byPass, hashSize, aggrInternalStream, aggrInternalStreamEnd, numAggr, aggrURAM,
                                  overflowURAM);
    } else if (PU == 2) {
        merge2to1<512, false>(indexStream[0], indexStreamEnd[0], indexStream[1], indexStreamEnd[1], numResidual,
                              localStream, localStreamEnd);

        merge2to1<64, false>(aggrStream[0], aggrStreamEnd[0], aggrStream[1], aggrStreamEnd[1], tmp, aggrInternalStream,
                             aggrInternalStreamEnd);

        hashAggrCounter<depth, 1>(byPass, hashSize, aggrInternalStream, aggrInternalStreamEnd, numAggr, aggrURAM,
                                  overflowURAM);
    } else {
        merge1to1<512, false>(indexStream[0], indexStreamEnd[0], numResidual, localStream, localStreamEnd);

        merge1to1<64, false>(aggrStream[0], aggrStreamEnd[0], tmp, aggrInternalStream, aggrInternalStreamEnd);

        hashAggrCounter<depth, 0>(byPass, hashSize, aggrInternalStream, aggrInternalStreamEnd, numAggr, aggrURAM,
                                  overflowURAM);
    }

    writeOut(localStream, localStreamEnd, bufferPong);
}

template <int maxDevice, int PU>
void nHopPingPong(ap_uint<32> numHop,
                  ap_uint<32> intermediate,
                  ap_uint<32> batchSize,
                  ap_uint<32> hashSize,
                  ap_uint<32> byPass,
                  ap_uint<32> duplicate,
                  hls::stream<ap_uint<512> >& pair,
                  hls::stream<bool>& pairEnd,

                  ap_uint<32> offsetTable[PU + 3],
                  ap_uint<32> indexTable[PU + 1],
                  ap_uint<64> cardTable[PU][maxDevice],

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
                  hls::stream<ap_uint<512> > switchStream[8],
                  hls::stream<bool> switchStreamEnd[8],
                  hls::stream<ap_uint<512> > outStream[8],
                  hls::stream<bool> outStreamEnd[8]) {
#pragma HLS INLINE off

    const int depth = 16;
    const int maxBatch = 1 << depth;

#ifndef __SYNTHESIS__
    ap_uint<96>* aggrURAM[PU];
    ap_uint<96>* overflowURAM[PU];

    for (int i = 0; i < PU; i++) {
        aggrURAM[i] = (ap_uint<96>*)malloc(maxBatch * sizeof(ap_uint<96>));
        overflowURAM[i] = (ap_uint<96>*)malloc(2048 * sizeof(ap_uint<96>));
    }
#else
    ap_uint<96> aggrURAM[PU][maxBatch];
#pragma HLS bind_storage variable = aggrURAM type = ram_2p impl = uram
#pragma HLS ARRAY_PARTITION variable = aggrURAM block factor = 8
    ap_uint<96> overflowURAM[PU][2048];
#pragma HLS bind_storage variable = overflowURAM type = ram_2p impl = uram
#pragma HLS ARRAY_PARTITION variable = overflowURAM block factor = 8
#endif

#ifndef __SYNTHESIS__
    std::cout << "========================InitializeURAM===================" << std::endl;
#endif

    xf::graph::internal::Hop::initURAM<depth, PU>(byPass, hashSize, aggrURAM, overflowURAM);

    ap_uint<32> numAggr = 0;
    bool ping_pong_flag = true;
    bool batchEnd = false;
    ap_uint<32> offsetStart = offsetTable[PU+1];
    ap_uint<32> offsetEnd = offsetTable[PU+2];
    ap_uint<32> numResidual = 0;

HopLoop:
    while (numResidual > 0 || !batchEnd) {
#pragma HLS PIPELINE off
#pragma HLS loop_tripcount min = 2

#ifndef __SYNTHESIS__
        std::cout << "========================nHopCoreStart===================" << std::endl;
#endif

        bool loadBatch = numResidual == 0;
        ap_uint<32> numPair = loadBatch ? batchSize : numResidual;

        if (ping_pong_flag) {
            nHopCore<depth, maxDevice, PU>(loadBatch, batchEnd, numHop, intermediate, byPass, duplicate, hashSize,
                                           numPair, pair, pairEnd, offsetStart, offsetEnd, offsetTable, indexTable,
                                           cardTable, offset0, index0, offset1, index1, offset2, index2, offset3,
                                           index3, 
                                           aggrURAM, overflowURAM, numAggr, numResidual, bufferPing, bufferPong,
                                           switchStream, switchStreamEnd, outStream, outStreamEnd);
        } else {
            nHopCore<depth, maxDevice, PU>(loadBatch, batchEnd, numHop, intermediate, byPass, duplicate, hashSize,
                                           numPair, pair, pairEnd, offsetStart, offsetEnd, offsetTable, indexTable,
                                           cardTable, offset0, index0, offset1, index1, offset2, index2, offset3,
                                           index3, 
                                           aggrURAM, overflowURAM, numAggr, numResidual, bufferPong, bufferPing,
                                           switchStream, switchStreamEnd, outStream, outStreamEnd);
        }
        ping_pong_flag = !ping_pong_flag;

#ifndef __SYNTHESIS__
        std::cout << "numResidual=" << numResidual << std::endl;
        std::cout << "========================nHopCoreEnd=====================" << std::endl;
#endif

#ifndef __SYNTHESIS__
        std::cout << "=======================OutputAggrURAM===================" << std::endl;
#endif

        xf::graph::internal::Hop::outputURAM<depth, PU>(byPass, hashSize, numAggr, numResidual, aggrURAM, overflowURAM,
                                                        outStream, outStreamEnd);
    }

    for (int i = 0; i < PU; i++) {
#pragma HLS UNROLL
        switchStream[i].write(0);
        switchStreamEnd[i].write(true);
        outStream[i].write(0);
        outStreamEnd[i].write(true);
    }

#ifndef __SYNTHESIS__
    for (int i = 0; i < PU; i++) {
        free(aggrURAM[i]);
        free(overflowURAM[i]);
    }
#endif
}

template <int maxDevice, int PU>
void nHopTop(ap_uint<32> numHop,
             ap_uint<32> intermediate,
             ap_uint<32> batchSize,
             ap_uint<32> hashSize,
             ap_uint<32> byPass,
             ap_uint<32> duplicate,
             hls::stream<ap_uint<512> >& pair,
             hls::stream<bool>& pairEnd,

             ap_uint<32> offsetTable[PU + 3],
             ap_uint<32> indexTable[PU + 1],
             ap_uint<64> cardTable[PU][maxDevice],

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

             ap_uint<32>& numLocal,
             ap_uint<32>& numSwitch,
             ap_uint<512>* bufferLocal,
             hls::stream<ap_uint<512> >& switchOut,
             hls::stream<bool>& switchOutEnd) {
#pragma HLS INLINE off
#pragma HLS DATAFLOW

    hls::stream<ap_uint<512> > switchStream[8];
#pragma HLS stream variable = switchStream depth = 512
#pragma HLS resource variable = switchStream core = FIFO_BRAM
    hls::stream<bool> switchStreamEnd[8];
#pragma HLS stream variable = switchStreamEnd depth = 512
#pragma HLS resource variable = switchStreamEnd core = FIFO_SRL
    hls::stream<ap_uint<512> > outStream[8];
#pragma HLS stream variable = outStream depth = 512
#pragma HLS resource variable = outStream core = FIFO_BRAM
    hls::stream<bool> outStreamEnd[8];
#pragma HLS stream variable = outStreamEnd depth = 512
#pragma HLS resource variable = outStreamEnd core = FIFO_SRL

#ifndef __SYNTHESIS__
    std::cout << "=======================Ping-Pong Start==================" << std::endl;
#endif

    nHopPingPong<maxDevice, PU>(numHop, intermediate, batchSize, hashSize, byPass, duplicate, pair, pairEnd,
                                offsetTable, indexTable, cardTable, offset0, index0, offset1, index1, offset2, index2,
                                offset3, index3, 
                                bufferPing, bufferPong, switchStream, switchStreamEnd, outStream, outStreamEnd);
#ifndef __SYNTHESIS__
    std::cout << "========================Ping-Pong End===================" << std::endl;
#endif

    hls::stream<ap_uint<512> > switchMergeStream;
#pragma HLS stream variable = switchMergeStream depth = 512
#pragma HLS resource variable = switchMergeStream core = FIFO_BRAM
    hls::stream<bool> switchMergeStreamEnd;
#pragma HLS stream variable = switchMergeStreamEnd depth = 512
#pragma HLS resource variable = switchMergeStreamEnd core = FIFO_SRL
    hls::stream<ap_uint<512> > outMergeStream;
#pragma HLS stream variable = outMergeStream depth = 512
#pragma HLS resource variable = outMergeStream core = FIFO_BRAM
    hls::stream<bool> outMergeStreamEnd;
#pragma HLS stream variable = outMergeStreamEnd depth = 512
#pragma HLS resource variable = outMergeStreamEnd core = FIFO_SRL

#ifndef __SYNTHESIS__
    std::cout << "====================Output Result to Local=============" << std::endl;
#endif

    if (PU == 4) {
        merge4to1<512, false>(switchStream[0], switchStreamEnd[0], switchStream[1], switchStreamEnd[1], switchStream[2],
                              switchStreamEnd[2], switchStream[3], switchStreamEnd[3], numSwitch, switchOut,
                              switchOutEnd);

        merge4to1<512, false>(outStream[0], outStreamEnd[0], outStream[1], outStreamEnd[1], outStream[2],
                              outStreamEnd[2], outStream[3], outStreamEnd[3], numLocal, outMergeStream,
                              outMergeStreamEnd);

        writeOut(outMergeStream, outMergeStreamEnd, bufferLocal);
    } else if (PU == 2) {
        merge2to1<512, false>(switchStream[0], switchStreamEnd[0], switchStream[1], switchStreamEnd[1], numSwitch,
                              switchOut, switchOutEnd);

        merge2to1<512, false>(outStream[0], outStreamEnd[0], outStream[1], outStreamEnd[1], numLocal, outMergeStream,
                              outMergeStreamEnd);

        writeOut(outMergeStream, outMergeStreamEnd, bufferLocal);
    } else {
        // PU == 1
        merge1to1<512, false>(switchStream[0], switchStreamEnd[0], numSwitch, switchOut, switchOutEnd);

        merge1to1<512, false>(outStream[0], outStreamEnd[0], numLocal, outMergeStream, outMergeStreamEnd);

        writeOut(outMergeStream, outMergeStreamEnd, bufferLocal);
    }
}

} // namespace Hop
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

void nHop(unsigned numHop,
          unsigned intermediate,
          unsigned numPairs,
          unsigned batchSize,
          unsigned hashSize,
          unsigned byPass,
          unsigned duplicate,

          hls::stream<ap_uint<512> >& switchIn,
          hls::stream<bool>& switchInEnd,

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

          hls::stream<ap_uint<512> >& switchOut,
          hls::stream<bool>& switchOutEnd) {
#pragma HLS INLINE off

    const int puNum = 4;
    const int maxDevice = 32;
    ap_uint<32> table0[16];
#pragma HLS ARRAY_PARTITION variable = table0 complete
    ap_uint<32> table1[16];
#pragma HLS ARRAY_PARTITION variable = table1 complete
    ap_uint<64> table2[puNum][maxDevice];
#pragma HLS ARRAY_PARTITION variable = table2 complete

#ifndef __SYNTHESIS__
    std::cout << "========================LoadLUT===================" << std::endl;
#endif

LoadOffsetTable:
    for (int i = 0; i < puNum + 3; i++) {
#pragma HLS PIPELINE II = 1
        table0[i] = offsetTable[i];
#ifndef __SYNTHESIS__
        std::cout << "offsetTable[" << i << "]=" << offsetTable[i] << std::endl;
#endif
    }
LoadIndexTable:
    for (int i = 0; i < puNum + 1; i++) {
#pragma HLS PIPELINE II = 1
        table1[i] = indexTable[i];
#ifndef __SYNTHESIS__
        std::cout << "indexTable[" << i << "]=" << indexTable[i] << std::endl;
#endif
    }
LoadCardTable:
    for (int i = 0; i < maxDevice; i++) {
#pragma HLS PIPELINE II = 1
        ap_uint<64> tmp;
        tmp = cardTable[i];
        for (int j = 0; j < puNum; j++) table2[j][i] = tmp;
    }

#ifndef __SYNTHESIS__
    std::cout << "=========================nHopTopStart====================" << std::endl;
#endif

    ap_uint<32> localBufferSize;
    ap_uint<32> switchBufferSize;

    xf::graph::internal::Hop::nHopTop<maxDevice, puNum>(
        numHop, intermediate, batchSize, hashSize, byPass, duplicate, switchIn, switchInEnd, table0, table1, table2,
        offset0, index0, offset1, index1, offset2, index2, offset3, index3, bufferPing, bufferPong, localBufferSize, switchBufferSize, bufferLocal, switchOut,
        switchOutEnd);

    numOut[0] = localBufferSize;
    numOut[1] = switchBufferSize;

#ifndef __SYNTHESIS__
    std::cout << "numLocal=" << numOut[0] << " numSwitch=" << numOut[1] << std::endl;
    std::cout << "===========================nHopTopEnd=====================" << std::endl;
#endif
}

} // namespace graph
} // namespace xf

#endif
