/*
 * Copyright 2019 Xilinx, Inc.
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

#ifndef HLS_TEST
#include "xcl2.hpp"
#else
#include "nHop_kernel.hpp"
#endif

#include "ap_int.h"
#include "utils.hpp"
#include "utils2.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <vector>
#include <stdlib.h>

#include <unordered_map>
#include "ctime.hpp"
CTimeModule<unsigned long> gtimer;

ap_uint<64>*  GetPair(const char* pairfile, long* num)
{

    long fileIdx = 0;
    std::fstream pairfstream(pairfile, std::ios::in);
    if (!pairfstream) {
        std::cout << "Error : " << pairfile << " file doesn't exist !" << std::endl;
        exit(1);
    }
    char line[1024] = {0};
    pairfstream.getline(line, sizeof(line));
    std::stringstream numPdata(line);
    int numPairs;
    numPdata >> numPairs;
    *num = numPairs;

    ap_uint<64>* pair = aligned_alloc<ap_uint<64> >(numPairs);

    while (pairfstream.getline(line, sizeof(line))) {
        std::stringstream data(line);
        ap_uint<64> tmp64;
        unsigned src;
        unsigned des;
        data >> src;
        tmp64.range(31, 0) = src - 1;
        data >> des;
        tmp64.range(63, 32) = des - 1;
        pair[fileIdx] = tmp64;
        fileIdx++;
    }
    pairfstream.close();
    return pair;
}

int Demo_1(ArgParser& parser){
    //1. get commendInfo, include the default value of the input commend
    commendInfo commendInfo;
    std::string xclbin_path;
#ifndef HLS_TEST
    if (!parser.getCmdOption("-xclbin", xclbin_path)) {
        std::cout << "ERROR:xclbin path is not set!\n";
        return 1;
    }
#endif

    std::string graphfile;
    std::string offsetfile;
    std::string indexfile;
    std::string pairfile;
    std::string goldenfile;
    std::string args;

    if (!parser.getCmdOption("--graph", graphfile)) {
        parser.getCmdOption("--offset", offsetfile);
        parser.getCmdOption("--index", indexfile);
        if(offsetfile.empty() && indexfile.empty() ){
            std::cout << "ERROR: graph file path is not set!\n";
            return -1;   
        }else{
            std::cout << "offset file path: "<< offsetfile <<"\n"; 
            std::cout << "index  file path: "<< indexfile <<"\n"; 
        }   
    }else{
        std::cout << "graph file path: "<< graphfile <<"\n";
    }

    if (!parser.getCmdOption("--pair", pairfile)) {
        std::cout << "WARNING: pair file path is not set!\n";
    }else{
        std::cout << "pair  file path: "<< pairfile <<"\n";
    }

    if (!parser.getCmdOption("--golden", goldenfile)) {
        //std::cout << "WARNING: golden file path is not set!, but only use in hls flow\n";
    }

    if (!parser.getCmdOption("--numKernel", args)) {
        std::cout << "Using default number of kernel: "<<commendInfo.numKernel<<std::endl;
    }else{
        commendInfo.numKernel = stoi(args);
    }

    if (!parser.getCmdOption("--numPuPerKernel", args)) {
        std::cout << "Using default number of PU: "<<commendInfo.numPuPerKernel<<std::endl;
    }else{
        commendInfo.numPuPerKernel = stoi(args);
    }

    ap_uint<32> num_hop = 3;
    if (!parser.getCmdOption("--hop", args)) {
        std::cout << "Using default number of hop: "<<num_hop<<std::endl;
    }else{
        num_hop = stoi(args);
    }

    if (!parser.getCmdOption("--batch", args)) {
        std::cout << "Using default number of batch size: "<<commendInfo.sz_bat<<std::endl;
    }else{
        commendInfo.sz_bat = stoi(args);
    }
    
    if (!parser.getCmdOption("--bypass", args)) {
        std::cout << "Using default bypass the aggration module? 0 means not bypass : "<<commendInfo.byPass<<std::endl;
    }else{
        commendInfo.byPass = stoi(args);
    }

    if (!parser.getCmdOption("--duplicate", args)) {
        std::cout << "Using default not duplicate for all channel? 0 means split the pair package to each channel "<<commendInfo.duplicate<<std::endl;
    }else{
        commendInfo.duplicate = stoi(args);
    }

    //for debug
    double tmp = 64.0;//  64 M vertex or edge
    if (!parser.getCmdOption("--limit", args)) {
        std::cout << "Using default limit(MB) for all channel "<<tmp<<std::endl;
    }else{
        tmp = stod(args);
        std::cout << "Using input limit(MB) for all channel "<<tmp<<std::endl;
    }

    std::string fn = pairfile.substr(pairfile.find_last_of('/') + 1);
    commendInfo.filename = fn.substr(0, fn.rfind(".")) + ".hop";  
    if (!parser.getCmdOption("--test", args)) {
        std::cout << "The results will output to the ./"<< commendInfo.filename <<" file "<<std::endl;
    }else{
        commendInfo.output = false;
        std::cout << "Using test mode and will not generate the output *.hop file "<<std::endl;   
    }

    commendInfo.xclbin_path = xclbin_path;

    //2. get graph and start partition
    std::cout << "INFO: Loading files ... "<<std::endl;
    CSR<unsigned> csr0;
    if(!graphfile.empty()){
        csr0.Init( graphfile.c_str(), true);
    }else{
        csr0.Init( offsetfile.c_str(), indexfile.c_str());
    }
    csr0.ShowInfo("csr0 Graph");

    PartitionHop<unsigned> par1(&csr0);
    double Limit_MB_v = tmp;//0.0004;//64;
    double Limit_MB_e = tmp;//0.0004;//64;//128;
    par1.CreatePartitionForKernel(commendInfo.numKernel, commendInfo.numPuPerKernel, Limit_MB_v*4*(1<<20), Limit_MB_e*4*(1<<20));

    //3. dispatch subgraph to multi kernels
    IndexStatistic stt;
    timeInfo timeInfo;
    long num_pair;
    ap_uint<64>* pair = GetPair(pairfile.c_str(), &num_pair);
    par1.LoadPair2Buffs(pair, num_pair, csr0.NV, csr0.NE, num_hop, commendInfo, &timeInfo, &stt);

    par1.PrintRpt( num_hop, num_pair, commendInfo, timeInfo, stt);
    //long num = 1000; 
    //ap_uint<64>* pair = GenPair(csr0.NV, num);
    //par1.LoadPair2Buffs(pair, num, csr0.NV, csr0.NE, 2);
    //SavePair(pair, num, "/wrk/xsjhdnobkup1/ryanw/nHop/xf_graph/L2/tests/nHop/pair1000.mtx");

    free(pair);
    return 0;
}

int main(int argc, const char* argv[]) {
    std::cout << "\n---------------------N Hop-------------------\n";

    // cmd parser
    ArgParser parser(argc, argv);

    return Demo_1(parser);

}
