#ifndef _HOST_HPP_
#define _HOST_HPP_

#include "nHopPartition.h"

#ifndef HLS_TEST

void nHop_L2_host(
                    unsigned* config,
                    ap_uint<512>* pair,

                    unsigned* offsetTable,
                    unsigned* indexTable,
                    ap_uint<64>* cardTable,
                    unsigned** offsetBuffer,
                    ap_uint<128>** indexBuffer,

                    ap_uint<512>* zeroBuffer0,
                    ap_uint<512>* zeroBuffer1,
                    ap_uint<512>* zeroBuffer2,

                    unsigned* numOut,
                    ap_uint<512>* local,
                    ap_uint<512>* netSwitch,
                    
                    long numVertices,
                    long numEdges,
                    int pu,
                    timeInfo* p_timeInfo,
                    std::string xclbin_path
) {

    unsigned numPairs = config[2];
    unsigned duplicate = config[7];

    cl_int fail;
    // platform related operations
    std::vector<cl::Device> devices = xcl::get_xil_devices();
    cl::Device device = devices[0];

    // Creating Context and Command Queue for selected Device
    cl::Context context(device, NULL, NULL, NULL, &fail);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &fail);
    std::string devName = device.getInfo<CL_DEVICE_NAME>();
    printf("Found Device=%s\n", devName.c_str());

    cl::Program::Binaries xclBins = xcl::import_binary_file(xclbin_path);
    devices.resize(1);
    devices[0] = device;
    cl::Program program(context, devices, xclBins, NULL, &fail);
    cl::Kernel nHop;
    nHop = cl::Kernel(program, "nHop_kernel", &fail);
    std::cout << "kernel has been created" << std::endl;

    std::vector<cl_mem_ext_ptr_t> mext_o(19);

    mext_o[0] = {(unsigned int)(0) | XCL_MEM_TOPOLOGY, pair, 0};
#if (pu==8)
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
    mext_o[18] = {(unsigned int)(27) | XCL_MEM_TOPOLOGY, zeroBuffer1, 0};

    mext_o[19] = {(unsigned int)(28) | XCL_MEM_TOPOLOGY, offsetTable, 0};
    mext_o[20] = {(unsigned int)(28) | XCL_MEM_TOPOLOGY, indexTable, 0};
    mext_o[21] = {(unsigned int)(28) | XCL_MEM_TOPOLOGY, cardTable, 0};
    mext_o[22] = {(unsigned int)(28) | XCL_MEM_TOPOLOGY, local, 0};
    mext_o[23] = {(unsigned int)(29) | XCL_MEM_TOPOLOGY, netSwitch, 0};
    mext_o[24] = {(unsigned int)(28) | XCL_MEM_TOPOLOGY, numOut, 0};
#else
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
    mext_o[11] = {(unsigned int)(12) | XCL_MEM_TOPOLOGY, zeroBuffer2, 0};

    mext_o[12] = {(unsigned int)(0) | XCL_MEM_TOPOLOGY, config, 0};
    mext_o[13] = {(unsigned int)(0) | XCL_MEM_TOPOLOGY, offsetTable, 0};
    mext_o[14] = {(unsigned int)(0) | XCL_MEM_TOPOLOGY, indexTable, 0};
    mext_o[15] = {(unsigned int)(0) | XCL_MEM_TOPOLOGY, cardTable, 0};
    mext_o[16] = {(unsigned int)(1) | XCL_MEM_TOPOLOGY, numOut, 0};
    mext_o[17] = {(unsigned int)(1) | XCL_MEM_TOPOLOGY, local, 0};
    mext_o[18] = {(unsigned int)(14) | XCL_MEM_TOPOLOGY, netSwitch, 0};
#endif
    // create device buffer and map dev buf to host buf
    cl::Buffer config_buf, pair_buf, local_buf, switch_buf, ping_buf, pong_buf, fifo_buf, offset_table, index_table,
        card_table, num_out;
    cl::Buffer offset_buf[pu];
    cl::Buffer index_buf[pu];
#if (pu==8)
    pair_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                          sizeof(ap_uint<128>) * (numPairs + 4096), &mext_o[0]);
    ping_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                          sizeof(ap_uint<512>) * (4 << 20), &mext_o[17]);
    pong_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                          sizeof(ap_uint<512>) * (4 << 20), &mext_o[18]);
    offset_table = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                              sizeof(unsigned) * (1024), &mext_o[19]);
    index_table = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                             sizeof(unsigned) * (1024), &mext_o[20]);
    card_table = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                            sizeof(ap_uint<64>) * (1024), &mext_o[21]);
    local_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                           sizeof(ap_uint<512>) * (3 << 20), &mext_o[22]);
    switch_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                            sizeof(ap_uint<512>) * (3 << 20), &mext_o[23]);
    num_out = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                         sizeof(unsigned) * (1024), &mext_o[24]);
