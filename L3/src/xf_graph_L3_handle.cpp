/*
 * Copyright 2020 Xilinx, Inc.
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

#include "xf_graph_L3_handle.hpp"

namespace xf {
namespace graph {
namespace L3 {

/*
void Handle::initOpSP(const char* kernelName,
                      char* xclbinFile,
                      char* kernelAlias,
                      unsigned int requestLoad,
                      unsigned int numDevices,
                      unsigned int cuPerBoard) {
    uint32_t* deviceID;
    uint32_t* cuID;
    xrm->fetchCuInfo(kernelName, kernelAlias, requestLoad, deviceNm, maxChannelSize, maxCU, &deviceID, &cuID);
    opsp->setHWInfo(deviceNm, maxCU);
    opsp->init((char*)kernelName, xclbinFile, deviceID, cuID, requestLoad);
    opsp->initThread(xrm, kernelName, kernelAlias, requestLoad, numDevices, cuPerBoard);
    delete[] cuID;
    delete[] deviceID;
};
*/

/*
void Handle::initOpTriangleCount(const char* kernelName,
                                 char* xclbinFile,
                                 char* kernelAlias,
                                 unsigned int requestLoad,
                                 unsigned int numDevices,
                                 unsigned int cuPerBoard) {
    uint32_t* deviceID;
    uint32_t* cuID;
    xrm->fetchCuInfo(kernelName, kernelAlias, requestLoad, deviceNm, maxChannelSize, maxCU, &deviceID, &cuID);
    optcount->setHWInfo(deviceNm, maxCU);
    optcount->init((char*)kernelName, xclbinFile, deviceID, cuID, requestLoad);
    optcount->initThread(xrm, kernelName, kernelAlias, requestLoad, numDevices, cuPerBoard);
    delete[] cuID;
    delete[] deviceID;
};
*/



/*
void Handle::initOpBFS(const char* kernelName,
                       char* xclbinFile,
                       char* kernelAlias,
                       unsigned int requestLoad,
                       unsigned int numDevices,
                       unsigned int cuPerBoard) {
    uint32_t* deviceID;
    uint32_t* cuID;
    xrm->fetchCuInfo(kernelName, kernelAlias, requestLoad, deviceNm, maxChannelSize, maxCU, &deviceID, &cuID);
    opbfs->setHWInfo(deviceNm, maxCU);
    opbfs->init((char*)kernelName, xclbinFile, deviceID, cuID, requestLoad);
    opbfs->initThread(xrm, kernelName, kernelAlias, requestLoad, numDevices, cuPerBoard);
    delete[] cuID;
    delete[] deviceID;
};
*/

/*
void Handle::initOpWCC(const char* kernelName,
                       char* xclbinFile,
                       char* kernelAlias,
                       unsigned int requestLoad,
                       unsigned int numDevices,
                       unsigned int cuPerBoard) {
    uint32_t* deviceID;
    uint32_t* cuID;
    xrm->fetchCuInfo(kernelName, kernelAlias, requestLoad, deviceNm, maxChannelSize, maxCU, &deviceID, &cuID);
    opwcc->setHWInfo(deviceNm, maxCU);
    opwcc->init((char*)kernelName, xclbinFile, deviceID, cuID, requestLoad);
    opwcc->initThread(xrm, kernelName, kernelAlias, requestLoad, numDevices, cuPerBoard);
    delete[] cuID;
    delete[] deviceID;
};
*/

/*
void Handle::initOpSCC(const char* kernelName,
                       char* xclbinFile,
                       char* kernelAlias,
                       unsigned int requestLoad,
                       unsigned int numDevices,
                       unsigned int cuPerBoard) {
    uint32_t* deviceID;
    uint32_t* cuID;
    xrm->fetchCuInfo(kernelName, kernelAlias, requestLoad, deviceNm, maxChannelSize, maxCU, &deviceID, &cuID);
    opscc->setHWInfo(deviceNm, maxCU);
    opscc->init((char*)kernelName, xclbinFile, deviceID, cuID, requestLoad);
    opscc->initThread(xrm, kernelName, kernelAlias, requestLoad, numDevices, cuPerBoard);
    delete[] cuID;
    delete[] deviceID;
};
*/


