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

//#include "xf_utils_sw///logger.hpp"
#include "ap_int.h"
#include "utils.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <vector>
#include <stdlib.h>

#include <unordered_map>
#include <map>

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

unsigned oneHop(unsigned hop, unsigned des, unsigned next, unsigned* offset, unsigned* index) {
    if (hop == 0) {
        if (next == des)
            return 1;
        else
            return 0;
    } else {
        unsigned start = offset[next];
        unsigned end = offset[next + 1];
        unsigned cnt = 0;
        for (int j = start; j < end; j++) {
            cnt += oneHop(hop - 1, des, index[j], offset, index);
        }
        if (next == des)
            return cnt + 1;
        else
            return cnt;
    }
}

void referenceHop(unsigned hop,
                  unsigned* offset,
                  unsigned* index,
                  unsigned numPairs,
                  ap_uint<128>* pairs,
                  std::unordered_map<unsigned long, float>& goldenHashMap) {
    unsigned long golden_src;
    unsigned long golden_des;
    unsigned golden_res;
    for (int i = 0; i < numPairs; i++) {
        golden_src = pairs[i](31, 0);
        golden_des = pairs[i](63, 32);
        golden_res = 0;
        unsigned start = offset[golden_src];
        unsigned end = offset[golden_src + 1];
        for (int j = start; j < end; j++) {
            golden_res += oneHop(hop - 1, golden_des, index[j], offset, index);
            // if(golden_src==1261226 && golden_des==477321)
            //    std::cout<<index[j]<<std::endl;
        }
        unsigned long tmp = 0UL | (golden_src + 1) << 32UL | (golden_des + 1);
        if (golden_res != 0) {
            goldenHashMap.insert(std::pair<unsigned long, unsigned>(tmp, golden_res));
            // std::cout << "ref result: src=" << golden_src + 1 << " des=" << golden_des + 1 << " res=" << golden_res
            //          << std::endl;
        }
    }
}

