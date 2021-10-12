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

namespace xf {
namespace fuzzyMatch {

const int max_validated_people = 10000000;


// extract select column from CSV file.
// column id starts from 0.
// pass -1 to max_entry_num to get all lines.
int load_csv(const size_t max_entry_num,
             const size_t max_field_len,
             const std::string& file_path,
             const unsigned col,
             std::vector<std::string>& vec_str);

namespace internal {

class FMCPUChecker {
   public:
    FMCPUChecker() {
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

    std::vector<std::vector<std::string> > vec_grp =
        std::vector<std::vector<std::string> >(max_len_in_char);


    // do one fuzzy process
    bool doFuzzyTask(int id,
                     const size_t upper_limit,
                     const std::string& ptn_str,
                     const std::vector<std::vector<std::string> >& vec_grp);
    // do fuzzy match against given list, return true if matched.
    bool strFuzzy(const size_t upper_limit,
                  const std::string& ptn_str,
                  std::vector<std::vector<std::string> >& vec_grp_str);
                  /*
    // do equal match against given list, return true if matched.
    bool strEqual(const std::string& code1, const std::string& code2);
    // do string contain against given list, return true if matched.
    bool strContain(const std::string& description);*/

   private:
    static const size_t max_len_in_char = 1024 * 1024; // in char
  

    std::future<bool> worker[100];
    unsigned int totalThreadNum = std::thread::hardware_concurrency();

}; // end class FMCPUChecker
} // namespace internal

class FMChecker : public internal::FMCPUChecker {
   public:
    FMChecker() { this->max_fuzzy_len = 35; }

    // The intialize process will download FPGA binary to FPGA card, and initialize the HBM/DDR FACTIVA tables.
    int startFuzzyMatch(const std::string& xclbinPath, std::string deviceNames);
    int fuzzyMatchLoadVec(std::vector<std::string>& patternVec);

    // The check method returns whether the transaction is okay, and triggering condition if any.
    bool executefuzzyMatch(const std::string& t);

   private:
   static const int PU_NUM = 8;
    int boost;
    cl::Context ctx;
    cl::Program prg;
    cl::CommandQueue queue;
    cl::Kernel fuzzy[4];
    int buf_f_i_idx=0;

    int sum_line;
    //std::vector<int> vec_base = std::vector<std::vector<int> >;
    //std::vector<int> vec_offset = std::vector<std::vector<int> >;
    std::vector<int> vec_base ;
    std::vector<int> vec_offset ;

   cl::Buffer buf_field_i1;
   cl::Buffer buf_field_i2;
   cl::Buffer buf_csv[2 * PU_NUM];
   cl::Buffer buf_field_o1;
   cl::Buffer buf_field_o2;

   std::vector<cl::Event> events_write;
   std::vector<cl::Event> events_kernel;
   std::vector<cl::Event> events_read;

   void preCalculateOffsetPerPU(std::vector<std::vector<std::string> >& vec_grp_str,
                 std::vector<int>& vec_base,
                 std::vector<int>& vec_offset);


}; // end class FMChecker

} // namespace Fuzzymatch
} // namespace xf

#endif
