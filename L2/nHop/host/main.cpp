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
ap_uint<64>* GenPair(long NV, long num){
    ap_uint<64>* pair = aligned_alloc<ap_uint<64> >(num);
    for(int i=0; i<num; i++){
        ap_uint<32> head= random()%NV+1;
        ap_uint<32> tail= random()%NV+1;
        pair[i](31,0) = head;
        pair[i](63,32) = tail; 
    }
    return pair;
}
void SavePair(ap_uint<64>* pair, long num, const char* name){
    FILE* fp=fopen(name, "w");
    fprintf(fp,"%ld\n", num);
    for(int i=0; i<num; i++){
        long head= pair[i](31,0);
        long tail= pair[i](63,32);
        fprintf(fp, "%ld %ld \n", head, tail);
    }
    fclose(fp);
}


int Demo_1(ArgParser& parser){
    //1. get commendInfo
    commendInfo commendInfo;
    std::string xclbin_path;
#ifndef HLS_TEST
    if (!parser.getCmdOption("-xclbin", xclbin_path)) {
        std::cout << "ERROR:xclbin path is not set!\n";
        return 1;
    }
#endif

    std::string offsetfile;
    std::string indexfile;
    std::string pairfile;
    std::string goldenfile;
    std::string args;

    if (!parser.getCmdOption("--offset", offsetfile)) {
        std::cout << "ERROR: offset file path is not set!\n";
        return -1;
    }

    if (!parser.getCmdOption("--index", indexfile)) {
        std::cout << "ERROR: index file path is not set!\n";
        return -1;
    }

    if (!parser.getCmdOption("--pair", pairfile)) {
        std::cout << "ERROR: pair file path is not set!\n";
        return -1;
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
        std::cout << "Using default duplicate for all channel? 1 means do the duplicate "<<commendInfo.duplicate<<std::endl;
    }else{
        commendInfo.duplicate = stoi(args);
    }

    commendInfo.xclbin_path = xclbin_path;

    //2. get graph and start partition
    CSR<unsigned> csr0;
    csr0.Init( offsetfile.c_str(), indexfile.c_str());
    csr0.ShowInfo("csr0 Graph");

    PartitionHop<unsigned> par1(&csr0);
    int Limit_MB_v = 64;
    int Limit_MB_e = 64;//128;
    par1.CreatePartitionForKernel(commendInfo.numKernel, commendInfo.numPuPerKernel, Limit_MB_v*4*(1<<20), Limit_MB_e*4*(1<<20));

    //3. dispatch subgraph to multi kernels
    long num_pair;
    ap_uint<64>* pair = GetPair(pairfile.c_str(), &num_pair);
    par1.LoadPair2Buffs(pair, num_pair, csr0.NV, csr0.NE, num_hop, commendInfo);

    //long num = 1000; 
    //ap_uint<64>* pair = GenPair(csr0.NV, num);
    //par1.LoadPair2Buffs(pair, num, csr0.NV, csr0.NE, 2);
    //SavePair(pair, num, "/wrk/xsjhdnobkup1/ryanw/nHop/xf_graph/L2/tests/nHop/pair1000.mtx");

    free(pair);
    return 0;
}

int main(int argc, const char* argv[]) {
    std::cout << "\n---------------------N Hop-------------------\n";
    cl_int fail;

    // cmd parser
    ArgParser parser(argc, argv);

    return Demo_1(parser);

}