int main(int argc, const char* argv[]) {
    std::cout << "\n---------------------N Hop-------------------\n";
    //xf::common::utils_sw:://logger //logger(std::cout, std::cerr);
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
        std::cout << "INFO: golden file path is not set!\n";
    }

    if (!parser.getCmdOption("--check", args)) {
        std::cout << "ERROR: check status is bot set!\n";
        return -1;
    }
    ap_uint<32> check = stoi(args);

    if (!parser.getCmdOption("--hop", args)) {
        std::cout << "ERROR: hop number is not set!\n";
        return -1;
    }
    ap_uint<32> numHop = stoi(args);

    if (!parser.getCmdOption("--batch", args)) {
        std::cout << "ERROR: batch size is not set!\n";
        return -1;
    }
    ap_uint<32> batchSize = stoi(args);

    if (!parser.getCmdOption("--bypass", args)) {
        std::cout << "ERROR: bypass is not set!\n";
        return -1;
    }
    ap_uint<32> byPass = stoi(args);

    if (!parser.getCmdOption("--duplicate", args)) {
        std::cout << "ERROR: duplicate is not set!\n";
        return -1;
    }
    ap_uint<32> duplicate = stoi(args);

    // -------------setup k0 params---------------
    int err = 0;

    char line[1024] = {0};
    int fileIdx = 0;

    int numVertices;
    int numEdges;
    int numPairs;

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
    indexfstream.close();

    fileIdx = 0;
    std::fstream pairfstream(pairfile.c_str(), std::ios::in);
    if (!pairfstream) {
        std::cout << "Error : " << pairfile << " file doesn't exist !" << std::endl;
        exit(1);
    }

    pairfstream.getline(line, sizeof(line));
    std::stringstream numPdata(line);
    numPdata >> numPairs;

    ap_uint<128>* pair = aligned_alloc<ap_uint<128> >(numPairs + 4096);

    while (pairfstream.getline(line, sizeof(line))) {
        std::stringstream data(line);
        ap_uint<128> tmp128;
        unsigned src;
        unsigned des;
        data >> src;
        tmp128.range(31, 0) = src - 1;
        data >> des;
        tmp128.range(63, 32) = des - 1;
        tmp128.range(127, 64) = 0;
        pair[fileIdx] = tmp128;
        fileIdx++;
    }
    pairfstream.close();

    // dispatch offset and index
    const int PU = 4;
    unsigned* offsetTable;
    offsetTable = aligned_alloc<unsigned>(1024);
    unsigned* indexTable;
    indexTable = aligned_alloc<unsigned>(1024);
    ap_uint<64>* cardTable;
    cardTable = aligned_alloc<ap_uint<64> >(1024);
    unsigned* offsetBuffer[PU];
    unsigned* indexBuffer[PU];

    if (duplicate == 0) {
        ap_uint<32> numVerticesPU = numVertices / PU;
        offsetTable[PU] = numVertices;
        for (int i = 0; i < PU; i++) {
            offsetTable[i] = numVerticesPU * i;
        }
        for (int i = 0; i < PU; i++) {
            offsetBuffer[i] = aligned_alloc<unsigned>(offsetTable[i + 1] - offsetTable[i] + 4096);
        }
        for (int i = 0; i < PU; i++) {
            for (int j = offsetTable[i]; j < offsetTable[i + 1] + 1; j++) {
                offsetBuffer[i][j - offsetTable[i]] = offset32[j];
                // std::cout << "i=" << i << " j=" << j - offsetTable[i] << " offset=" << offset32[j] << std::endl;
            }
        }

        for (int i = 0; i < PU + 1; i++) {
            indexTable[i] = offset32[offsetTable[i]];
        }
        for (int i = 0; i < PU + 1; i++) {
            std::cout << "id=" << i << " offsetTable=" << offsetTable[i] << " indexTable=" << indexTable[i]
                      << std::endl;
        }
        for (int i = 0; i < PU; i++) {
            ap_uint<32> numEdgesPU = indexTable[i + 1] - indexTable[i];
            std::cout << "numEgdesPU=" << numEdgesPU << std::endl;
            indexBuffer[i] = aligned_alloc<unsigned>(numEdgesPU + 4096);
            for (int j = indexTable[i]; j < indexTable[i + 1]; j++) {
                indexBuffer[i][j - indexTable[i]] = index32[j];
                // std::cout << "i=" << i << " j=" << j - indexTable[i] << " index=" << index32[j] << std::endl;
            }
        }
    } else {
        offsetTable[PU] = numVertices;
        for (int i = 0; i < PU; i++) {
            offsetTable[i] = numVertices;
            offsetBuffer[i] = aligned_alloc<unsigned>(numVertices + 4096);
        }
        for (int i = 0; i < PU; i++) {
            for (int j = 0; j < numVertices + 1; j++) {
                offsetBuffer[i][j] = offset32[j];
            }
        }

        for (int i = 0; i < PU + 1; i++) {
            indexTable[i] = offset32[offsetTable[i]];
        }
        for (int i = 0; i < PU + 1; i++) {
            std::cout << "id=" << i << " offsetTable=" << offsetTable[i] << " indexTable=" << indexTable[i]
                      << std::endl;
        }
        for (int i = 0; i < PU; i++) {
            indexBuffer[i] = aligned_alloc<unsigned>(numEdges + 4096);
            for (int j = 0; j < numEdges; j++) {
                indexBuffer[i][j] = index32[j];
                // std::cout << "i=" << i << " j=" << j - indexTable[i] << " index=" << index32[j] << std::endl;
            }
        }
    }
    offsetTable[PU + 1] = 0;
    offsetTable[PU + 2] = numVertices;

    for (int i = 0; i < 32; i++) {
        ap_uint<32> id = i;
        ap_uint<32> tmp = numVertices * i;
        cardTable[i] = (tmp, id);
    }

    // initilaize buffer and config
    unsigned* numOut = aligned_alloc<unsigned>(1024);
    ap_uint<512>* local = aligned_alloc<ap_uint<512> >(3 << 20);
    memset(local, 0, sizeof(ap_uint<512>) * (3 << 20));
    ap_uint<512>* netSwitch = aligned_alloc<ap_uint<512> >(3 << 20);
    memset(netSwitch, 0, sizeof(ap_uint<512>) * (3 << 20));
    ap_uint<512>* zeroBuffer0 = aligned_alloc<ap_uint<512> >(4 << 20);
    memset(zeroBuffer0, 0, sizeof(ap_uint<512>) * (4 << 20));
    ap_uint<512>* zeroBuffer1 = aligned_alloc<ap_uint<512> >(4 << 20);
    memset(zeroBuffer1, 0, sizeof(ap_uint<512>) * (4 << 20));

    // do pre-process on CPU
    struct timeval start_time, end_time;

