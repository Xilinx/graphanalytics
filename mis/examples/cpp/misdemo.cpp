/*
 * Copyright 2021 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include <algorithm>
#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <cassert>
//#include "binFiles.hpp"
//#include "mis_kernel_xrt.hpp"
//#include "graph.hpp"
#include "xilinxmis.hpp"
using namespace xilinx_apps::mis;

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

template <typename T, typename A>
bool readBin(const std::string filename, const std::streampos readSize, std::vector<T, A>& vec) {
    std::ifstream file(filename, std::ios::binary);
    file.unsetf(std::ios::skipws);
    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    if (readSize > 0 && fileSize != readSize) {
        std::cout << "WARNNING: file " << filename << " size " << fileSize << " doesn't match required size " << readSize
             << std::endl;
    }
    assert(fileSize >= readSize);

    const std::streampos vecSize = fileSize / sizeof(T);
    vec.resize(vecSize);

    file.read(reinterpret_cast<char*>(vec.data()), fileSize);
    file.close();
    if (file)
        return true;
    else
        return false;
}
int main(int argc, const char* argv[]) {
    ArgParser parser(argc, argv);

    std::string xclbin_path;
    std::string deviceNames;
    std::string in_dir;
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
        std::cout << "ERROR: input matrix file path is not set!\n";
        return -1;
    }
    if (parser.getCmdOption("--devices", deviceNames)) {
        std::cout << "INFO: Set deviceNames to " << deviceNames << std::endl;
    } else {
    	deviceNames = "xilinx_u50_gen3x16_xdma_201920_3";
        std::cout << "INFO: Use default deviceNames " << deviceNames << std::endl;
    }

//search for input file matrix meta data information which stored in infos.txt
    std::ifstream file(in_dir + "/infos.txt");
    std::string line;
    getline(file, line);
    getline(file, line);
    uint32_t n = atoi(line.c_str());
    getline(file, line);
    getline(file, line);
    uint32_t nz = atoi(line.c_str());
    file.close();

    Options options;
    options.xclbinPath=xclbin_path;
    options.deviceNames=deviceNames;
    
    MIS xmis(options);
    std::vector<int> h_rowPtr(n + 1);
    std::vector<int> h_colIdx(nz);

    readBin(in_dir + "/rowPtr.bin", (n + 1) * sizeof(uint32_t), h_rowPtr);
    readBin(in_dir + "/colIdx.bin", nz * sizeof(uint32_t), h_colIdx);

    GraphCSR graph(std::move(h_rowPtr), std::move(h_colIdx));
 
    xmis.startMis();
    xmis.setGraph(&graph);
    auto start = std::chrono::high_resolution_clock::now();
    xmis.executeMIS();

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = stop-start;
    double elapsed = duration.count();
    int count = xmis.count();

    std::cout << "\nFound MIS with " << count << " vertices within " << elapsed << " sec" << std::endl;

    return 0;
 
}