// currently used for cosine similarity
int32_t Handle::initOpSimDense(std::string kernelName,
                               std::string xclbinFile,
                               std::string kernelAlias,
                               unsigned int requestLoad,
                               unsigned int numDevices,
                               unsigned int cuPerBoard) {
    uint32_t* deviceID;
    uint32_t* cuID;
    int32_t status = 0;

    // fetchCuInfo will scan all available devices/CUs. 
    numDevices_ = numDevices; // set initial requested number of devices
    status = xrm->fetchCuInfo(kernelName.c_str(), kernelAlias.c_str(), requestLoad, numDevices_, 
                              maxChannelSize, maxCU_, &deviceID, &cuID);
    if (status < 0)
        return status;
    opsimdense->setHWInfo(numDevices_, maxCU_);
    opsimdense->init(xrm, kernelName, kernelAlias, xclbinFile, deviceID, cuID, requestLoad);
    opsimdense->initThread(xrm, kernelName, kernelAlias, requestLoad, numDevices, cuPerBoard);
    delete[] cuID;
    delete[] deviceID;

    return status;
};


#ifdef LOUVAINMOD
int32_t Handle::initOpLouvainModularity(std::string kernelName,
                                     std::string xclbinFile,
                                     std::string kernelAlias,
                                     unsigned int requestLoad,
                                     unsigned int numDevices,
                                     unsigned int cuPerBoard) {
    uint32_t* deviceID;
    uint32_t* cuID;
    int32_t status = 0;

    // fetchCuInfo will scan all available devices/CUs. 
    numDevices_ = numDevices; // set initial requested number of devices
    status = xrm->fetchCuInfo(kernelName.c_str(), kernelAlias.c_str(), requestLoad, numDevices_, 
                              maxChannelSize, maxCU_, &deviceID, &cuID);
    if (status < 0)
        return status;

    oplouvainmod->setHWInfo(numDevices_, maxCU_);
    oplouvainmod->init(xrm, kernelName, kernelAlias, xclbinFile, deviceID, cuID, 
                       requestLoad);
    oplouvainmod->initThread(xrm, kernelName, kernelAlias, requestLoad, numDevices, cuPerBoard);
    delete[] cuID;
    delete[] deviceID;

    return status;
};
#endif

void Handle::addOp(singleOP op) {
    ops.push_back(op);
}

