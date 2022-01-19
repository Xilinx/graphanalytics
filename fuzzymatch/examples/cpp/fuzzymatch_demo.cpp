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

#include <algorithm>
#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include "fuzzymatch.hpp"


using namespace xilinx_apps::fuzzymatch;

std::string print_result(int id, bool r, float timeTaken) {
    std::string res = "";
    res += std::to_string(id);
    res += ",";
    if(r) {
        res += "KO" ;
    } else {
        res += "OK" ; 
    }
    res += ",";
    
    if (r) {
        res += ":";
        res += "Sender";
    }
    res += ",";
    res += std::to_string(timeTaken);

    return res;
}

class ArgParser {
   public:
    ArgParser(int& argc, const char** argv) {
        for (int i = 1; i < argc; ++i) mTokens.push_back(std::string(argv[i]));
    }
    bool getCmdOption(const std::string option) const {
        std::vector<std::string>::const_iterator itr;
        itr = std::find(this->mTokens.begin(), this->mTokens.end(), option);
        if (itr != this->mTokens.end()) {
            return true;
        }
        return false;
    }
    bool getCmdOption(const std::string option, std::string& value) const {
        std::vector<std::string>::const_iterator itr;
        itr = std::find(this->mTokens.begin(), this->mTokens.end(), option);
        if (itr != this->mTokens.end() && ++itr != this->mTokens.end()) {
            value = *itr;
            return true;
        }
        return false;
    }

   private:
    std::vector<std::string> mTokens;
};

