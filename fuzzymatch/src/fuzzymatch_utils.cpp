/**
* Copyright (C) 2021 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/

#include <cassert>
#include <iostream>
#include <sstream>
#include <limits>

#include "fuzzymatch.hpp"

/**
 * Define this macro to make functions in fuzzymatch_loader.cpp inline instead of extern.  You would use this macro
 * when including fuzzymatch_loader.cpp in a header file, as opposed to linking with libXilinxFuzzyMatch_loader.a.
 */
#ifdef XILINX_FUZZYMATCH_INLINE_IMPL
#define XILINX_FUZZYMATCH_IMPL_DECL inline
#else
#define XILINX_FUZZYMATCH_IMPL_DECL extern
#endif

namespace xilinx_apps {
namespace fuzzymatch {
    // ------------------------------------------------------------------------
    // Utility functions 
    // ------------------------------------------------------------------------
   
    // generate the pattern by group 
    XILINX_FUZZYMATCH_IMPL_DECL
    void preSortbyLength(std::vector<std::string>& vec_pattern,
                         std::vector<std::vector<std::string>>& vec_pattern_grp)
    {
        for (std::vector<std::string>::iterator it = vec_pattern.begin(); it != vec_pattern.end(); ++it) {
            size_t len = it->length();
            assert(len < max_pattern_len_in_char && "Defined <max_pattern_len_in_char> is not enough!");
            vec_pattern_grp[len].push_back(*it);
        }
    }

    XILINX_FUZZYMATCH_IMPL_DECL
    int load_csv(const size_t max_entry_num, const size_t max_field_len, const std::string &file_path,
                    const unsigned col, std::vector<std::string> &vec)
    {
        std::cout << "INFO: Loading " << max_entry_num << " entities from CSV file " << file_path << std::endl;
    
        std::ifstream file(file_path.c_str());
        if (!file.is_open())
        {
            std::cerr << "\nError:" << file_path << " file cannot be opened for reading!" << std::endl;
            return -1;
        }
    
        size_t n = 0;
        std::string line;
        // skip the header
        std::getline(file, line);
        while (std::getline(file, line))
        {
            unsigned cur_col = 0;
            bool in_escape_str = false;
            std::stringstream ins(line), outs;
            char c;
            while (ins.get(c))
            {
                if (c == '"')
                {
                    if (ins.peek() == '"')
                    {
                        // escaped, not state change, just unescape
                        ins.get(c);
                        outs.put(c);
                    }
                    else
                    {
                        // toggle state
                        in_escape_str = !in_escape_str;
                    }
                }
                else if (c == ',' && !in_escape_str)
                {
                    if (cur_col == col)
                    {
                        // already got our column
                        break;
                    }
                    else
                    {
                        // empty buffer and continue to next column
                        cur_col++;
                        //outs = std::stringstream();
                        outs.str("");
                    }
                }
                else if (c == '\r')
                {
                    // in-case we are working with MS new line...
                }
                else
                {
                    outs.put(c);
                }
            }
            assert(cur_col == col && "not enough column!");
            std::string data = outs.str();
            //std::cout << data << std::endl;
            if (data.length() <= max_field_len)
            {
                vec.push_back(data);
                // std::cerr << line << "\n\t{" << data << "}" << std::endl;
                n++;
            }
            if (n > max_entry_num)
            {
                std::cout << "\nWarning: the input file " << file_path << " contains more enties than " << max_entry_num
                            << " which will not be added in this check." << std::endl;
                file.close();
                return 0;
            }
        }
        file.close();
        return 0;
    }

    XILINX_FUZZYMATCH_IMPL_DECL
    int min(int a, int b)
    {
        return (a < b ? a : b);
    }

    XILINX_FUZZYMATCH_IMPL_DECL
    int max(int a, int b) {
        return (a < b ? b : a);
    }

    XILINX_FUZZYMATCH_IMPL_DECL
    int abs(int a, int b)
    {
        return (a < b ? (b - a) : (a - b));
    }
/*
    XILINX_FUZZYMATCH_IMPL_DECL
    float similarity(std::string str1, std::string str2);
*/
    XILINX_FUZZYMATCH_IMPL_DECL
    int similarity(int threshold, std::string str1, std::string str2);

    XILINX_FUZZYMATCH_IMPL_DECL
    size_t getMaxDistance(size_t len)
    {
        return (len / 10);
    }