int Handle::setUp() {
    getEnv();
    unsigned int opNm = ops.size();
    unsigned int deviceCounter = 0;
    int32_t status = 0;

    for (unsigned int i = 0; i < opNm; ++i) {
        if (ops[i].operationName == "shortestPathFloat") {
            /*unsigned int boardNm = ops[i].numDevices;
            if (deviceCounter + boardNm > numDevices) {
                std::cout << "Error: Need more devices" << std::endl;
                exit(1);
            }
            std::thread thUn[boardNm];
            for (int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            std::thread th[boardNm];
            for (int j = 0; j < boardNm; ++j) {
                th[j] = loadXclbinNonBlock(deviceCounter + j, ops[i].xclbinFile);
            }
            for (int j = 0; j < boardNm; ++j) {
                th[j].join();
            }
            deviceCounter += boardNm;
            initOpSP(ops[i].kernelName, ops[i].xclbinFile, ops[i].kernelAlias, 
                     ops[i].requestLoad, ops[i].numDevices,
                     ops[i].cuPerBoard);*/
        } else if (ops[i].operationName == "similarityDense") {
            // currently active
            unsigned int boardNm = ops[i].numDevices;
            if (deviceCounter + boardNm > totalDevices) {
                std::cout << "ERROR: Current node does not have requested device count." 
                    << "Requested: " << deviceCounter + boardNm 
                    << "Available: " << totalDevices << std::endl;
                return XF_GRAPH_L3_ERROR_NOT_ENOUGH_DEVICES;
            }
            std::thread thUn[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
#ifndef NDEBUG__
                std::cout << "DEBUG: " << __FUNCTION__ << ": xrm->unloadXclbinNonBlock " 
                          << deviceCounter + j << std::endl;
#endif
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            std::future<int> th[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
#ifndef NDEBUG__
                std::cout << "DEBUG: " << __FUNCTION__ << ": xrm->loadXclbinAsync " 
                          << deviceCounter + j 
                          << " ops[i].xclbinPath=" << ops[i].xclbinPath << std::endl;
#endif
                th[j] = loadXclbinAsync(deviceCounter + j, ops[i].xclbinPath);
            }
            for (unsigned int j = 0; j < boardNm; ++j) {
                auto loadedDevId = th[j].get();
                if (loadedDevId < 0) {
                        std::cout << "ERROR: failed to load " << ops[i].xclbinPath << 
                        "(Status=" << loadedDevId << "). Please check if it is " <<
                        "created for the Xilinx Acceleration card installed on " <<
                        "the server." << std::endl;
                    return loadedDevId;
                }
            }
            deviceCounter += boardNm;

            status = initOpSimDense(ops[i].kernelName.c_str(), ops[i].xclbinPath, 
                                    ops[i].kernelAlias.c_str(), ops[i].requestLoad,
                                    ops[i].numDevices, ops[i].cuPerBoard);
            if (status < 0)
                return XF_GRAPH_L3_ERROR_ALLOC_CU;
        } else if (ops[i].operationName == "triangleCount") {
            /*unsigned int boardNm = ops[i].numDevices;
            if (deviceCounter + boardNm > numDevices) {
                std::cout << "Error: Need more devices" << std::endl;
                exit(1);
            }
            std::thread thUn[boardNm];
            for (int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            std::thread th[boardNm];
            for (int j = 0; j < boardNm; ++j) {
                th[j] = loadXclbinNonBlock(deviceCounter + j, ops[i].xclbinFile);
            }
            for (int j = 0; j < boardNm; ++j) {
                th[j].join();
            }
            deviceCounter += boardNm;
            initOpTriangleCount(ops[i].kernelName, ops[i].xclbinFile, ops[i].kernelAlias, ops[i].requestLoad,
                                ops[i].numDevices, ops[i].cuPerBoard);*/
        } else if (ops[i].operationName == "WCC") {
            /*unsigned int boardNm = ops[i].numDevices;
            if (deviceCounter + boardNm > numDevices) {
                std::cout << "Error: Need more devices" << std::endl;
                exit(1);
            }
            std::thread thUn[boardNm];
            for (int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            std::thread th[boardNm];
            for (int j = 0; j < boardNm; ++j) {
                th[j] = loadXclbinNonBlock(deviceCounter + j, ops[i].xclbinFile);
            }
            for (int j = 0; j < boardNm; ++j) {
                th[j].join();
            }
            deviceCounter += boardNm;
            initOpWCC(ops[i].kernelName, ops[i].xclbinFile, ops[i].kernelAlias, ops[i].requestLoad, ops[i].numDevices,
                      ops[i].cuPerBoard);*/
        } 
#ifdef LOUVAINMOD
        if (ops[i].operationName == "louvainModularity") {
            unsigned int boardNm = ops[i].numDevices;
            if (deviceCounter + boardNm > totalDevices) {
                std::cout << "Error: Need more devices" << std::endl;
                exit(1);
            }
            std::thread thUn[boardNm];
            for (int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            std::thread th[boardNm];
            for (int j = 0; j < boardNm; ++j) {
                th[j] = loadXclbinNonBlock(deviceCounter + j, ops[i].xclbinPath);
            }
            for (int j = 0; j < boardNm; ++j) {
                th[j].join();
            }
            deviceCounter += boardNm;
            initOpLouvainModularity(ops[i].kernelName, ops[i].xclbinPath, ops[i].kernelAlias, ops[i].requestLoad,
                                    ops[i].numDevices, ops[i].cuPerBoard);
        }
#endif
        if (0) {
            std::cout << "Error: the operation " << ops[i].operationName << " is not supported" << std::endl;
            exit(1);
        }
    }
    return 0;
}

void Handle::getEnv() {
    cl_uint platformID = 0;
    cl_platform_id* platforms = NULL;
    char vendor_name[128] = {0};
    cl_uint num_platforms = 0;
    cl_int err2 = clGetPlatformIDs(0, NULL, &num_platforms);
    if (CL_SUCCESS != err2) {
        std::cout << "INFO: get platform failed" << std::endl;
    }
    platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
    if (NULL == platforms) {
        std::cout << "INFO: allocate platform failed" << std::endl;
    }
    err2 = clGetPlatformIDs(num_platforms, platforms, NULL);
    if (CL_SUCCESS != err2) {
        std::cout << "INFO: get platform failed" << std::endl;
    }
    for (cl_uint ui = 0; ui < num_platforms; ++ui) {
        err2 = clGetPlatformInfo(platforms[ui], CL_PLATFORM_VENDOR, 128 * sizeof(char), vendor_name, NULL);
        if (CL_SUCCESS != err2) {
            std::cout << "INFO: get platform failed" << std::endl;
        } else if (!std::strcmp(vendor_name, "Xilinx")) {
            platformID = ui;
        }
    }
    cl_device_id* devices;
    std::vector<cl::Device> devices0 = xcl::get_xil_devices();
    totalDevices = devices0.size();
    devices = (cl_device_id*)malloc(sizeof(cl_device_id) * totalDevices);
    err2 = clGetDeviceIDs(platforms[platformID], CL_DEVICE_TYPE_ALL, totalDevices, devices, NULL);
    size_t valueSize;
    char* value;
    for (uint32_t i = 0; i < totalDevices; ++i) {
        // print device name
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 0, NULL, &valueSize);
        value = new char[valueSize];
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, valueSize, value, NULL);
        std::cout << "INFO: " << __FUNCTION__ << ": Device " << i 
                  << ":" << value << std::endl;
        delete[] value;
    }
}