#else
    pair_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                          sizeof(ap_uint<128>) * (numPairs + 4096), &mext_o[0]);

    ping_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                          sizeof(ap_uint<512>) * (4 << 20), &mext_o[9]);
    pong_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                          sizeof(ap_uint<512>) * (4 << 20), &mext_o[10]);

    fifo_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                          sizeof(ap_uint<512>) * (4 << 20), &mext_o[11]);

    config_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                            sizeof(unsigned) * (1024), &mext_o[12]);
    offset_table = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                              sizeof(unsigned) * (1024), &mext_o[13]);
    index_table = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                             sizeof(unsigned) * (1024), &mext_o[14]);
    card_table = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                            sizeof(ap_uint<64>) * (1024), &mext_o[15]);

    num_out = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                         sizeof(unsigned) * (1024), &mext_o[16]);
    local_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                           sizeof(ap_uint<512>) * (3 << 20), &mext_o[17]);
    switch_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                            sizeof(ap_uint<512>) * (3 << 20), &mext_o[18]);
#endif

    if (duplicate == 0) {
        for (int i = 0; i < pu; i++) {
            offset_buf[i] =
                cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                           sizeof(unsigned) * (offsetTable[i + 1] - offsetTable[i] + 4096), &mext_o[1 + 2 * i]);
            index_buf[i] =
                cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                           sizeof(unsigned) * (indexTable[i + 1] - indexTable[i] + 4096), &mext_o[2 + 2 * i]);
        }
    } else {
        for (int i = 0; i < pu; i++) {
            offset_buf[i] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                       sizeof(unsigned) * (numVertices + 4096), &mext_o[1 + 2 * i]);
            index_buf[i] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                      sizeof(unsigned) * (numEdges + 4096), &mext_o[2 + 2 * i]);
        }
    }

    std::vector<cl::Memory> init;
    init.push_back(config_buf);
    init.push_back(pair_buf);
    init.push_back(local_buf);
    init.push_back(switch_buf);
    init.push_back(num_out);
    init.push_back(offset_table);
    init.push_back(index_table);
    init.push_back(card_table);
    init.push_back(fifo_buf);
    init.push_back(ping_buf);
    init.push_back(pong_buf);
    for (int i = 0; i < pu; i++) {
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
    ob_in.push_back(config_buf);
    ob_in.push_back(offset_table);
    ob_in.push_back(index_table);
    ob_in.push_back(card_table);
    for (int i = 0; i < pu; i++) {
        ob_in.push_back(offset_buf[i]);
        ob_in.push_back(index_buf[i]);
    }
    q.enqueueMigrateMemObjects(ob_in, 0, nullptr, &events_write[0]);

    ob_out.push_back(num_out);
    ob_out.push_back(local_buf);
    ob_out.push_back(switch_buf);
    // launch kernel and calculate kernel execution time
    std::cout << "kernel start------" << std::endl;
    //gettimeofday(&start_time, 0);
    TimePointType h_compute_start = chrono::high_resolution_clock::now();
    TimePointType h_compute_end;

    int j = 0;

    nHop.setArg(j++, config_buf);
    nHop.setArg(j++, pair_buf);
    nHop.setArg(j++, offset_table);
    nHop.setArg(j++, index_table);
    nHop.setArg(j++, card_table);
    for (int i = 0; i < pu; i++) {
        nHop.setArg(j++, offset_buf[i]);
        nHop.setArg(j++, index_buf[i]);
    }
    nHop.setArg(j++, ping_buf);
    nHop.setArg(j++, pong_buf);
    nHop.setArg(j++, fifo_buf);
    nHop.setArg(j++, fifo_buf);
    nHop.setArg(j++, num_out);
    nHop.setArg(j++, local_buf);
    nHop.setArg(j++, switch_buf);

    q.enqueueTask(nHop, &events_write, &events_kernel[0]);
    q.enqueueMigrateMemObjects(ob_out, 1, &events_kernel, &events_read[0]);
    q.finish();

    //gettimeofday(&end_time, 0);
    double timeWrkCompute;
    getDiffTime(h_compute_start, h_compute_end, timeWrkCompute);
    p_timeInfo->timeWrkCompute += timeWrkCompute;
    std::cout << "kernel end------" << std::endl;
    // get related times
    unsigned long timeStart, timeEnd, exec_time, write_time, read_time;
    std::cout << "-------------------------------------------------------" << std::endl;
    events_write[0].getProfilingInfo(CL_PROFILING_COMMAND_START, &timeStart);
    events_write[0].getProfilingInfo(CL_PROFILING_COMMAND_END, &timeEnd);
    write_time = (timeEnd - timeStart) / 1000.0;
    std::cout << "INFO: Data transfer from host to device: " << write_time << " us\n";
    std::cout << "-------------------------------------------------------" << std::endl;
    events_read[0].getProfilingInfo(CL_PROFILING_COMMAND_START, &timeStart);
    events_read[0].getProfilingInfo(CL_PROFILING_COMMAND_END, &timeEnd);
    read_time = (timeEnd - timeStart) / 1000.0;
    std::cout << "INFO: Data transfer from device to host: " << read_time << " us\n";
    std::cout << "-------------------------------------------------------" << std::endl;
    events_kernel[0].getProfilingInfo(CL_PROFILING_COMMAND_START, &timeStart);
    events_kernel[0].getProfilingInfo(CL_PROFILING_COMMAND_END, &timeEnd);
    exec_time = (timeEnd - timeStart) / 1000.0;
    std::cout << "INFO: Average kernel execution per run: " << exec_time << " us\n";
    p_timeInfo->timeKernel += exec_time / 1000000.0;
    std::cout << "-------------------------------------------------------" << std::endl;

}

#endif
#endif