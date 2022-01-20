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
    std::string patternFile, inputFile;
    unsigned int patternIndex = 1, inputIndex = 1;
    unsigned int work_mode = 0; // FPGA-only mode
    std::string deviceNames;
    unsigned int totalEntities = 10000000;
    unsigned int numEntities = 100;
    unsigned int similarity_threshold = 90;
    std::string is_check_str;

    if (parser.getCmdOption("-h")) {
        std::cout << "Usage:\n\ttest.exe -xclbin XCLBIN_PATH -d WATCH_LIST_PATH [-c (0|1|2)]\n" << std::endl;
        std::cout
            << "Option:\n\t-xclbin XCLBIN_PATH\t\trequired, path to xclbin binary\n\t-d WATCH_LIST_PATH\t\trequired, "
               "the folder of watch list csv files\n\t-c 0|1|2\t\t\toptional, default 0 for FPAG only, 1 for CPU only, "
               "2 for both and comparing results\n";
        return 0;
    }

    if (!parser.getCmdOption("--xclbin", xclbin_path)) {
        std::cout << "ERROR: xclbin path is not set!\n";
        return -1;
    }

    if (!parser.getCmdOption("--pattern_file", patternFile)) {
        std::cout << "ERROR: csv file for pattern vectors is not set!\n";
        return -2;
    }

    if (parser.getCmdOption("--pattern_index", is_check_str)) {
        try {
            patternIndex = std::stoi(is_check_str);
        } catch (...) {
            patternIndex = 1;
        }
    }

    if (!parser.getCmdOption("--input_file", inputFile)) {
        std::cout << "ERROR: csv file for input vectors is not set!\n";
        return -3;
    }

    if (parser.getCmdOption("--input_index", is_check_str)) {
        try {
            inputIndex = std::stoi(is_check_str);
        } catch (...) {
            inputIndex = 1;
        }
    }

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
        return -4;
    }

    // Add Watch List CSV Files
    std::ifstream f;
    f.open(patternFile);
    if (f.good()) {
        f.close();
        f.clear();
    } else {
        std::cout << "Error: File " << patternFile << " cannot be found, please check and re-run.\n\n";
        exit(1);
    }
 
    // Read some transactions
    f.open(inputFile);
    if (f.good()) {
        f.close();
        f.clear();
    } else {
        std::cout << "Error: Test input file " << inputFile
                  << " cannot be found, please check and re-run." << std::endl;
        exit(-5);
    }

    std::vector<std::string> allInputVector;
    load_csv(numEntities, -1U, inputFile, inputIndex, allInputVector); 
    std::vector<std::string> inputVector(numEntities);
    for (unsigned int i = 0; i < numEntities; i++) {
        inputVector[i] = allInputVector[i];
    }

    std::vector<std::vector<std::pair<int,int>>> hwResult(numEntities);

    std::vector<std::string> patternVector;
    load_csv(totalEntities, -1U, patternFile , patternIndex, patternVector);
    std::cout << "INFO: total number of patterns loaded: " << patternVector.size() << std::endl;
    
    // Begin to analyze if on mode 0 or 2
    float hwExecTime = 0.0;
    if (work_mode == 0 || work_mode == 2) {
        Options options;
        options.xclbinPath=xclbin_path;
        options.deviceNames=deviceNames;
        FuzzyMatch fm(options);
        
        if (fm.startFuzzyMatch() < 0) {
            std::cout << "ERROR: Failed to initialize " << deviceNames << " with xclbin " << xclbin_path << std::endl;
            return -1;
        }

        fm.fuzzyMatchLoadVec(patternVector);
        float min = std::numeric_limits<float>::max(), max = 0.0;
   
        auto ts = std::chrono::high_resolution_clock::now();
        hwResult = fm.executefuzzyMatch(inputVector, similarity_threshold);
        auto te = std::chrono::high_resolution_clock::now();
        float timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(te - ts).count() / 1000.0f;

        if (min > timeTaken) min = timeTaken;
        if (max < timeTaken) max = timeTaken;
        hwExecTime += timeTaken;

#ifndef NDEBUG
        for (unsigned int i = 0; i < numEntities; i++) {
            if (hwResult[i].size() > 0)
                std::cout << "Input " << i << ":" << inputVector[i] << std::endl;

            for (unsigned int j = 0; j < hwResult[i].size(); j++) {
                int hw_id_t = hwResult[i][j].first;
                int hw_score_t = hwResult[i][j].second;
                std::cout << "    hw_id_t=" << hw_id_t 
                          << " matched pattern=" << patternVector[hw_id_t] 
                          << " similarity score=" << hw_score_t << std::endl;
            }
        }

#endif
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
        std::cout << "INFO: FuzzyMatch FPGA result: "  
                  << numEntities << " transactions were processed in " << hwExecTime << " ms" << std::endl;
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }


    // check the result
    FuzzyMatchSW cpu_checker;
    float swExecTime = 0.0; 
    if (work_mode == 1 || work_mode == 2) {
        std::vector<float> swperf(numEntities);
        int nerror = 0;
        cpu_checker.initialize(patternVector);

        float min = std::numeric_limits<float>::max(), max = 0.0, sum = 0.0;
        for (int i = 0; i < numEntities; i++) {

#ifndef NDEBUG
            std::cout << "DEBUG: finding match for " << inputVector[i] << std::endl;
#endif            
            auto ts = std::chrono::high_resolution_clock::now();
            std::unordered_map<int,int> swResult = cpu_checker.check(similarity_threshold, inputVector[i]);
            auto te = std::chrono::high_resolution_clock::now();
            float timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(te - ts).count() / 1000.0f;
            swperf[i] = timeTaken;

            if (work_mode == 2) {
                if (swResult.size() < 100 && swResult.size() != hwResult[i].size()) {
                    std::cout << "ERROR: Trans-"<< i << " number of matches is NOT matched!!!" << std::endl;
                    std::cout << swResult.size() << " <-> " << hwResult[i].size() << std::endl;
                    nerror++;
                } else {
#ifndef NDEBUG                    
                    std::cout << "total number of matched string = " << swResult.size() << std::endl;
#endif                    
                    for (unsigned int j = 0; j < hwResult[i].size(); j++) {
                        int hw_id_t = hwResult[i][j].first;
                        int hw_score_t = hwResult[i][j].second;
                        if (swResult.find(hw_id_t) != swResult.end() && swResult[hw_id_t] == hw_score_t) {
#ifndef NDEBUG                            
                            std::cout << "Check matched. <" << hw_id_t << ", " << hw_score_t << ">" << std::endl;
#endif                            
                        } else {
                            nerror++;
                            if (swResult.find(hw_id_t) == swResult.end())
                                std::cout << "ID is not matched!!! " << std::endl;
                            else
                                std::cout << "Score is not matched!!! " << swResult[hw_id_t] << " <-> " << hw_score_t
                                          << std::endl;
                        }
                    }
                }              

                if (min > swperf[i]) min = swperf[i];
                if (max < swperf[i]) max = swperf[i];
                swExecTime += swperf[i] ;
            } else if (work_mode == 1) {
                if (swResult.size() > 0) {
                    //std::cout << swResult[hw_id_t]
                }


            }
        }

        std::cout << "\nINFO: FuzzyMatch CPU Result: " << std::endl;
        std::cout << "Min(ms)\t\tMax(ms)\t\tAvg(ms)\n";
        std::cout << "----------------------------------------" << std::endl;
        std::cout << min << "\t\t" << max << "\t\t" << swExecTime / (numEntities - nerror) << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    
        if (work_mode == 2) {
            if (nerror != 0) {
                std::cout << "ERROR: faild to check " << nerror << " transactions!\n";
                return -1;
            } else {
                std::cout << "INFO: Check passed!\n";
                std::cout << "INFO: Pattern vector size: " << patternVector.size() << std::endl;
                std::cout << "INFO: Input size: " << numEntities << std::endl;
                std::cout << "INFO: FuzzyMatch FPGA speedup: " << swExecTime/hwExecTime << std::endl;
            }
        }
    }

    return 0;
}
