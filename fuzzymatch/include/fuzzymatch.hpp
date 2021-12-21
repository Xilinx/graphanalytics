/**
* Copyright (C) 2020 Xilinx, Inc
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

#ifndef _XILINX_AML_CHECKER_HEADER_
#define _XILINX_AML_CHECKER_HEADER_

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unordered_map>

#include "xilinx_apps_common.hpp"

/**
 * Define this macro to make functions in fuzzymatch_loader.cpp inline instead of extern.  You would use this macro
 * when including fuzzymatch_loader.cpp in a header file, as opposed to linking with libXilinxFuzzyMatch_loader.a.
 */
#ifdef XILINX_FUZZYMATCH_INLINE_IMPL
#define XILINX_FUZZYMATCH_IMPL_DECL inline
#else
#define XILINX_FUZZYMATCH_IMPL_DECL extern
#endif

// Make the variables below macro for now so the code can be shared with TigerGraph
// UDFs
#define max_validated_pattern 10000000
#define max_len_in_char 1024*1024
#define totalThreadNum std::thread::hardware_concurrency()
#define max_pattern_len_in_char 1024*1024

namespace xilinx_apps {
namespace fuzzymatch {

    struct Options;
    class FuzzyMatchImpl;

    // ------------------------------------------------------------------------
    // Utility functions 
    // ------------------------------------------------------------------------
    // generate the pattern by group 
    XILINX_FUZZYMATCH_IMPL_DECL
    void preSortbyLength(std::vector<std::string>& vec_pattern,
                         std::vector<std::vector<std::string>>& vec_pattern_grp);

    // extract select column from CSV file.
    // column id starts from 0.
    // pass -1 to max_entry_num to get all lines.
    XILINX_FUZZYMATCH_IMPL_DECL
    int load_csv(const size_t max_entry_num, const size_t max_field_len, const std::string &file_path,
                 const unsigned col, std::vector<std::string> &vec);

    XILINX_FUZZYMATCH_IMPL_DECL         
    int min(int a, int b);
    
    XILINX_FUZZYMATCH_IMPL_DECL
    int abs(int a, int b);

    XILINX_FUZZYMATCH_IMPL_DECL
    float similarity(std::string str1, std::string str2);

    XILINX_FUZZYMATCH_IMPL_DECL
    size_t getMaxDistance(size_t len);

    XILINX_FUZZYMATCH_IMPL_DECL
    bool doFuzzyTask(int thread_id, const size_t upper_limit, const std::string &pattern,
                     const std::vector<std::vector<std::string>> &vec_grp_str);

    XILINX_FUZZYMATCH_IMPL_DECL
    bool strFuzzy(const size_t upper_limit, const std::string &pattern,
                  std::vector<std::vector<std::string>> &vec_grp_str);

}

}

extern "C" {
XILINX_FUZZYMATCH_IMPL_DECL
xilinx_apps::fuzzymatch::FuzzyMatchImpl *xilinx_fuzzymatch_createImpl(const xilinx_apps::fuzzymatch::Options& options);

XILINX_FUZZYMATCH_IMPL_DECL
void xilinx_fuzzymatch_destroyImpl(xilinx_apps::fuzzymatch::FuzzyMatchImpl *pImpl);
}

namespace xilinx_apps {
namespace fuzzymatch {

/**
 * @brief %Exception class for cosine similarity run-time errors
 * 
 * This exception class is derived from `std::exception` and provides the standard @ref what() member function.
 * An object of this class is constructed with an error message string, which is stored internally and
 * retrieved with the @ref what() member function.
 */
class Exception : public std::exception {
    std::string message;
public:
    /**
     * Constructs an Exception object.
     * 
     * @param msg an error message string, which is copied and stored internal to the object
     */
    Exception(const std::string &msg) : message(msg) {}
    
    /**
     * Returns the error message string passed to the constructor.
     * 
     * @return the error message string
     */
    virtual const char* what() const noexcept override { return message.c_str(); }
};

/**
 * @brief Struct containing CosineSim configuration options
 */
struct Options {
    XString xclbinPath;
    XString deviceNames;
};

class FuzzyMatch  {
   public:
    
    FuzzyMatch(const Options &options) : pImpl_(xilinx_fuzzymatch_createImpl(options)) { }

    ~FuzzyMatch() { xilinx_fuzzymatch_destroyImpl(pImpl_); }
    // The intialize process will download FPGA binary to FPGA card, and initialize the HBM/DDR FACTIVA tables.
    int startFuzzyMatch();
    //int fuzzyMatchLoadVec(std::vector<std::string>& patternVec);
    int fuzzyMatchLoadVec(std::vector<std::string>& vec_pattern,std::vector<int> vec_id=std::vector<int>());

    // run fuzzymatch in batch mode
    // return vector of  hit patterns for each input string. 
    std::vector<std::vector<std::pair<int,int>>> executefuzzyMatch(std::vector<std::string> input_patterns, int similarity_level);

   private:
     FuzzyMatchImpl *pImpl_ = nullptr;

}; // end class FuzzyMatch

class FuzzyMatchSW {
   public:
    FuzzyMatchSW() {
        max_fuzzy_len = -1;
        max_contain_len = -1;
        max_equan_len = 12;
    }

    //  initialize the FACTIVA tables and do pre-sort
    // if vec_id is missing, internally  it will be set as vec_pattern index number
    int initialize(const std::string& fileName);
    int initialize(std::vector<std::string>& vec_pattern, std::vector<int> vec_id=std::vector<int>{});


    // The check method returns top result id->scores, and triggering condition if any.
    //bool check(const std::string& t);
    std::unordered_map<int,int>  check(int threshold, const std::string &ptn_string);

   protected:
    size_t max_fuzzy_len;
    size_t max_contain_len;
    size_t max_equan_len;

    std::vector<std::vector<std::string> > vec_pattern_grp =
        std::vector<std::vector<std::string> >(max_len_in_char);

    std::vector<std::vector<int> > vec_pattern_id = 
        std::vector<std::vector<int> >(max_len_in_char);

}; // end class FuzzyMatchSW

} // namespace fuzzymatch
} // namespace xilinx_apps

#endif