#ifndef HLS_TEST
    // platform related operations
    std::vector<cl::Device> devices = xcl::get_xil_devices();
    cl::Device device = devices[1];

    // Creating Context and Command Queue for selected Device
    cl::Context context(device, NULL, NULL, NULL, &fail);
    //logger.logCreateContext(fail);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &fail);
    //logger.logCreateCommandQueue(fail);
    std::string devName = device.getInfo<CL_DEVICE_NAME>();
    printf("Found Device=%s\n", devName.c_str());

    cl::Program::Binaries xclBins = xcl::import_binary_file(xclbin_path);
    devices.resize(1);
    devices[0] = device;
    cl::Program program(context, devices, xclBins, NULL, &fail);
    //logger.logCreateProgram(fail);
    cl::Kernel nHop;
    nHop = cl::Kernel(program, "nHop_kernel", &fail);
    //logger.logCreateKernel(fail);
    std::cout << "kernel has been created" << std::endl;

    std::vector<cl_mem_ext_ptr_t> mext_o(17);

    mext_o[0] = {(unsigned int)(0) | XCL_MEM_TOPOLOGY, pair, 0};

    mext_o[1] = {(unsigned int)(2) | XCL_MEM_TOPOLOGY, offsetBuffer[0], 0};
    mext_o[2] = {(unsigned int)(3) | XCL_MEM_TOPOLOGY, indexBuffer[0], 0};
    mext_o[3] = {(unsigned int)(4) | XCL_MEM_TOPOLOGY, offsetBuffer[1], 0};
    mext_o[4] = {(unsigned int)(5) | XCL_MEM_TOPOLOGY, indexBuffer[1], 0};
    mext_o[5] = {(unsigned int)(6) | XCL_MEM_TOPOLOGY, offsetBuffer[2], 0};
    mext_o[6] = {(unsigned int)(7) | XCL_MEM_TOPOLOGY, indexBuffer[2], 0};
    mext_o[7] = {(unsigned int)(8) | XCL_MEM_TOPOLOGY, offsetBuffer[3], 0};
    mext_o[8] = {(unsigned int)(9) | XCL_MEM_TOPOLOGY, indexBuffer[3], 0};

    mext_o[9] = {(unsigned int)(10) | XCL_MEM_TOPOLOGY, zeroBuffer0, 0};
    mext_o[10] = {(unsigned int)(11) | XCL_MEM_TOPOLOGY, zeroBuffer1, 0};

    mext_o[11] = {(unsigned int)(12) | XCL_MEM_TOPOLOGY, offsetTable, 0};
    mext_o[12] = {(unsigned int)(12) | XCL_MEM_TOPOLOGY, indexTable, 0};
    mext_o[13] = {(unsigned int)(12) | XCL_MEM_TOPOLOGY, cardTable, 0};
    mext_o[14] = {(unsigned int)(12) | XCL_MEM_TOPOLOGY, local, 0};
    mext_o[15] = {(unsigned int)(14) | XCL_MEM_TOPOLOGY, netSwitch, 0};
    mext_o[16] = {(unsigned int)(12) | XCL_MEM_TOPOLOGY, numOut, 0};

    // create device buffer and map dev buf to host buf
    cl::Buffer pair_buf, local_buf, switch_buf, ping_buf, pong_buf, offset_table, index_table, card_table, num_out;
    cl::Buffer offset_buf[PU];
    cl::Buffer index_buf[PU];

    pair_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                          sizeof(ap_uint<128>) * (numPairs + 4096), &mext_o[0]);
    ping_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                          sizeof(ap_uint<512>) * (4 << 20), &mext_o[9]);
    pong_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                          sizeof(ap_uint<512>) * (4 << 20), &mext_o[10]);
    offset_table = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                              sizeof(unsigned) * (1024), &mext_o[11]);
    index_table = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                             sizeof(unsigned) * (1024), &mext_o[12]);
    card_table = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                            sizeof(ap_uint<64>) * (1024), &mext_o[13]);
    local_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                           sizeof(ap_uint<512>) * (3 << 20), &mext_o[14]);
    switch_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                            sizeof(ap_uint<512>) * (3 << 20), &mext_o[15]);
    num_out = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                         sizeof(unsigned) * (1024), &mext_o[16]);

    if (duplicate == 0) {
        for (int i = 0; i < PU; i++) {
            offset_buf[i] =
                cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                           sizeof(unsigned) * (offsetTable[i + 1] - offsetTable[i] + 4096), &mext_o[1 + 2 * i]);
            index_buf[i] =
                cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                           sizeof(unsigned) * (indexTable[i + 1] - indexTable[i] + 4096), &mext_o[2 + 2 * i]);
        }
    } else {
        for (int i = 0; i < PU; i++) {
            offset_buf[i] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                       sizeof(unsigned) * (numVertices + 4096), &mext_o[1 + 2 * i]);
            index_buf[i] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                      sizeof(unsigned) * (numEdges + 4096), &mext_o[2 + 2 * i]);
        }
    }

    std::vector<cl::Memory> init;
    init.push_back(pair_buf);
    init.push_back(local_buf);
    init.push_back(switch_buf);
    init.push_back(num_out);
    init.push_back(offset_table);
    init.push_back(index_table);
    init.push_back(card_table);
    init.push_back(ping_buf);
    init.push_back(pong_buf);
    for (int i = 0; i < PU; i++) {
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
    ob_in.push_back(card_table);
    for (int i = 0; i < PU; i++) {
        ob_in.push_back(offset_buf[i]);
        ob_in.push_back(index_buf[i]);
    }
    q.enqueueMigrateMemObjects(ob_in, 0, nullptr, &events_write[0]);

    ob_out.push_back(num_out);
    ob_out.push_back(local_buf);
    ob_out.push_back(switch_buf);
    // launch kernel and calculate kernel execution time
    std::cout << "kernel start------" << std::endl;
    gettimeofday(&start_time, 0);

    int j = 0;

    nHop.setArg(j++, numHop);
    nHop.setArg(j++, 0);
    nHop.setArg(j++, numPairs);
    nHop.setArg(j++, batchSize);
    nHop.setArg(j++, 65536);
    nHop.setArg(j++, 16384);
    nHop.setArg(j++, byPass);
    nHop.setArg(j++, duplicate);
    nHop.setArg(j++, pair_buf);

    nHop.setArg(j++, offset_table);
    nHop.setArg(j++, index_table);
    nHop.setArg(j++, card_table);
    for (int i = 0; i < PU; i++) {
        nHop.setArg(j++, offset_buf[i]);
        nHop.setArg(j++, index_buf[i]);
    }

    nHop.setArg(j++, ping_buf);
    nHop.setArg(j++, pong_buf);

    nHop.setArg(j++, num_out);
    nHop.setArg(j++, local_buf);
    nHop.setArg(j++, switch_buf);

    q.enqueueTask(nHop, &events_write, &events_kernel[0]);

    q.enqueueMigrateMemObjects(ob_out, 1, &events_kernel, &events_read[0]);
    q.finish();

    gettimeofday(&end_time, 0);
    std::cout << "kernel end------" << std::endl;

#else

    nHop_kernel(numHop, 0, numPairs, batchSize, 65536, 32768, byPass, duplicate, (ap_uint<512>*)pair, offsetTable,
                indexTable, cardTable,

                offsetBuffer[0], (ap_uint<128>*)indexBuffer[0], offsetBuffer[1], (ap_uint<128>*)indexBuffer[1],
                offsetBuffer[2], (ap_uint<128>*)indexBuffer[2], offsetBuffer[3], (ap_uint<128>*)indexBuffer[3],

                zeroBuffer0, zeroBuffer1,

                numOut, local, netSwitch);

#endif

    std::cout << "numLocal=" << numOut[0] << " numSwitch=" << numOut[1] << std::endl;
    /*
        std::map<unsigned long, float> sortMap;
        for (int i = 0; i < numOut[0]; i++) {
            for (int j = 0; j < 4; j++) {
                unsigned long tmp_src = local[i].range(128 * j + 31, 128 * j);
                unsigned long tmp_des = local[i].range(128 * j + 63, 128 * j + 32);
                unsigned long tmp_res = local[i].range(128 * j + 95, 128 * j + 64);
                unsigned long tmp = 0UL | (tmp_src + 1) << 32UL | (tmp_des + 1);
                if ((tmp_src != 0) && (tmp_des != 0)) sortMap.insert(std::pair<unsigned long, unsigned>(tmp, tmp_res));
            }
        }
        for (auto it = sortMap.begin(); it != sortMap.end(); it++) {
            unsigned long tmp_src = (it->first) / (1UL << 32UL);
            unsigned long tmp_des = (it->first) % (1UL << 32UL);
            unsigned long tmp_res = it->second;
            std::cout << "kernel result: src=" << tmp_src << " des=" << tmp_des << " res=" << tmp_res << std::endl;
        }
    */

    if(check != 0) {
        std::unordered_map<unsigned long, float> goldenHashMap;
        std::fstream goldenfstream(goldenfile.c_str(), std::ios::in);
        if (!goldenfstream) {
            std::cout << "INFO: generating golden" << std::endl;
            referenceHop(numHop, offset32, index32, numPairs, pair, goldenHashMap);
        } else {
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
        }

        std::unordered_map<unsigned long, float> resHashMap;
        for (int i = 0; i < numOut[0]; i++) {
            for (int j = 0; j < 4; j++) {
                unsigned long tmp_src = local[i].range(128 * j + 31, 128 * j);
                unsigned long tmp_des = local[i].range(128 * j + 63, 128 * j + 32);
                unsigned long tmp_res = local[i].range(128 * j + 95, 128 * j + 64);
                unsigned long tmp = 0UL | (tmp_src + 1) << 32UL | (tmp_des + 1);
                if (tmp_res != 0) {
                    resHashMap.insert(std::pair<unsigned long, unsigned>(tmp, tmp_res));
                }

                // std::cout << "kernel result: src=" << tmp_src << " des=" << tmp_des << " res=" << tmp_res <<
                // std::endl;
            }
        }

        if (resHashMap.size() != goldenHashMap.size()) {
            std::cout << "miss pairs! number of kernel result=" << resHashMap.size()
                      << " number of golden=" << goldenHashMap.size() << std::endl;

            for (auto it = goldenHashMap.begin(); it != goldenHashMap.end(); it++) {
                unsigned long tmp_src = (it->first) / (1UL << 32UL);
                unsigned long tmp_des = (it->first) % (1UL << 32UL);
                unsigned long tmp_res = it->second;
                auto got = resHashMap.find(it->first);
                if (got == resHashMap.end()) {
                    std::cout << "ERROR: pair not found! golden_src: " << tmp_src << " golden_des: " << tmp_des
                              << " golden_res: " << tmp_res << std::endl;
                    err++;
                } else if (got->second != it->second) {
                    std::cout << "ERROR: incorrect count! tmp_src: " << (got->first) / (1UL << 32UL)
                              << " tmp_des: " << (got->first) % (1UL << 32UL) << " tmp_res: " << (got->second)
                              << " golden_src: " << tmp_src << " golden_des: " << tmp_des << " golden_res: " << tmp_res
                              << std::endl;
                    err++;
                }
            }
        } else {
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
                              << " cnt_src: " << tmp_src << " cnt_des: " << tmp_des << " cnt_res: " << tmp_res
                              << std::endl;
                    err++;
                }
            }
        }
    }

    if (err) {
        //logger.error(xf::common::utils_sw:://logger::Message::TEST_FAIL);
    } else {
        //logger.info(xf::common::utils_sw:://logger::Message::TEST_PASS);
    }
    return 0;
}