int main(int argc, const char* argv[]) {
    ArgParser parser(argc, argv);

    std::string xclbin_path;
    std::string in_dir;
    unsigned int work_mode = 0; // FPGA-only mode
    std::string deviceNames;
    unsigned int totalEntities = 10000000;
    unsigned int numEntities = 100;
    unsigned int similarity_threshold = 90;

    if (parser.getCmdOption("-h")) {
        std::cout << "Usage:\n\ttest.exe -xclbin XCLBIN_PATH -d WATCH_LIST_PATH [-c (0|1|2)]\n" << std::endl;
        std::cout
            << "Option:\n\t-xclbin XCLBIN_PATH\t\trequired, path to xclbin binary\n\t-d WATCH_LIST_PATH\t\trequired, "
               "the folder of watch list csv files\n\t-c 0|1|2\t\t\toptional, default 0 for FPAG only, 1 for CPU only, "
               "2 for both and comparing results\n";
        return 0;
    }

    if (!parser.getCmdOption("-xclbin", xclbin_path)) {
        std::cout << "ERROR: xclbin path is not set!\n";
        return -1;
    }
    if (!parser.getCmdOption("-d", in_dir)) {
        std::cout << "ERROR: input watch list csv file path is not set!\n";
        return -1;
    }
    std::string is_check_str;
    if (parser.getCmdOption("-c", is_check_str)) {
        try {
            work_mode = std::stoi(is_check_str);
        } catch (...) {
            work_mode = 0;
        }
    }
    if (parser.getCmdOption("--devices", deviceNames)) {
        std::cout << "INFO: Set deviceNames to " << deviceNames << std::endl;
    } else {
    	deviceNames = "xilinx_u50_gen3x16_xdma_201920_3";
        std::cout << "INFO: Use default deviceNames " << deviceNames << std::endl;
    }
    
    if (parser.getCmdOption("--total_entities", is_check_str)) {
        try {
            totalEntities = std::stoi(is_check_str);
        } catch (...) {
            totalEntities = 10000000;
        }
    }

    if (parser.getCmdOption("--num_entities", is_check_str)) {
        try {
            numEntities = std::stoi(is_check_str);
        } catch (...) {
            numEntities = 100;
        }
    }

    if (parser.getCmdOption("--threshold", is_check_str)) {
        try {
            similarity_threshold = std::stoi(is_check_str);
        } catch (...) {
            similarity_threshold = 90;
        }
    }

    if (work_mode == 0)
        std::cout << "Select FPGA-only work mode\n";
    else if (work_mode == 1)
        std::cout << "Select CPU-only work mode\n";
    else if (work_mode == 2)
        std::cout << "Select both FPGA and CPU checker\n";
    else {
        std::cout << "ERROR: work mode out of range [0,2]" << std::endl;
        return -1;
    }


    // Add Watch List CSV Files
    std::ifstream f;
    const std::string peopleFile = in_dir + "/" + "all-names.csv";
    f.open(peopleFile);
    if (f.good()) {
        f.close();
        f.clear();
    } else {
        std::cout << "Error: File " << peopleFile << " cannot be found, please check and re-run.\n\n";
        exit(1);
    }
 
    // Read some transactions
    std::string test_input = in_dir + "/" + "new-names.csv";
    f.open(test_input);
    if (f.good()) {
        f.close();
        f.clear();
    } else {
        std::cout << "Error: Test input file " << test_input
                  << " cannot be found, please check and re-run.\n\n";
        exit(1);
    }

    std::vector<std::string> list_trans;
    load_csv(numEntities, -1U, test_input, 1, list_trans); 
    std::vector<std::string> test_transaction(numEntities);
    for (int i = 0; i < numEntities; i++) {
        test_transaction[i] = list_trans[i];
    }

    std::vector<std::vector<std::pair<int,int>>> result_set(numEntities);
    float perf0;

    std::vector<std::string> peopleVec;
    load_csv(totalEntities, -1U, peopleFile , 1, peopleVec);
    
    // Begin to analyze if on mode 0 or 2
    if (work_mode == 0 || work_mode == 2) {
        Options options;
        options.xclbinPath=xclbin_path;
        options.deviceNames=deviceNames;
        FuzzyMatch fm(options);
        
        //checker.initialize(xclbin_path, stopKeywordFile, peopleFile, entityFile, BICRefFile, 0); // card 0
        if (fm.startFuzzyMatch() < 0) {
            std::cout << "ERROR: Failed to initialize " << deviceNames << " with xclbin " << xclbin_path << std::endl;
            return -1;
        }

        fm.fuzzyMatchLoadVec(peopleVec);
        float min = std::numeric_limits<float>::max(), max = 0.0, sum = 0.0;
   
        auto ts = std::chrono::high_resolution_clock::now();
        result_set = fm.executefuzzyMatch(test_transaction, similarity_threshold);
        auto te = std::chrono::high_resolution_clock::now();
        float timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(te - ts).count() / 1000.0f;
        perf0 = timeTaken;

        if (min > timeTaken) min = timeTaken;
        if (max < timeTaken) max = timeTaken;
        sum += timeTaken;

        std::cout << "\nFor FPGA, ";
        std::cout << numEntities << " transactions were processed.\n";

        std::cout << "Min(ms)\t\tMax(ms)\t\tAvg(ms)\n";
        std::cout << "----------------------------------------" << std::endl;
        std::cout << min << "\t\t" << max << "\t\t" << sum / numEntities << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }

    // check the result
    FuzzyMatchSW cpu_checker;
    if (work_mode == 1 || work_mode == 2) {
        std::vector<float> swperf0(numEntities);
        if (work_mode == 2) std::cout << "\nStart to check...\n";
        int nerror = 0;
        cpu_checker.initialize(peopleFile);

        float min = std::numeric_limits<float>::max(), max = 0.0, sum = 0.0;
        for (int i = 0; i < numEntities; i++) {
            auto ts = std::chrono::high_resolution_clock::now();
            std::unordered_map<int,int> swresult = cpu_checker.check(similarity_threshold,test_transaction[i]);
            auto te = std::chrono::high_resolution_clock::now();
            float timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(te - ts).count() / 1000.0f;
            swperf0[i] = timeTaken;

            if (work_mode == 2) {
                if(swresult.size()<100 && swresult.size() != result_set[i].size()) {
                    std::cout << "Error: Trans-"<< i << " number of matches is NOT matched!!!" << std::endl;
                    std::cout << swresult.size() << " <-> " << result_set[i].size() << std::endl;
                    nerror++;
                } else {
                    std::cout << "total number of matched string = " << swresult.size() << std::endl;
                    for (unsigned int j = 0; j < result_set[i].size(); j++) {
                        int hw_id_t = result_set[i][j].first;
                        int hw_score_t = result_set[i][j].second;
                        if (swresult.find(hw_id_t) != swresult.end() && swresult[hw_id_t] == hw_score_t) {
                            std::cout << "Check matched. <" << hw_id_t << ", " << hw_score_t << ">" << std::endl;
                        } else {
                            nerror++;
                            if (swresult.find(hw_id_t) == swresult.end())
                                std::cout << "ID is not matched!!! " << std::endl;
                            else
                                std::cout << "Score is not matched!!! " << swresult[hw_id_t] << " <-> " << hw_score_t
                                          << std::endl;
                        }
                    }
                }              

                if (min > swperf0[i]) min = swperf0[i];
                if (max < swperf0[i]) max = swperf0[i];
                sum +=swperf0[i] ;
            }
        }

        std::cout << "\nFor CPU, " << std::endl;
        std::cout << "Min(ms)\t\tMax(ms)\t\tAvg(ms)\n";
        std::cout << "----------------------------------------" << std::endl;
        std::cout << min << "\t\t" << max << "\t\t" << sum / (numEntities - nerror) << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    
        if (work_mode == 2) {
            if (nerror != 0) {
                std::cout << "Error: faild to check " << nerror << " transactions!\n";
                return -1;
            } else {
                std::cout << "Check passed!\n";
            }
        }
    }

    return 0;
}
