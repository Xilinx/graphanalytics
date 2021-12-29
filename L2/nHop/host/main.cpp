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

#include "xf_utils_sw/logger.hpp"
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

class ArgParser {
   public:
    ArgParser(int& argc, const char** argv) {
        for (int i = 1; i < argc; ++i) mTokens.push_back(std::string(argv[i]));
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
    CSR<unsigned> csr0;
    //bool hasWeight=true;
    //csr0.Init( xclbin_path.c_str(), hasWeight);
    //csr0.ShowInfo("csr0 Graph");

    std::string offsetfile;
    parser.getCmdOption("--offset", offsetfile);
    std::string indexfile;
    parser.getCmdOption("--index", indexfile);
    csr0.Init( offsetfile.c_str(), indexfile.c_str());
    csr0.ShowInfo("csr0 Graph");

    PartitionHop<unsigned> par1(&csr0);

    xclbinInfo xclbinInfo;
    parser.getCmdOption("-xclbin", xclbinInfo.xclbin_path);
    int num_knl =1;// small graph can be 8
    xclbinInfo.num_chnl = 4;

    int Limit_MB_v = 20;
    int Limit_MB_e = 20;
    par1.CreatePartitionForKernel(num_knl, xclbinInfo.num_chnl, Limit_MB_v*4*(1<<20), Limit_MB_e*4*(1<<20));

    
    std::string pairfile;
    if (!parser.getCmdOption("--pair", pairfile)) {
        std::cout << "ERROR: pair file path is not set!\n";
        return -1;
    }
    int num_hop = 3;
    std::string str_hop;
    if (!parser.getCmdOption("--hop", str_hop)) {
        std::cout << "Using default number of hop: "<<num_hop<<std::endl;
    }else{
        num_hop = atoi(str_hop.c_str());
    }
    long num_pair;
 
    ap_uint<64>* pair = GetPair(pairfile.c_str(), &num_pair);
    par1.LoadPair2Buffs(pair, num_pair, csr0.NV, csr0.NE, num_hop, xclbinInfo);

    long num = 1000; 
    //ap_uint<64>* pair = GenPair(csr0.NV, num);
    //par1.LoadPair2Buffs(pair, num, csr0.NV, csr0.NE, 2);
    //SavePair(pair, num, "/wrk/xsjhdnobkup1/ryanw/nHop/xf_graph/L2/tests/nHop/pair1000.mtx");

    free(pair);
    return 0;
}
int main(int argc, const char* argv[]) {
    std::cout << "\n---------------------N Hop-------------------\n";
    xf::common::utils_sw::Logger logger(std::cout, std::cerr);
    cl_int fail;

    // cmd parser
    ArgParser parser(argc, argv);
    std::string xclbin_path;
#ifndef HLS_TEST
    if (!parser.getCmdOption("-xclbin", xclbin_path)) {
        std::cout << "ERROR:xclbin path is not set!\n";
        return 1;
    }
#endif
    return Demo_1(parser);

    std::string offsetfile;
    std::string indexfile;
    std::string pairfile;
    std::string goldenfile;

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
        std::cout << "ERROR: golden file path is not set!\n";
        return -1;
    }

    // -------------setup k0 params---------------
    int err = 0;

    char line[1024] = {0};
    int fileIdx = 0;

    unsigned numVertices;
    unsigned numEdges;
    int numPairs;

    unsigned* offset32;
    unsigned* index32;
    if( LoadGraphFile_Dir<unsigned> ( 
        offsetfile.c_str(), indexfile.c_str(),
        numVertices, numEdges,
        offset32, index32)==-1){
            exit(-1);
        }
        
    CSR<unsigned> csr1;
    csr1.Init( offsetfile.c_str(), indexfile.c_str());
    csr1.ShowInfo("Source Graph");
    csr1.PrintSelf();
    CSR<unsigned>* par[8];
    int num_par =  CSRPartition_average<unsigned> (4,csr1,
        256000000/sizeof(unsigned), 256000000/sizeof(unsigned), par);

    for(int i=0; i< num_par; i++){
        par[i]->ShowInfo("Par_", i);
        par[i]->PrintSelf();
    }
    
/*
    std::fstream offsetfstream(offsetfile.c_str(), std::ios::in);
    if (!offsetfstream) {
        std::cout << "Error : " << offsetfile << " file doesn't exist !" << std::endl;
        exit(1);
    }

    offsetfstream.getline(line, sizeof(line));
    std::stringstream numOdata(line);
    numOdata >> numVertices;

    unsigned* offset32 = aligned_alloc<unsigned>(numVertices + 1);
    while (offsetfstream.getline(line, sizeof(line))) {
        std::stringstream data(line);
        data >> offset32[fileIdx];
        fileIdx++;
    }
    offsetfstream.close();

    fileIdx = 0;
    std::fstream indexfstream(indexfile.c_str(), std::ios::in);
    if (!indexfstream) {
        std::cout << "Error : " << indexfile << " file doesn't exist !" << std::endl;
        exit(1);
    }

    indexfstream.getline(line, sizeof(line));
    std::stringstream numCdata(line);
    numCdata >> numEdges;

    unsigned* index32 = aligned_alloc<unsigned>(numEdges);
    while (indexfstream.getline(line, sizeof(line))) {
        std::stringstream data(line);
        data >> index32[fileIdx];
        float tmp;
        data >> tmp;
        fileIdx++;
    }
    indexfstream.close();*/

