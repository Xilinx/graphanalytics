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
#ifndef _WIDETYPE_H_
#define _WIDETYPE_H_

#include <type_traits>
#ifndef __SYNTHESIS__
#include <iostream>
#endif

#include "ap_int.h"
#include "hls_stream.h"
#include "ap_axi_sdata.h"

template <typename T, int t_Width, int t_DataWidth = sizeof(T) * 8, typename Enable = void>
class WideType {
   private:
    T m_Val[t_Width];
    static const unsigned int FLOAT_WIDTH = 7;

   public:
    static const int t_TypeWidth = t_Width * t_DataWidth;
    typedef ap_uint<t_TypeWidth> t_TypeInt;
    typedef ap_axis<t_TypeWidth, 0, 0, 0> t_TypeAxi;
    typedef T DataType;
    static const int t_WidthS = t_Width;

   public:
    T& getVal(unsigned int i) {
#pragma HLS INLINE
#ifndef __SYNTHESIS__
        assert(i < t_Width);
#endif
        return (m_Val[i]);
    }
    T& operator[](unsigned int p_Idx) {
#pragma HLS INLINE
#ifndef __SYNTHESIS__
        assert(p_Idx < t_Width);
#endif
        return (m_Val[p_Idx]);
    }
    const T& operator[](unsigned int p_Idx) const {
#pragma HLS INLINE
#ifndef __SYNTHESIS__
        assert(p_Idx < t_Width);
#endif
        return (m_Val[p_Idx]);
    }
    T* getValAddr() {
#pragma HLS INLINE
        return (&m_Val[0]);
    }

    WideType() {
#pragma HLS INLINE
#pragma HLS ARRAY_PARTITION variable = m_Val complete dim = 1
    }

    WideType(const WideType& wt) {
#pragma HLS INLINE
#pragma HLS ARRAY_PARTITION variable = m_Val complete dim = 1
        constructor(wt);
    }

    void constructor(const WideType& wt) {
#pragma HLS INLINE
        for (unsigned int i = 0; i < t_Width; i++)
#pragma HLS UNROLL
            m_Val[i] = wt[i];
    }

    WideType(const t_TypeInt& p_val) {
#pragma HLS INLINE
#pragma HLS ARRAY_PARTITION variable = m_Val complete dim = 1
        constructor(p_val);
    }
    void constructor(const t_TypeInt& p_val) {
#pragma HLS INLINE
        for (int i = 0; i < t_Width; ++i) {
#pragma HLS UNROLL
            ap_uint<t_DataWidth> l_val = p_val.range(t_DataWidth * (1 + i) - 1, t_DataWidth * i);
            m_Val[i] = *reinterpret_cast<T*>(&l_val);
        }
    }

    WideType(const T p_initScalar) {
#pragma HLS INLINE
#pragma HLS ARRAY_PARTITION variable = m_Val complete dim = 1
        constructor(p_initScalar);
    }
    void constructor(const T p_initScalar) {
#pragma HLS INLINE
        for (int i = 0; i < t_Width; ++i) {
#pragma HLS UNROLL
            m_Val[i] = p_initScalar;
        }
    }

    WideType(const t_TypeAxi& val) {
#pragma HLS INLINE
#pragma HLS ARRAY_PARTITION variable = m_Val complete dim = 1
        const t_TypeInt v = val.data;
        constructor(v);
    }

    operator const t_TypeAxi() {
        t_TypeAxi val;
        val.data = this->t_TypeInt();
        return val;
    }

    operator const t_TypeInt() {
        t_TypeInt l_fVal;
        for (int i = 0; i < t_Width; ++i) {
#pragma HLS UNROLL
            T l_v = m_Val[i];
            ap_uint<t_DataWidth> l_val = *reinterpret_cast<ap_uint<t_DataWidth>*>(&l_v);
            l_fVal.range(t_DataWidth * (1 + i) - 1, t_DataWidth * i) = l_val;
        }
        return l_fVal;
    }

