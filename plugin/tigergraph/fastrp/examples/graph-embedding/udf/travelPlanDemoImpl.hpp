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

#ifndef GRAPHEMBEDDING_DEMO_IMPL_HPP
#define GRAPHEMBEDDING_DEMO_IMPL_HPP

#include <chrono>

namespace graphEmbeddingDemo {

typedef std::chrono::time_point<std::chrono::high_resolution_clock> t_time_point, *pt_time_point; 
//extern t_time_point timer_start_time;

inline t_time_point &getTimerStartTime() {
    static t_time_point s_startTime;
    return s_startTime;
}

//Layout of fields in /proc/self/status
//VmPeak:     8216 kB This is peak Virtual Memory size
//VmHWM:       752 kB This is peak Resident Set Size
// return size of field in kB, return value of -1.0 means error
inline double extract_size_in_kB(std::string& line) {
    double retValue = -1.0;
    std::istringstream sstream(line);
    std::string field, value, unit;
    sstream >> field >> value >> unit;
    if (unit == "kB") {
        retValue = std::stod(value);
    }
    return retValue;
}

} // namespace

#endif /* GRAPHEMBEDDING_DEMO_IMPL_HPP */

