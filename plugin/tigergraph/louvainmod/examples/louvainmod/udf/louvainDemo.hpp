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
 * WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LOUVAIN_DEMO_HPP
#define LOUVAIN_DEMO_HPP

// mergeHeaders 1 name louvainDemo

// mergeHeaders 1 section include start louvainDemo DO NOT REMOVE!
#include "louvainDemoImpl.hpp"
// mergeHeaders 1 section include end louvainDemo DO NOT REMOVE!

//#####################################################################################################################

namespace UDIMPL {

// mergeHeaders 1 section body start louvainDemo DO NOT REMOVE!

inline bool concat_uint64_to_str(string& ret_val, uint64_t val) {
    (ret_val += " ") += std::to_string(val);
    return true;
}

inline bool udf_reset_timer(bool dummy) {
    louvainDemo::getTimerStartTime() = std::chrono::high_resolution_clock::now();
    return true;
}

inline double udf_elapsed_time(bool dummy) {
    louvainDemo::t_time_point cur_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = cur_time - louvainDemo::getTimerStartTime();
    double l_timeMs = l_durationSec.count() * 1e3;
    return l_timeMs;
}

template <typename tuple>
inline uint64_t getDeltaQ (tuple tup) {
  return tup.deltaQ;
}

template<typename tup>
inline int64_t getOutDegree(tup t) {
  return t.OutDgr;
}

template<typename tup>
inline VERTEX getCc(tup t) {
  return t.cc;
}

// mergeHeaders 1 section body end louvainDemo DO NOT REMOVE!

}  // namespace UDIMPL
#endif