    XILINX_FUZZYMATCH_IMPL_DECL
    bool isGreater(std::pair<int, int> a, std::pair<int, int> b) {
        return a.second > b.second;
    }

    XILINX_FUZZYMATCH_IMPL_DECL
    void doFuzzyTask(const int similarity_level,
                    const size_t upper_limit,
                    const std::string& pattern,
                    const std::vector<std::vector<std::string> >& vec_grp_str,
                    const std::vector<std::vector<int> >& vec_id_people,
                    std::unordered_map<int, int>& result_map) {
        result_map.clear();
        std::vector<std::pair<int, int> > result_vec;
        size_t len = pattern.length();
        size_t med = len * (100 - similarity_level) / 100;
        // size_t start_len = (len > (upper_limit - 3) && len <= upper_limit) ? (upper_limit + 1) : (len -
        // med);
        size_t start_len = len - med;
        size_t end_len = len + med;

        for (size_t n = start_len; n <= end_len; n++) {
            std::vector<std::string> deny_list = vec_grp_str[n];
            std::vector<int> id_list = vec_id_people[n];
            for (unsigned int i = 0; i < deny_list.size(); i++) {
                int sim = similarity(similarity_level, pattern, deny_list.at(i));
                if (sim >= similarity_level) {
                    result_vec.push_back(std::make_pair(id_list.at(i), sim));
                }
            }
        }

        std::sort(result_vec.begin(), result_vec.end(), &isGreater);
        // if (result_vec.size() > 100) result_vec.resize(100);
        std::for_each(result_vec.begin(), result_vec.end(), [&](const std::pair<int, int> t) {
            result_map.insert({t.first, t.second});
        });
    }
/*
    XILINX_FUZZYMATCH_IMPL_DECL
    bool doFuzzyTask(int thread_id,
                                            const size_t upper_limit,
                                            const std::string &pattern,
                                            const std::vector<std::vector<std::string>> &vec_grp_str)
    {
        bool match = false;
        size_t len = pattern.length();
        size_t med = getMaxDistance(len);
        size_t start_len = (len > (upper_limit - 3) && len <= upper_limit) ? (upper_limit + 1) : (len - med);
        size_t end_len = len + med;
    
        for (size_t n = start_len; n <= end_len; n++)
        {
            std::vector<std::string> deny_list = vec_grp_str[n];
            int step = (deny_list.size() + totalThreadNum - 1) / totalThreadNum;
            int size = size_t(thread_id * step + step) > deny_list.size() ? (deny_list.size() - thread_id * step) : step;
            for (int i = thread_id * step; i < (thread_id * step + size); i++)
            {
                float sim = similarity(pattern, deny_list.at(i));
                if (sim >= 0.9)
                {
                    match = true;
                    break;
                }
            }
    
            if (match)
                break;
        }
    
        return match;
    }*/
    XILINX_FUZZYMATCH_IMPL_DECL
    int similarity(int threshold, std::string str1, std::string str2) {
        const int n = str1.length();
        const int m = str2.length();
        if (n == 0 || m == 0) return 0;

        int maxDistance = (int)(max(n, m) * (100 - threshold) / 100);

        if (maxDistance < abs(m, n)) return 0;

        std::vector<int> p(n + 1, 0);
        std::vector<int> d(n + 1, 0);

        for (int i = 0; i <= n; i++) p[i] = i;

        for (int j = 1; j <= m; j++) {
            int bestPossibleEditDistance = m;
            char t_j = str2.at(j - 1);
            d[0] = j;

            for (int i = 1; i <= n; i++) {
                if (t_j != str1.at(i - 1))
                    d[i] = min(min(d[i - 1], p[i]), p[i - 1]) + 1;
                else
                    d[i] = min(min(d[i - 1] + 1, p[i] + 1), p[i - 1]);
                bestPossibleEditDistance = min(bestPossibleEditDistance, d[i]);
            }

            if (j > maxDistance && bestPossibleEditDistance > maxDistance) return 0;

            std::swap_ranges(p.begin(), p.end(), d.begin());
        }

        return (100 - (100 * p[n] / max(m, n)));
    }
/*
    XILINX_FUZZYMATCH_IMPL_DECL
    float similarity(std::string str1, std::string str2)
    {
        const int n = str1.length();
        const int m = str2.length();
        if (n == 0 || m == 0)
            return 0.0;
    
        int maxDistance = (int)(0.1 * min(n, m));
    
        if (maxDistance < abs(m, n))
            return 0.0;
    
        std::vector<int> p(n + 1, 0);
        std::vector<int> d(n + 1, 0);
    
        for (int i = 0; i <= n; i++)
            p[i] = i;
    
        for (int j = 1; j <= m; j++)
        {
            int bestPossibleEditDistance = m;
            char t_j = str2.at(j - 1);
            d[0] = j;
    
            for (int i = 1; i <= n; i++)
            {
                if (t_j != str1.at(i - 1))
                    d[i] = min(min(d[i - 1], p[i]), p[i - 1]) + 1;
                else
                    d[i] = min(min(d[i - 1] + 1, p[i] + 1), p[i - 1]);
                bestPossibleEditDistance = min(bestPossibleEditDistance, d[i]);
            }
    
            if (j > maxDistance && bestPossibleEditDistance > maxDistance)
                return 0.0;
    
            std::swap_ranges(p.begin(), p.end(), d.begin());
        }
    
        return (1.0 - ((float)p[n] / (float)min(m, n)));
    }*/
/*
    XILINX_FUZZYMATCH_IMPL_DECL
    bool strFuzzy(const size_t upper_limit, const std::string &pattern,
                  std::vector<std::vector<std::string>> &vec_grp_str)
    {
        std::future<bool> worker[100];
        for (unsigned i = 0; i < totalThreadNum; i++) {
            worker[i] = std::async(std::launch::async, &doFuzzyTask, i, upper_limit,
                        std::ref(pattern), std::ref(vec_grp_str));
        }
        bool sw_match = false;
        for (unsigned i = 0; i < totalThreadNum; i++)
            sw_match = sw_match || worker[i].get();
        return sw_match;
    }*/

