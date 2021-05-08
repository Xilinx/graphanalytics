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

#ifndef LOUVAIN_DEMO_IMPL_HPP
#define LOUVAIN_DEMO_IMPL_HPP

#include <chrono>

namespace louvainDemo {

using Mutex = xilComDetect::Mutex;
using Lock = xilComDetect::Lock;

typedef std::chrono::time_point<std::chrono::high_resolution_clock> t_time_point, *pt_time_point; 
//extern t_time_point timer_start_time;

inline t_time_point &getTimerStartTime() {
    static t_time_point s_startTime;
    return s_startTime;
}


} // namespace

#endif /* LOUVAIN_DEMO_IMPL_HPP */

