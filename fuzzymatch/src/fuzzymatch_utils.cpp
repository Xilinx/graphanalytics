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
    int abs(int a, int b)
    {
        return (a < b ? (b - a) : (a - b));
    }

    XILINX_FUZZYMATCH_IMPL_DECL
    float similarity(std::string str1, std::string str2);

    XILINX_FUZZYMATCH_IMPL_DECL
    size_t getMaxDistance(size_t len)
    {
        return (len / 10);
    }

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
    }

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
    }

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
    }

    XILINX_FUZZYMATCH_IMPL_DECL   
    int FuzzyMatchSW::initialize(const std::string &fileName)
    {
        std::vector<std::string> vec_pattern;
        // Read Watch List data
        int nerror = 0;
        std::cout << "Loading people.csv..." << std::flush;
        nerror = load_csv(max_validated_pattern, -1U, fileName, 1, vec_pattern);
        if (nerror)
        {
            std::cout << "Failed to load file: people.csv\n";
            exit(1);
        }
        else
            std::cout << "completed\n";
    
        // do pre-sort on pattern LIST
        preSortbyLength(vec_pattern, this->vec_pattern_grp);
    
        return nerror;
    }
    
    XILINX_FUZZYMATCH_IMPL_DECL   
    int FuzzyMatchSW::initialize(std::vector<std::string>& vec_pattern)
    {
        std::cout << "DEBUG: FuzzyMatchSW::initialize vec_pattern size=" << vec_pattern.size() << std::endl;
        // do pre-sort on pattern LIST
        preSortbyLength(vec_pattern, this->vec_pattern_grp);
    
        return 0;
    }

    XILINX_FUZZYMATCH_IMPL_DECL   
    bool FuzzyMatchSW::check(const std::string &t)
    {
        std::cout << "DEBUG: FuzzyMatchSW::check t=" << t << std::endl;
        //auto ts = std::chrono::high_resolution_clock::now();
        //FMResult r;
        // check for t against pattern vec
        bool r = strFuzzy(this->max_fuzzy_len, t, vec_pattern_grp);
    
        //auto te = std::chrono::high_resolution_clock::now();
        //r.timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(te - ts).count() / 1000.0f;
        std::cout << "DEBUG: FuzzyMatchSW::check r=" << r << std::endl;
        return r;
    }
    
} // namespace fuzzymatch
} // namespace xilinx_apps