    XILINX_FUZZYMATCH_IMPL_DECL   
    int FuzzyMatchSW::initialize(const std::string &fileName)
    {
        std::vector<std::string> vec_pattern;
        // Read Watch List data
        int nerror = 0;
        //std::cout << "INFO: Loading "  << fileName << std::endl;
        nerror = load_csv(max_validated_pattern, -1U, fileName, 1, vec_pattern);
        if (nerror)
        {
            std::cout << "ERROR: Failed to load file " << fileName << std::endl;
            exit(1);
        }
        else
            std::cout << "INFO: completed loading " << fileName << std::endl;
    
        // do pre-sort on pattern LIST
        //preSortbyLength(vec_pattern,this->vec_pattern_grp);
        initialize(vec_pattern);
        return nerror;
    }
/*
    XILINX_FUZZYMATCH_IMPL_DECL   
    int FuzzyMatchSW::initialize(std::vector<std::string>& vec_pattern)
    {
        preSortbyLength(vec_pattern,this->vec_pattern_grp);  
        return 0;
    }    
*/
    XILINX_FUZZYMATCH_IMPL_DECL   
    int FuzzyMatchSW::initialize(std::vector<std::string>& vec_pattern, std::vector<int> vec_id)
    {
        if(!vec_id.empty()){
            for (int idx=0; idx < vec_pattern.size(); idx++) {
                size_t len = vec_pattern[idx].length();
                assert(len < max_pattern_len_in_char && "Defined <max_people_len_in_char> is not enough!");
                vec_pattern_id[len].push_back(vec_id[idx]);
            }
        } else {
            //default assignment
            int id_cnt = 0;
            for (std::vector<std::string>::iterator it = vec_pattern.begin(); it != vec_pattern.end(); ++it) {
                size_t len = it->length();
                assert(len < max_pattern_len_in_char && "Defined <max_people_len_in_char> is not enough!");
                vec_pattern_id[len].push_back(++id_cnt);
            }
        }
        

        preSortbyLength(vec_pattern,this->vec_pattern_grp);  
        return 0;
    }       
    XILINX_FUZZYMATCH_IMPL_DECL   
    std::unordered_map<int,int> FuzzyMatchSW::check(int threshold, const std::string &ptn_string)
    {
        //bool r = strFuzzy(this->max_fuzzy_len, t, vec_pattern_grp);
        std::unordered_map<int, int> result_map;
        doFuzzyTask(threshold, this->max_fuzzy_len, ptn_string, vec_pattern_grp, vec_pattern_id, result_map);
        return result_map;
    }
    
} // namespace fuzzymatch
} // namespace xilinx_apps