    fileIdx = 0;
    std::fstream pairfstream(pairfile.c_str(), std::ios::in);
    if (!pairfstream) {
        std::cout << "Error : " << pairfile << " file doesn't exist !" << std::endl;
        exit(1);
    }

    pairfstream.getline(line, sizeof(line));
    std::stringstream numPdata(line);
    numPdata >> numPairs;

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

    // dispatch offset and index
    ap_uint<32>* offsetTable;
    offsetTable =aligned_alloc<ap_uint<32> >(9);
    ap_uint<32>* indexTable;
    indexTable =aligned_alloc<ap_uint<32> >(9);
    unsigned* offsetBuffer[8];
    unsigned* indexBuffer[8];

    ap_uint<32> numVerticesPU = numVertices / 8;
    offsetTable[8] = numVertices;
    for (int i = 0; i < 8; i++) {
        offsetTable[i] = numVerticesPU * i;
        offsetBuffer[i] = aligned_alloc<unsigned>(numVerticesPU + 4096);
    }
    for (int i = 0; i < 8; i++) {
        for (int j = offsetTable[i]; j < offsetTable[i + 1] + 1; j++) {
            offsetBuffer[i][j - offsetTable[i]] = offset32[j];
            // std::cout << "i=" << i << " j=" << j - offsetTable[i] << " offset=" << offset32[j] << std::endl;
        }
    }

    for (int i = 0; i < 9; i++) {
        indexTable[i] = offset32[offsetTable[i]];
    }
    for (int i = 0; i < 9; i++) {
        std::cout << "id=" << i << " offsetTable=" << offsetTable[i] << " indexTable=" << indexTable[i] << std::endl;
    }
    for (int i = 0; i < 8; i++) {
        ap_uint<32> numEdgesPU = indexTable[i + 1] - indexTable[i];
        std::cout << "numEgdesPU=" << numEdgesPU << std::endl;
        indexBuffer[i] = aligned_alloc<unsigned>(numEdgesPU + 4096);
        for (int j = indexTable[i]; j < indexTable[i + 1]; j++) {
            indexBuffer[i][j - indexTable[i]] = index32[j];
            // std::cout << "i=" << i << " j=" << j - indexTable[i] << " index=" << index32[j] << std::endl;
        }
    }

    // initilaize buffer and config
    ap_uint<32> numHop = 2;
#ifndef HLS_TEST
    ap_uint<32> batchSize = 100;
#else
    ap_uint<32> batchSize = 20;
#endif
    ap_uint<128>* result = aligned_alloc<ap_uint<128> >(numPairs + 1);
    memset(result, 0, sizeof(ap_uint<128>) * (numPairs + 1));
    ap_uint<128>* zeroBuffer0 = aligned_alloc<ap_uint<128> >(1 << 20);
    memset(zeroBuffer0, 0, sizeof(ap_uint<128>) * (1 << 20));
    ap_uint<128>* zeroBuffer1 = aligned_alloc<ap_uint<128> >(1 << 20);
    memset(zeroBuffer1, 0, sizeof(ap_uint<128>) * (1 << 20));
    
    // do pre-process on CPU
    struct timeval start_time, end_time;

#ifndef HLS_TEST
    // platform related operations
    std::vector<cl::Device> devices = xcl::get_xil_devices();
    cl::Device device = devices[0];

