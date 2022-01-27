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

inline int64_t levenshtein_distance(string s1, string s2)
{
    int len_s1 = s1.length();
    int len_s2 = s2.length();
    int dist[len_s1+1][len_s2+1];

    int i;
    int j;
    int len_cost;

    for (i = 0;i <= len_s1; i++) {
        dist[i][0] = i;
    }
    for(j = 0; j<= len_s2; j++) {
        dist[0][j] = j;
    }
    for (i = 1;i <= len_s1;i++) {
        for (j = 1; j<= len_s2; j++) {
            if ( s1[i-1] == s2[j-1] ) {
                len_cost = 0;
            } else {
                len_cost = 1;
            }

            dist[i][j] = std::min(dist[i-1][j] + 1,                  // delete
                                  std::min(dist[i][j-1] + 1,         // insert
                                  dist[i-1][j-1] + len_cost)           // substitution
                                 );
            if ((i > 1) && (j > 1) && (s1[i-1] == s2[j-2]) && (s1[i-2] == s2[j-1])) {
                dist[i][j] = std::min(dist[i][j],
                                      dist[i-2][j-2] + len_cost   // transposition
                                     );
            }
        }
    }
    return dist[len_s1][len_s2];
}

inline int udf_fuzzymatch_tg(ListAccum<string> sourceList, ListAccum<string> targetList) 
{

    std::cout << "INFO: udf_fuzzymatch_tg sourceList size=" << sourceList.size() 
              << " targetList size=" << targetList.size() << std::endl;

    int execTime;
    int threshold = 90;
    string srcString, targetString;
    int editDistance;

    uint32_t sourceListLen = sourceList.size();
    uint32_t targetListLen = targetList.size();
    udf_reset_timer(true);
    for (unsigned i = 0; i < sourceListLen; ++i) {
        srcString = sourceList.get(i);
        for (unsigned j = 0; j < targetListLen; ++j) {
            targetString = targetList.get(j);
            editDistance = levenshtein_distance(srcString, targetString);
            std::cout << "    " << i << "," << j << " ed=" << editDistance << std::endl;
        }
    }

    execTime = udf_elapsed_time(true);

    return execTime;
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
