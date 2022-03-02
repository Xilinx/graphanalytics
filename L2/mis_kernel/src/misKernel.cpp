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
#include <cstdint>
#include "ap_int.h"
#include "interface.hpp"
#include "widetype.hpp"

#ifndef __SYNTHESIS__
#include <cassert>
#endif

void checkColIdx(const uint32_t cStart,
                 const uint32_t cEnd,
                 const ap_uint<MIS_entries * 32>* colIdx0,
                 const ap_uint<MIS_entries * 32>* colIdx1,
                 const ap_uint<MIS_entries * 32>* colIdx2,
                 const ap_uint<MIS_entries * 32>* colIdx3,
                 const ap_uint<MIS_entries * 32>* colIdx4,
                 const ap_uint<MIS_entries * 32>* colIdx5,
                 const ap_uint<MIS_entries * 32>* colIdx6,
                 const ap_uint<MIS_entries * 32>* colIdx7,
                 const ap_uint<MIS_entries * 32>* colIdx8,
                 const ap_uint<MIS_entries * 32>* colIdx9,
                 const ap_uint<MIS_entries * 32>* colIdxa,
                 const ap_uint<MIS_entries * 32>* colIdxb,
                 const ap_uint<MIS_entries * 32>* colIdxc,
                 const ap_uint<MIS_entries * 32>* colIdxd,
                 const ap_uint<MIS_entries * 32>* colIdxe,
                 const ap_uint<MIS_entries * 32>* colIdxf,
                 const uint16_t l_prior[MIS_numChannels][MIS_maxRows / MIS_numChannels],
                 const ap_uint<14> rp,
                 bool& move_out,
                 bool& undet) {
    bool l_mOut[MIS_numChannels][MIS_entries];
    bool l_undet[MIS_numChannels][MIS_entries];
    for (int c = 0; c < MIS_numChannels; c++) {
        for (int pe = 0; pe < MIS_entries; pe++) {
#pragma HLS UNROLL
            l_mOut[c][pe] = false;
            l_undet[c][pe] = false;
        }
    }
    for (int c = cStart; c < cEnd; c++) {
#pragma HLS PIPELINE II = 1
        WideType<int, MIS_entries> cId[MIS_numChannels];
        cId[0x00] = colIdx0[c];
        cId[0x01] = colIdx1[c];
        cId[0x02] = colIdx2[c];
        cId[0x03] = colIdx3[c];
        cId[0x04] = colIdx4[c];
        cId[0x05] = colIdx5[c];
        cId[0x06] = colIdx6[c];
        cId[0x07] = colIdx7[c];
        cId[0x08] = colIdx8[c];
        cId[0x09] = colIdx9[c];
        cId[0x0a] = colIdxa[c];
        cId[0x0b] = colIdxb[c];
        cId[0x0c] = colIdxc[c];
        cId[0x0d] = colIdxd[c];
        cId[0x0e] = colIdxe[c];
        cId[0x0f] = colIdxf[c];
        for (int ch = 0; ch < MIS_numChannels; ch++) {
#pragma HLS UNROLL
            for (int pe = 0; pe < MIS_entries; pe++) {
#pragma HLS UNROLL
                int cAddr = cId[ch][pe];
                if (cAddr != -1) {
#ifndef __SYNTHESIS__
                    assert((cAddr % MIS_numChannels) == ch);
#endif
                    const uint16_t cprior = l_prior[ch][cAddr / MIS_numChannels];
                    const ap_uint<2> cst = (cprior >> 14) & 0x03;
                    const ap_uint<14> cp = cprior & 0x3fff;
                    l_mOut[ch][pe] = l_mOut[ch][pe] || (cst == 1);
                    l_undet[ch][pe] = l_undet[ch][pe] || (cst == 0 && rp < cp);
                }
            }
        }
    }
    for (int ch = 0; ch < MIS_numChannels; ch++) {
#pragma HLS UNROLL
        for (int pe = 0; pe < MIS_entries; pe++) {
#pragma HLS UNROLL
            move_out = move_out || l_mOut[ch][pe];
            undet = undet || l_undet[ch][pe];
        }
    }
}