    // Creating Context and Command Queue for selected Device
    cl::Context context(device, NULL, NULL, NULL, &fail);
    logger.logCreateContext(fail);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &fail);
    logger.logCreateCommandQueue(fail);
    std::string devName = device.getInfo<CL_DEVICE_NAME>();
    printf("Found Device=%s\n", devName.c_str());

    cl::Program::Binaries xclBins = xcl::import_binary_file(xclbin_path);
    devices.resize(1);
    devices[0] = device;
    cl::Program program(context, devices, xclBins, NULL, &fail);
    logger.logCreateProgram(fail);
    cl::Kernel nHop;
    nHop = cl::Kernel(program, "nHop_kernel", &fail);
    logger.logCreateKernel(fail);
    std::cout << "kernel has been created" << std::endl;

    std::vector<cl_mem_ext_ptr_t> mext_o(22);

    mext_o[0] = {(unsigned int)(0) | XCL_MEM_TOPOLOGY, pair, 0};

    mext_o[1] = {(unsigned int)(2) | XCL_MEM_TOPOLOGY, offsetBuffer[0], 0};
    mext_o[2] = {(unsigned int)(4) | XCL_MEM_TOPOLOGY, indexBuffer[0], 0};
    mext_o[3] = {(unsigned int)(3) | XCL_MEM_TOPOLOGY, offsetBuffer[1], 0};
    mext_o[4] = {(unsigned int)(6) | XCL_MEM_TOPOLOGY, indexBuffer[1], 0};
    mext_o[5] = {(unsigned int)(8) | XCL_MEM_TOPOLOGY, offsetBuffer[2], 0};
    mext_o[6] = {(unsigned int)(10) | XCL_MEM_TOPOLOGY, indexBuffer[2], 0};
    mext_o[7] = {(unsigned int)(9) | XCL_MEM_TOPOLOGY, offsetBuffer[3], 0};
    mext_o[8] = {(unsigned int)(12) | XCL_MEM_TOPOLOGY, indexBuffer[3], 0};
    mext_o[9] = {(unsigned int)(14) | XCL_MEM_TOPOLOGY, offsetBuffer[4], 0};
    mext_o[10] = {(unsigned int)(16) | XCL_MEM_TOPOLOGY, indexBuffer[4], 0};
    mext_o[11] = {(unsigned int)(15) | XCL_MEM_TOPOLOGY, offsetBuffer[5], 0};
    mext_o[12] = {(unsigned int)(18) | XCL_MEM_TOPOLOGY, indexBuffer[5], 0};
    mext_o[13] = {(unsigned int)(20) | XCL_MEM_TOPOLOGY, offsetBuffer[6], 0};
    mext_o[14] = {(unsigned int)(22) | XCL_MEM_TOPOLOGY, indexBuffer[6], 0};
    mext_o[15] = {(unsigned int)(21) | XCL_MEM_TOPOLOGY, offsetBuffer[7], 0};
    mext_o[16] = {(unsigned int)(24) | XCL_MEM_TOPOLOGY, indexBuffer[7], 0};

    mext_o[17] = {(unsigned int)(26) | XCL_MEM_TOPOLOGY, zeroBuffer0, 0};
    mext_o[18] = {(unsigned int)(28) | XCL_MEM_TOPOLOGY, zeroBuffer1, 0};

    mext_o[19] = {(unsigned int)(0) | XCL_MEM_TOPOLOGY, offsetTable, 0};
    mext_o[20] = {(unsigned int)(0) | XCL_MEM_TOPOLOGY, indexTable, 0};

    mext_o[21] = {(unsigned int)(1) | XCL_MEM_TOPOLOGY, result, 0};

    // create device buffer and map dev buf to host buf
    cl::Buffer pair_buf, result_buf, ping_buf, pong_buf, offset_table, index_table;
    cl::Buffer offset_buf[8];
    cl::Buffer index_buf[8];

    pair_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                          sizeof(ap_uint<64>) * numPairs, &mext_o[0]);
    ping_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                          sizeof(ap_uint<128>) * (1 << 20), &mext_o[17]);
    pong_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                          sizeof(ap_uint<128>) * (1 << 20), &mext_o[18]);
    offset_table = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                              sizeof(ap_uint<32>) * (9), &mext_o[19]);
    index_table = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                             sizeof(ap_uint<32>) * (9), &mext_o[20]);
    result_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                            sizeof(ap_uint<128>) * (numPairs + 1), &mext_o[21]);

    for (int i = 0; i < 8; i++) {
        offset_buf[i] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                   sizeof(unsigned) * (numVerticesPU + 4096), &mext_o[1 + 2 * i]);
        index_buf[i] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                  sizeof(unsigned) * (indexTable[i + 1] - indexTable[i] + 4096), &mext_o[2 + 2 * i]);
    }

    std::vector<cl::Memory> init;
    init.push_back(pair_buf);
    init.push_back(result_buf);
    init.push_back(offset_table);
    init.push_back(index_table);
    init.push_back(ping_buf);
    init.push_back(pong_buf);
    for (int i = 0; i < 8; i++) {
        init.push_back(offset_buf[i]);
        init.push_back(index_buf[i]);
    }
    q.enqueueMigrateMemObjects(init, CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED, nullptr, nullptr);
    q.finish();

    std::vector<cl::Memory> ob_in;
    std::vector<cl::Memory> ob_out;
    std::vector<cl::Event> events_write(1);
    std::vector<cl::Event> events_kernel(1);
    std::vector<cl::Event> events_read(1);

    ob_in.push_back(pair_buf);
    ob_in.push_back(offset_table);
    ob_in.push_back(index_table);
    for (int i = 0; i < 8; i++) {
        ob_in.push_back(offset_buf[i]);
        ob_in.push_back(index_buf[i]);
    }
    q.enqueueMigrateMemObjects(ob_in, 0, nullptr, &events_write[0]);

    ob_out.push_back(result_buf);
    // launch kernel and calculate kernel execution time
    std::cout << "kernel start------" << std::endl;
    gettimeofday(&start_time, 0);
    int j = 0;
    nHop.setArg(j++, numHop);
    nHop.setArg(j++, numPairs);
    nHop.setArg(j++, batchSize);
    nHop.setArg(j++, pair_buf);
    nHop.setArg(j++, offset_table);
    nHop.setArg(j++, index_table);
    for (int i = 0; i < 8; i++) {
        nHop.setArg(j++, offset_buf[i]);
        nHop.setArg(j++, index_buf[i]);
    }
    nHop.setArg(j++, ping_buf);
    nHop.setArg(j++, pong_buf);
    nHop.setArg(j++, result_buf);

    q.enqueueTask(nHop, &events_write, &events_kernel[0]);

    q.enqueueMigrateMemObjects(ob_out, 1, &events_kernel, &events_read[0]);
    q.finish();

    gettimeofday(&end_time, 0);
    std::cout << "kernel end------" << std::endl;

