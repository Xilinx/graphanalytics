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

#ifndef TRAVELPLAN_DEMO_HPP
#define TRAVELPLAN_DEMO_HPP

// mergeHeaders 1 name travelPlanDemo

// mergeHeaders 1 section include start travelPlanDemo DO NOT REMOVE!
#include "travelPlanDemoImpl.hpp"
// mergeHeaders 1 section include end travelPlanDemo DO NOT REMOVE!

//#####################################################################################################################

namespace UDIMPL {

// mergeHeaders 1 section body start travelPlanDemo DO NOT REMOVE!

inline bool concat_uint64_to_str(string& ret_val, uint64_t val) {
    (ret_val += " ") += std::to_string(val);
    return true;
}

inline bool udf_reset_timer(bool dummy) {
    travelPlanDemo::getTimerStartTime() = std::chrono::high_resolution_clock::now();
    return true;
}

inline double udf_elapsed_time(bool dummy) {
    travelPlanDemo::t_time_point cur_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = cur_time - travelPlanDemo::getTimerStartTime();
    double l_timeMs = l_durationSec.count() * 1e3;
    return l_timeMs;
}

inline int64_t udf_peak_memory_usage(double& VmPeak, double& VmHWM)
{
    // Open the /proc/self/status and grep for relevant fields
    //Layout of fields in /proc/self/status
    //VmPeak:     8216 kB This is peak Virtual Memory size
    //VmHWM:       752 kB This is peak Resident Set Size
    uint64_t vm_peak, vm_hwm;
    string line;
    std::ifstream proc_status("/proc/self/status", std::ios_base::in);
    while (std::getline(proc_status, line)) {
        std::size_t pos = line.find("VmPeak");
        if( pos != string::npos) {
            VmPeak = travelPlanDemo::extract_size_in_kB(line);
        }
        pos = line.find("VmHWM");
        if( pos != string::npos) {
            VmHWM = travelPlanDemo::extract_size_in_kB(line);
        }
    }
    return 0L;
}

// mergeHeaders 1 section body end travelPlanDemo DO NOT REMOVE!

}  // namespace UDIMPL
#endif