    bool operator==(const WideType& p_w) const {
        bool l_com = true;
        for (int i = 0; i < t_Width; ++i) {
            l_com = l_com && (m_Val[i] == p_w[i]);
        }
        return l_com;
    }

    T shift(T p_ValIn) {
#pragma HLS INLINE
        T l_valOut = m_Val[t_Width - 1];
    WIDE_TYPE_SHIFT:
        for (int i = t_Width - 1; i > 0; --i) {
#pragma HLS UNROLL
            T l_val = m_Val[i - 1];
            m_Val[i] = l_val;
        }
        m_Val[0] = p_ValIn;
        return (l_valOut);
    }

    T shift() {
#pragma HLS INLINE
        T l_valOut = m_Val[t_Width - 1];
        for (int i = t_Width - 1; i > 0; --i) {
#pragma HLS UNROLL
            T l_val = m_Val[i - 1];
            m_Val[i] = l_val;
        }
        return (l_valOut);
    }

    T unshift() {
#pragma HLS INLINE
        T l_valOut = m_Val[0];
        for (int i = 0; i < t_Width - 1; ++i) {
#pragma HLS UNROLL
            T l_val = m_Val[i + 1];
            m_Val[i] = l_val;
        }
        return (l_valOut);
    }

    T unshift(const T p_val) {
#pragma HLS INLINE
        T l_valOut = m_Val[0];
        for (int i = 0; i < t_Width - 1; ++i) {
#pragma HLS UNROLL
            T l_val = m_Val[i + 1];
            m_Val[i] = l_val;
        }
        m_Val[t_Width - 1] = p_val;
        return (l_valOut);
    }

    static const WideType zero() {
        WideType l_zero;
        for (int i = 0; i < t_Width; ++i) {
#pragma HLS UNROLL
            l_zero[i] = 0;
        }
        return (l_zero);
    }

#ifndef __SYNTHESIS__
    friend std::ostream& operator<<(std::ostream& os, WideType& p_Val) {
        for (int i = 0; i < t_Width; ++i) {
            os << std::setw(FLOAT_WIDTH) << p_Val.m_Val[i] << " ";
        }
        return (os);
    }
#endif
};

template <typename T, int t_DataWidth>
class WideType<T, 1, t_DataWidth, typename std::enable_if<std::is_same<ap_uint<t_DataWidth>, T>::value>::type> {
   private:
    T m_Val;
    static const unsigned int FLOAT_WIDTH = 7;

   public:
    static const int t_TypeWidth = t_DataWidth;
    typedef ap_uint<t_DataWidth> t_TypeInt;
    typedef T DataType;
    static const int t_WidthS = 1;

   public:
    T& operator[](unsigned int p_Idx) {
#pragma HLS INLINE
#ifndef __SYNTHESIS__
        assert(p_Idx == 0);
#endif
        return m_Val;
    }

    const T& operator[](unsigned int p_Idx) const {
#pragma HLS INLINE
#ifndef __SYNTHESIS__
        assert(p_Idx == 0);
#endif
        return m_Val;
    }

    T* getValAddr() {
#pragma HLS INLINE
        return (&m_Val);
    }

    WideType() {}

    WideType(const WideType& wt) { m_Val = wt[0]; }

    WideType(const t_TypeInt p_val) { m_Val = p_val; }

    operator const t_TypeInt() { return m_Val; }

    bool operator==(const WideType& p_w) const { return m_Val == p_w[0]; }

    T shift(T p_ValIn) {
        T l_valOut = m_Val;
        m_Val = p_ValIn;
        return l_valOut;
    }
    T shift() { return m_Val; }

    T unshift() { return m_Val; }

    T unshift(T p_ValIn) {
        T l_valOut = m_Val;
        m_Val = p_ValIn;
        return l_valOut;
    }

    static const WideType zero() { return WideType(0); }

#ifndef __SYNTHESIS__
    friend std::ostream& operator<<(std::ostream& os, WideType& p_Val) {
        os << std::setw(FLOAT_WIDTH) << p_Val.m_Val << " ";
        return (os);
    }
#endif
};

#endif
