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

#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1

#include <CL/cl2.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <vector>

#include "xilinx_apps_common.hpp"

namespace xilinx_apps
{
namespace fuzzymatch
{
    struct Options;
    class FuzzyMatchImpl;
}

}

/**
 * Define this macro to make functions in fuzzymatch_loader.cpp inline instead of extern.  You would use this macro
 * when including fuzzymatch_loader.cpp in a header file, as opposed to linking with libXilinxFuzzyMatch_loader.a.
 */
#ifdef XILINX_FUZZYMATCH_INLINE_IMPL
#define XILINX_FUZZYMATCH_IMPL_DECL inline
#else
#define XILINX_FUZZYMATCH_IMPL_DECL extern
#endif

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
    //std::string xclbinPath;
    //std::string deviceNames;
};




// extract select column from CSV file.
// column id starts from 0.
// pass -1 to max_entry_num to get all lines.
int load_csv(const size_t max_entry_num,
             const size_t max_field_len,
             const std::string& file_path,
             const unsigned col,
             std::vector<std::string>& vec_str);






class FuzzyMatch  {
   public:
    
    FuzzyMatch(const Options &options) : pImpl_(xilinx_fuzzymatch_createImpl(options)) { }

    ~FuzzyMatch() { xilinx_fuzzymatch_destroyImpl(pImpl_); }
    // The intialize process will download FPGA binary to FPGA card, and initialize the HBM/DDR FACTIVA tables.
    int startFuzzyMatch();
    int fuzzyMatchLoadVec(std::vector<std::string>& patternVec);

    // The check method returns whether the transaction is okay, and triggering condition if any.
    bool executefuzzyMatch(const std::string& t);

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
    int initialize(const std::string& fileName);

    // The check method returns whether the transaction is okay, and triggering condition if any.
    bool check(const std::string& t);

   protected:
    size_t max_fuzzy_len;
    size_t max_contain_len;
    size_t max_equan_len;

    std::vector<std::vector<std::string> > vec_pattern_grp =
        std::vector<std::vector<std::string> >(max_len_in_char);



   private:
    static const size_t max_len_in_char = 1024 * 1024; // in char


}; // end class FuzzyMatchSW

} // namespace fuzzymatch
} // namespace xilinx_apps

#endif