extern "C" void misKernel(const int rows,
                          const int* rowPtr,
                          const ap_uint<MIS_entries * 32>* colIdx0,
                          const ap_uint<MIS_entries * 32>* colIdx1,
                          const ap_uint<MIS_entries * 32>* colIdx2,
                          const ap_uint<MIS_entries * 32>* colIdx3,
                          const ap_uint<MIS_entries * 32>* colIdx4,
                          const ap_uint<MIS_entries * 32>* colIdx5,
                          const ap_uint<MIS_entries * 32>* colIdx6,
                          const ap_uint<MIS_entries * 32>* colIdx7,
                          const ap_uint<MIS_entries * 32>* colIdx8,
                          const ap_uint<MIS_entries * 32>* colIdx9,
                          const ap_uint<MIS_entries * 32>* colIdxa,
                          const ap_uint<MIS_entries * 32>* colIdxb,
                          const ap_uint<MIS_entries * 32>* colIdxc,
                          const ap_uint<MIS_entries * 32>* colIdxd,
                          const ap_uint<MIS_entries * 32>* colIdxe,
                          const ap_uint<MIS_entries * 32>* colIdxf,
                          uint16_t* prior) {
    POINTER(rowPtr, gmem_rowPtr)
    POINTER(colIdx0, gmem_colIdx0)
    POINTER(colIdx1, gmem_colIdx1)
    POINTER(colIdx2, gmem_colIdx2)
    POINTER(colIdx3, gmem_colIdx3)
    POINTER(colIdx4, gmem_colIdx4)
    POINTER(colIdx5, gmem_colIdx5)
    POINTER(colIdx6, gmem_colIdx6)
    POINTER(colIdx7, gmem_colIdx7)
    POINTER(colIdx8, gmem_colIdx8)
    POINTER(colIdx9, gmem_colIdx9)
    POINTER(colIdxa, gmem_colIdxa)
    POINTER(colIdxb, gmem_colIdxb)
    POINTER(colIdxc, gmem_colIdxc)
    POINTER(colIdxd, gmem_colIdxd)
    POINTER(colIdxe, gmem_colIdxe)
    POINTER(colIdxf, gmem_colIdxf)
    POINTER(prior, gmem_prior)

    SCALAR(rows)
    SCALAR(return )

#ifndef __SYNTHESIS__
    assert(MIS_maxRows >= rows);
#endif

    uint16_t l_prior[MIS_numChannels][(MIS_maxRows + MIS_numChannels - 1) / MIS_numChannels];
#pragma HLS ARRAY_PARTITION variable = l_prior dim = 1 complete
#pragma HLS BIND_STORAGE variable = l_prior type = RAM_1P impl = URAM

    for (int row = 0; row < rows; row++)
#pragma HLS PIPELINE II = 1
        l_prior[row % MIS_numChannels][row / MIS_numChannels] = prior[row];

    bool update = true;
    while (update) {
        update = false;
        int first = 0;
        int last = 0;
        for (int row = 0; row < rows; row++) {
            const uint16_t rprior = l_prior[row % MIS_numChannels][row / MIS_numChannels];
            const ap_uint<14> rp = rprior & 0x3fff;
            ap_uint<2> rst = (rprior >> 14) & 0x03;

            first = last;
            last = rowPtr[row + 1] / MIS_numChannels / MIS_entries;
            if (rst == 0) {
                bool move_out = false;
                bool undet = false;
                checkColIdx(first, last, colIdx0, colIdx1, colIdx2, colIdx3, colIdx4, colIdx5, colIdx6, colIdx7,
                            colIdx8, colIdx9, colIdxa, colIdxb, colIdxc, colIdxd, colIdxe, colIdxf, l_prior, rp,
                            move_out, undet);
                if (move_out)
                    rst = 3;
                else if (undet == true) {
                    rst = 0;
                    update = true;
                } else
                    rst = 1;

                ap_uint<16> tmp = rp;
                tmp.range(15, 14) = rst;
                l_prior[row % MIS_numChannels][row / MIS_numChannels] = tmp;
            }
        }
    }
    for (int row = 0; row < rows; row++)
#pragma HLS PIPELINE II = 1
        prior[row] = l_prior[row % MIS_numChannels][row / MIS_numChannels];
}
