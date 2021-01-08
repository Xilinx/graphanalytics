/*
 * Copyright 2020-2021 Xilinx, Inc.
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
#ifndef __XF_GRAPH_TYPES_HPP_
#define __XF_GRAPH_TYPES_HPP_

#include <ap_int.h>

template <typename I, typename F>
inline F bitsToFloat(I in) {
    union {
        I __I;
        F __F;
    } __T;
    __T.__I = in;
    return __T.__F;
}

template <typename F, typename I>
inline I floatToBits(F in) {
    union {
        I __I;
        F __F;
    } __T;
    __T.__F = in;
    return __T.__I;
}

template <typename T>
struct isFloat {
    operator bool() { return false; }
};

template <>
struct isFloat<float> {
    operator bool() { return true; }
};

#endif
