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

#ifndef ENTITY_RESOLUTION_HPP
#define ENTITY_RESOLUTION_HPP

// mergeHeaders 1 name entityResolution

// mergeHeaders 1 section include start entityResolution DO NOT REMOVE!
#include "entityResolutionImpl.hpp"
// mergeHeaders 1 section include end entityResolution DO NOT REMOVE!

//#####################################################################################################################

namespace UDIMPL {

// mergeHeaders 1 section body start entityResolution DO NOT REMOVE!

inline bool concat_uint64_to_str(string& ret_val, uint64_t val) {
    (ret_val += " ") += std::to_string(val);
    return true;
}

inline bool udf_reset_timer(bool dummy) {
    entityResolution::getTimerStartTime() = std::chrono::high_resolution_clock::now();
    return true;
}

inline double udf_elapsed_time(bool dummy) {
    entityResolution::t_time_point cur_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = cur_time - entityResolution::getTimerStartTime();
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
            VmPeak = entityResolution::extract_size_in_kB(line);
        }
        pos = line.find("VmHWM");
        if( pos != string::npos) {
            VmHWM = entityResolution::extract_size_in_kB(line);
        }
    }
    return 0L;
}

inline int udf_fuzzymatch_cpu(ListAccum<string> sourceList, ListAccum<string> targetList) 
{

    std::cout << "INFO: udf_fuzzymatch_cpu sourceList size=" << sourceList.size() 
              << " targetList size=" << targetList.size() << std::endl;

    std::vector<std::string> sourceVector;
    xilinx_apps::fuzzymatch::FuzzyMatchSW cpuChecker;
    std::unordered_map<int,int> swresult;
    int execTime;
    int threshold = 90;

    uint32_t sourceListLen = sourceList.size();
    for (unsigned i = 0 ; i < sourceListLen; ++i)
        sourceVector.push_back(sourceList.get(i));

    std::cout << "sourceVector size=" << sourceVector.size() << std::endl;
    cpuChecker.initialize(sourceVector);

    // only measure match time
    udf_reset_timer(true);
    uint32_t targetListLen = targetList.size();
    for (unsigned i = 0 ; i < targetListLen; ++i) {
        //std::cout << "targetList " << i << "=" << targetList.get(i) << std::endl;
        swresult = cpuChecker.check(threshold, targetList.get(i));
        //std::cout << i << "," << targetList.get(i) << "," << (check_result ? "KO" : "OK") << ","
        //          << (check_result ? ":Sender" : "") << std::endl;
    }
    execTime = udf_elapsed_time(true);

    return execTime;
}

// mergeHeaders 1 section body end entityResolution DO NOT REMOVE!

}  // namespace UDIMPL
#endif
