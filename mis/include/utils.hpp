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
#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cmath>
#include <exception>

typedef std::chrono::time_point<std::chrono::high_resolution_clock> TimePointType;

inline void showTimeData(std::string p_Task, TimePointType& t1, TimePointType& t2, double* p_TimeMsOut = 0) {
    t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = t2 - t1;
    double l_timeMs = l_durationSec.count() * 1e3;
    if (p_TimeMsOut) {
        *p_TimeMsOut = l_timeMs;
    }
    std::cout << p_Task << "  " << std::fixed << std::setprecision(6) << l_timeMs << " msec\n";
}

inline void timeStamp(std::string p_Task = "") {
    static int count = 0;
    static auto TIME_ZERO = std::chrono::high_resolution_clock::now();
    auto t0 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = t0 - TIME_ZERO;
    double l_timeMs = l_durationSec.count() * 1e3;
    if(p_Task == "")
        std::cout << "Time stamp " << count++ << " @ " << std::fixed << std::setprecision(6) << l_timeMs << " msec\n";
    else
        std::cout << p_Task << " @ " << std::fixed << std::setprecision(6) << l_timeMs << " msec\n";
}

template <typename T>
struct alignedAllocator {
    using value_type = T;
    T* allocate(std::size_t num) {
        void* ptr = nullptr;
        if (posix_memalign(&ptr, 4096, num * sizeof(T))) throw std::bad_alloc();
        return reinterpret_cast<T*>(ptr);
    }
    void deallocate(T* p, std::size_t num) { free(p); }
};


inline bool isLessEqual(uint32_t x, uint32_t y) {
    return x <= y;
}
// provide same functionality as numpy.isclose
template <typename T>
bool isClose(float p_tolRel, float p_tolAbs, T p_vRef, T p_v, bool& p_exactMatch) {
    float l_diffAbs = std::abs(p_v - p_vRef);
    p_exactMatch = (p_vRef == p_v);
    bool l_status = (l_diffAbs <= (p_tolAbs + p_tolRel * std::abs(p_vRef)));
    return (l_status);
}
template <typename T>
bool compare(T x, T ref) {
    return x == ref;
}

template<>
inline bool compare<double>(double x, double ref) {
    bool l_exactMatch;
    return isClose<float>(1e-4, 3e-6, x, ref, l_exactMatch);
}

template<>
inline bool compare<float>(float x, float ref) {
    bool l_exactMatch;
    return isClose<float>(1e-3, 3e-5, x, ref, l_exactMatch);
}

template <typename T>
bool compare(unsigned int n, T* x, T* ref) {
    bool l_ret = true;
    try {
        if (ref == nullptr) {
            if (x == nullptr) return true;
            for (unsigned int i = 0; i < n; i++) l_ret = l_ret && compare(x[i], (T)0);
        } else {
            for (unsigned int i = 0; i < n; i++) {
                l_ret = l_ret && compare(x[i], ref[i]);
            }
        }
    } catch (std::exception& e) {
        std::cout << "Exception happend: " << e.what() << std::endl;
        return false;
    }
    return l_ret;
}

template <typename T>
bool compare(unsigned int n, T* x, T* ref, int& err, bool verbose = false) {
    bool l_ret = true;
    try {
        if (ref == nullptr) {
            if (x == nullptr) return true;
            for (unsigned int i = 0; i < n; i++) {
                if (!compare(x[i], (T)0)) {
                    err++;
                    l_ret = false;
                }
            }
        } else {
            for (unsigned int i = 0; i < n; i++) {
                if (!compare(x[i], ref[i])) {
                    if (verbose) {
                        std::cout << "Mismatch at entry " << i << " outValue = " << x[i] << " refValue = " << ref[i]
                                  << std::endl;
                    }
                    l_ret = false;
                    err++;
                }
            }
        }
    } catch (std::exception& e) {
        std::cout << "Exception happend: " << e.what() << std::endl;
        return false;
    }
    return l_ret;
}
#endif
