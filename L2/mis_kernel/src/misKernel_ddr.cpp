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
                 const ap_uint<MIS_numChannels * 32>* colIdx,
                 const uint16_t l_prior[MIS_numChannels][MIS_maxRows / MIS_numChannels],
                 const ap_uint<14> rp,
                 bool& move_out,
                 bool& undet) {
    bool l_mOut[MIS_numChannels];
#pragma HLS ARRAY_PARTITION variable=l_mOut complete dim=0
    bool l_undet[MIS_numChannels];
#pragma HLS ARRAY_PARTITION variable=l_undet complete dim=0

    for (int c = 0; c < MIS_numChannels; c++) {
#pragma HLS UNROLL
        l_mOut[c] = false;
        l_undet[c] = false;
    }

    for (int c = cStart; c < cEnd; c++) {
#pragma HLS PIPELINE II = 1
        WideType<int, MIS_numChannels> cId = colIdx[c];
        for (int ch = 0; ch < MIS_numChannels; ch++) {
#pragma HLS UNROLL
            int cAddr = cId[ch];
            if (cAddr != -1) {
#ifndef __SYNTHESIS__
                assert((cAddr % MIS_numChannels) == ch);
#endif
                const uint16_t cprior = l_prior[ch][cAddr / MIS_numChannels];
                const ap_uint<2> cst = (cprior >> 14) & 0x03;
                const ap_uint<14> cp = cprior & 0x3fff;
                l_mOut[ch] = l_mOut[ch] || (cst == 1);
                l_undet[ch] = l_undet[ch] || (cst == 0 && rp < cp);
            }
        }
    }
    for (int ch = 0; ch < MIS_numChannels; ch++) {
#pragma HLS UNROLL
        move_out = move_out || l_mOut[ch];
        undet = undet || l_undet[ch];
    }
}

extern "C" void misKernel(const int rows,
        const int* rowPtr,
        const ap_uint<MIS_numChannels * 32>* colIdx,
        uint16_t* prior) {
    POINTER(rowPtr, gmem_rowPtr)
        POINTER(colIdx, gmem_colIdx)
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
                checkColIdx(first, last, colIdx, l_prior, rp, move_out, undet);
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