#else

    // nHop_kernel(numHop, numPairs, batchSize, pair, offsetTable, indexTable,

    //             offsetBuffer[0], indexBuffer[0], offsetBuffer[1], indexBuffer[1], offsetBuffer[2], indexBuffer[2],
    //             offsetBuffer[3], indexBuffer[3], offsetBuffer[4], indexBuffer[4], offsetBuffer[5], indexBuffer[5],
    //             offsetBuffer[6], indexBuffer[6], offsetBuffer[7], indexBuffer[7],

    //             zeroBuffer0, zeroBuffer1, result);

#endif

    std::fstream goldenfstream(goldenfile.c_str(), std::ios::in);
    if (!goldenfstream) {
        std::cout << "Error : " << goldenfile << " file doesn't exist !" << std::endl;
        exit(1);
    }

    std::unordered_map<unsigned long, float> goldenHashMap;
    while (goldenfstream.getline(line, sizeof(line))) {
        std::string str(line);
        std::replace(str.begin(), str.end(), ',', ' ');
        std::stringstream data(str.c_str());
        unsigned long golden_src;
        unsigned long golden_des;
        unsigned golden_res;
        data >> golden_src;
        data >> golden_des;
        data >> golden_res;
        unsigned long tmp = 0UL | golden_src << 32UL | golden_des;
        if (golden_res != 0) goldenHashMap.insert(std::pair<unsigned long, unsigned>(tmp, golden_res));
    }
    goldenfstream.close();

    std::unordered_map<unsigned long, float> resHashMap;
    for (int i = 1; i < result[0] + 1; i++) {
        unsigned long tmp_src = result[i].range(31, 0) + 1;
        unsigned long tmp_des = result[i].range(63, 32) + 1;
        unsigned long tmp_res = result[i].range(95, 64);
        unsigned long tmp = 0UL | tmp_src << 32UL | tmp_des;
        resHashMap.insert(std::pair<unsigned long, unsigned>(tmp, tmp_res));
    }

    if (resHashMap.size() != goldenHashMap.size()) std::cout << "miss pairs!" << std::endl;
    for (auto it = resHashMap.begin(); it != resHashMap.end(); it++) {
        unsigned long tmp_src = (it->first) / (1UL << 32UL);
        unsigned long tmp_des = (it->first) % (1UL << 32UL);
        unsigned long tmp_res = it->second;
        auto got = goldenHashMap.find(it->first);
        if (got == goldenHashMap.end()) {
            std::cout << "ERROR: pair not found! cnt_src: " << tmp_src << " cnt_des: " << tmp_des
                      << " cnt_res: " << tmp_res << std::endl;
            err++;
        } else if (got->second != it->second) {
            std::cout << "ERROR: incorrect count! golden_src: " << (got->first) / (1UL << 32UL)
                      << " golden_des: " << (got->first) % (1UL << 32UL) << " golden_res: " << (got->second)
                      << " cnt_src: " << tmp_src << " cnt_des: " << tmp_des << " cnt_res: " << tmp_res << std::endl;
            err++;
        }
    }

    if (err) {
        logger.error(xf::common::utils_sw::Logger::Message::TEST_FAIL);
    } else {
        logger.info(xf::common::utils_sw::Logger::Message::TEST_PASS);
    }

    return err;
}