void Handle::showHandleInfo() {
#ifndef NDEBUG    
    std::cout << "INFO: " << __FUNCTION__ << " numDevices_=" << numDevices_ 
              << " maxCU_=" << maxCU_ << std::endl;
    unsigned int opNm = ops.size();
    for (unsigned int i = 0; i < opNm; ++i) {
        std::cout << "INFO: " << __FUNCTION__ << 
            " operationName=" << ops[i].operationName << 
            " kernelname=" << ops[i].kernelName << 
            " requestLoad=" << ops[i].requestLoad << 
            " xclbinFile=" << ops[i].xclbinPath << std::endl;
    }
#endif    
}

void Handle::free() {
    unsigned int opNm = ops.size();
    unsigned int deviceCounter = 0;
    for (unsigned int i = 0; i < opNm; ++i) {
        if (ops[i].operationName == "pagerank") {
            /*unsigned int boardNm = ops[i].numDevices;
            std::thread thUn[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            deviceCounter += boardNm;
            oppg->freePG();*/
        } else if (ops[i].operationName == "shortestPathFloat") {
            /*unsigned int boardNm = ops[i].numDevices;
            std::thread thUn[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            deviceCounter += boardNm;
            opsp->freeSP();*/
        } else if (ops[i].operationName == "similarityDense") {
            //-----------------------------------------------------------------
            opsimdense->freeSimDense(xrm->ctx);            
            unsigned int boardNm = ops[i].numDevices;
            std::thread thUn[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            deviceCounter += boardNm;
        } else if (ops[i].operationName == "similarityDenseInt") {
            /*unsigned int boardNm = ops[i].numDevices;
            std::thread thUn[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            deviceCounter += boardNm;
            opsimdense->freeSimDense(xrm->ctx);
            xrm->freeCuGroup(boardNm);*/
        } else if (ops[i].operationName == "similaritySparse") {
            /*unsigned int boardNm = ops[i].numDevices;
            std::thread thUn[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            deviceCounter += boardNm;
            opsimsparse->freeSimSparse();*/
        } else if (ops[i].operationName == "triangleCount") {
            /*unsigned int boardNm = ops[i].numDevices;
            std::thread thUn[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            deviceCounter += boardNm;
            optcount->freeTriangleCount();*/
        } else if (ops[i].operationName == "labelPropagation") {
            /*unsigned int boardNm = ops[i].numDevices;
            std::thread thUn[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            deviceCounter += boardNm;
            oplprop->freeLabelPropagation();*/
        } else if (ops[i].operationName == "BFS") {
            /*unsigned int boardNm = ops[i].numDevices;
            std::thread thUn[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            deviceCounter += boardNm;
            opbfs->freeBFS();*/
        } else if (ops[i].operationName == "WCC") {
            /*unsigned int boardNm = ops[i].numDevices;
            std::thread thUn[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            deviceCounter += boardNm;
            opwcc->freeWCC();*/
        } else if (ops[i].operationName == "SCC") {
            /*unsigned int boardNm = ops[i].numDevices;
            std::thread thUn[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            deviceCounter += boardNm;
            opscc->freeSCC();*/
        } else if (ops[i].operationName == "convertCsrCsc") {
            /*unsigned int boardNm = ops[i].numDevices;
            std::thread thUn[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            deviceCounter += boardNm;
            opconvertcsrcsc->freeConvertCsrCsc();*/
        }
    }
    //TODO: the following line crashes GPE.
    //xrm->freeXRM();
};

void Handle::loadXclbin(unsigned int deviceId, char* xclbinName) {
    xrm->loadXclbin(deviceId, xclbinName);
};

std::thread Handle::loadXclbinNonBlock(unsigned int deviceId, std::string& xclbinName) {
    return xrm->loadXclbinNonBlock(deviceId, xclbinName);
};

std::future<int> Handle::loadXclbinAsync(unsigned int deviceId, std::string& xclbinPath) {
    std::cout << "DEBUG: " << __FUNCTION__ << " xclbinPath=" << xclbinPath << std::endl;
    return xrm->loadXclbinAsync(deviceId, xclbinPath);
};

} // L3
} // graph
} // xf
