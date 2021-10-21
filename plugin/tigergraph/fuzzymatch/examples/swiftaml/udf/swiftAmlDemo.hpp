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

#ifndef SWIFTAML_DEMO_HPP
#define SWIFTAML_DEMO_HPP

// mergeHeaders 1 name swiftAmlDemo

// mergeHeaders 1 section include start swiftAmlDemo DO NOT REMOVE!
#include "swiftAmlDemoImpl.hpp"
// mergeHeaders 1 section include end swiftAmlDemo DO NOT REMOVE!

//#####################################################################################################################

namespace UDIMPL {

// mergeHeaders 1 section body start swiftAmlDemo DO NOT REMOVE!

inline bool concat_uint64_to_str(string& ret_val, uint64_t val) {
    (ret_val += " ") += std::to_string(val);
    return true;
}

inline bool udf_reset_timer(bool dummy) {
    swiftAmlDemo::getTimerStartTime() = std::chrono::high_resolution_clock::now();
    return true;
}

inline double udf_elapsed_time(bool dummy) {
    swiftAmlDemo::t_time_point cur_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = cur_time - swiftAmlDemo::getTimerStartTime();
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
            VmPeak = swiftAmlDemo::extract_size_in_kB(line);
        }
        pos = line.find("VmHWM");
        if( pos != string::npos) {
            VmHWM = swiftAmlDemo::extract_size_in_kB(line);
        }
    }
    return 0L;
}

inline bool udf_fuzzymatch_cpu(ListAccum<string> blacklist, ListAccum<string> txPersons) 
{

    std::cout << "INFO: udf_fuzzymatch_cpu " << std::endl;

    std::vector<std::string> blacklistVector;
    xilFuzzyMatch::Context *pContext = xilFuzzyMatch::Context::getInstance();
    xilinx_apps::fuzzymatch::FuzzyMatch *pFuzzyMatch = pContext->getFuzzyMatchObj();
    
    uint32_t blacklistLen = blacklist.size();
    for (unsigned i = 0 ; i < blacklistLen; ++i)
        blacklistVector.push_back(blacklist.get(i));

    std::cout << "blacklistVector size=" << blacklistVector.size() << std::endl;

    uint32_t txPersonsLen = txPersons.size();
    for (unsigned i = 0 ; i < txPersonsLen; ++i)
        std::cout << "txPersons " << i << "=" << txPersons.get(i) << std::endl;

    return true;
}

// mergeHeaders 1 section body end swiftAmlDemo DO NOT REMOVE!

}  // namespace UDIMPL
#endif
