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

// currently used for cosine similarity
int32_t Handle::initOpSimDense(std::string xclbinFile,
                               std::string kernelName,
                               std::string kernelAlias,
                               unsigned int requestLoad,
                               unsigned int numDevices,
                               unsigned int cuPerBoard) 
{
#ifndef NDEBUG
    std::cout << "DEBUG: initOpSimDense " 
              << "\n    xclbinFile=" << xclbinFile    
              << "\n    kernelName=" << kernelName
              << "\n    kernelAlias=" << kernelAlias
              << "\n    requestLoad=" << requestLoad 
              << "\n    numDevices=" << numDevices
              << "\n    cuPerBoard=" << cuPerBoard << std::endl;
#endif
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
    // init allocates CUs based on kernelName and kernelAlias
    opsimdense->init(xrm, kernelName, kernelAlias, xclbinFile, deviceID, cuID, requestLoad);
    opsimdense->initThread(xrm, kernelName, kernelAlias, requestLoad, numDevices, cuPerBoard);
    delete[] cuID;
    delete[] deviceID;

    return status;
};


#ifdef LOUVAINMOD
int32_t Handle::initOpLouvainModularity(std::string xclbinFile, std::string kernelName,
                                        std::string kernelAlias, unsigned int requestLoad,
                                        unsigned int numDevices, unsigned int cuPerBoard) 
{
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
    oplouvainmod->init(xrm, kernelName, kernelAlias, xclbinFile, deviceID, cuID, requestLoad);
    oplouvainmod->initThread(xrm, kernelName, kernelAlias, requestLoad, numDevices, cuPerBoard);
    delete[] cuID;
    delete[] deviceID;

    return status;
};
#endif

void Handle::addOp(singleOP op) {
    ops.push_back(op);
}

int Handle::setUp(std::string deviceNames)
{
    const std::string delimiters(" ");
    for (int i = deviceNames.find_first_not_of(delimiters, 0); i != std::string::npos;
            i = deviceNames.find_first_not_of(delimiters, i)) {
        auto tokenEnd = deviceNames.find_first_of(delimiters, i);
        if (tokenEnd == std::string::npos)
            tokenEnd = deviceNames.size();
        const std::string token = deviceNames.substr(i, tokenEnd - i);
        supportedDeviceNames_.push_back(token);
        i = tokenEnd;
    }
    getEnv();

    unsigned int opNm = ops.size();
    unsigned int deviceCounter = 0;
    int32_t status = 0;

    for (unsigned int i = 0; i < opNm; ++i) {
        if (ops[i].operationName == "similarityDense") {
            unsigned int boardNm = ops[i].numDevices;
            if (deviceCounter + boardNm > totalSupportedDevices_) {
                std::cout << "ERROR: Current node does not have requested device count." 
                    << "Requested: " << deviceCounter + boardNm 
                    << "Available: " << totalSupportedDevices_ << std::endl;
                return XF_GRAPH_L3_ERROR_NOT_ENOUGH_DEVICES;
            }
            std::thread thUn[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
#ifndef NDEBUG
                std::cout << "DEBUG: " << __FUNCTION__ << ": xrm->unloadXclbinNonBlock " 
                          << supportedDeviceIds_[j] << std::endl;
#endif
                thUn[j] = xrm->unloadXclbinNonBlock(supportedDeviceIds_[j]);
            }
            for (unsigned int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            std::future<int> th[boardNm];
            for (unsigned int j = 0; j < boardNm; ++j) {
#ifndef NDEBUG
                std::cout << "DEBUG: " << __FUNCTION__ << ": xrm->loadXclbinAsync " 
                          << "\n    devId=" << supportedDeviceIds_[j]
                          << "\n    ops[i].xclbinPath=" << ops[i].xclbinPath << std::endl;
#endif
                th[j] = loadXclbinAsync(supportedDeviceIds_[j], ops[i].xclbinPath);
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

            status = initOpSimDense(ops[i].xclbinPath, ops[i].kernelName_, ops[i].kernelAlias_, 
                                    ops[i].requestLoad, ops[i].numDevices, ops[i].cuPerBoard);
            if (status < 0)
                return XF_GRAPH_L3_ERROR_ALLOC_CU;
        } 

#ifdef LOUVAINMOD
        if (ops[i].operationName == "louvainModularity") {
            unsigned int boardNm = ops[i].numDevices;
            if (deviceCounter + boardNm > totalSupportedDevices_) {
                std::cout << "ERROR: Current node does not have requested device count." 
                    << " Requested: " << deviceCounter + boardNm 
                    << " Available: " << totalSupportedDevices_ << std::endl;
                return XF_GRAPH_L3_ERROR_NOT_ENOUGH_DEVICES;
            }

            // Unload existing xclbin first if present
            std::thread thUn[boardNm];
            for (int j = 0; j < boardNm; ++j) {
#ifndef NDEBUG
                std::cout << "DEBUG: " << "xrm->unloadXclbinNonBlock devId=" << supportedDeviceIds_[j] << std::endl;                
#endif
                thUn[j] = xrm->unloadXclbinNonBlock(supportedDeviceIds_[j]);\
            }
            for (int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }

            // load xclbin asynchronously (i.e. non-blocking) using thread
            std::future<int> th[boardNm];

            for (int j = 0; j < boardNm; ++j) {
#ifndef NDEBUG
                std::cout << "DEBUG: " << __FUNCTION__ << ": xrm->loadXclbinAsync " 
                          << "\n    devId=" << supportedDeviceIds_[j]
                          << "\n    ops[i].xclbinPath=" << ops[i].xclbinPath << std::endl;
#endif
                th[j] = loadXclbinAsync(supportedDeviceIds_[j], ops[i].xclbinPath);
            }

            // wait for thread to finish
            for (int j = 0; j < boardNm; ++j) {
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
            status = initOpLouvainModularity(ops[i].xclbinPath, ops[i].kernelName_,  
                                             ops[i].kernelAlias_, ops[i].requestLoad,
                                             ops[i].numDevices, ops[i].cuPerBoard);
            if (status < 0)
                return XF_GRAPH_L3_ERROR_ALLOC_CU;
        }
#endif
        if (0) {
            std::cout << "Error: the operation " << ops[i].operationName << " is not supported" << std::endl;
            exit(1);
        }
    }
    return 0;

}


int Handle::setUp() 
{
    std::string deviceNames = "xilinx_u50_gen3x16_xdma_201920_3";
    return setUp(deviceNames);
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
    uint32_t totalXilinxDevices = devices0.size();
    totalSupportedDevices_ = 0;
    devices = (cl_device_id*)malloc(sizeof(cl_device_id) * totalXilinxDevices);
    err2 = clGetDeviceIDs(platforms[platformID], CL_DEVICE_TYPE_ALL, totalXilinxDevices, devices, NULL);
    size_t valueSize;
    char* value;

    for (uint32_t i = 0; i < totalXilinxDevices; ++i) {
        // print device name
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 0, NULL, &valueSize);
        value = new char[valueSize];
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, valueSize, value, NULL);
        if (std::find(supportedDeviceNames_.begin(), supportedDeviceNames_.end(), value) != supportedDeviceNames_.end()) {
            std::cout << "INFO: Found requested device: " << value << " ID=" << i << std::endl;            
            supportedDeviceIds_[totalSupportedDevices_++] = i;  // save curret supported supported devices
        } else {
            std::cout << "INFO: Skipped non-requested device: " << value << " ID=" << i << std::endl;
        }
        delete[] value;
    }
    std::cout << "INFO: Total matching devices: " << totalSupportedDevices_ << std::endl; 
}

void Handle::showHandleInfo() {
#ifndef NDEBUG    
    std::cout << "INFO: " << __FUNCTION__ << " numDevices_=" << numDevices_ 
              << " maxCU_=" << maxCU_ << std::endl;
    unsigned int opNm = ops.size();
    for (unsigned int i = 0; i < opNm; ++i) {
        std::cout << "INFO: " << __FUNCTION__ << 
            "\n    operationName=" << ops[i].operationName << 
            "\n    kernelName=" << ops[i].kernelName_ << 
            "\n    kernelAlias=" << ops[i].kernelAlias_ << 
            "\n    requestLoad=" << ops[i].requestLoad << 
            "\n    xclbinFile=" << ops[i].xclbinPath << std::endl;
    }
#endif    
}

void Handle::free() {
    unsigned int opNm = ops.size();
    unsigned int deviceCounter = 0;
    for (unsigned int i = 0; i < opNm; ++i) {
        if (ops[i].operationName == "similarityDense") {
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
        } 
#ifdef LOUVAINMOD
        if (ops[i].operationName == "louvainModularity") {
            unsigned int boardNm = ops[i].numDevices;
            std::thread thUn[boardNm];
            for (int j = 0; j < boardNm; ++j) {
                thUn[j] = xrm->unloadXclbinNonBlock(deviceCounter + j);
            }
            for (int j = 0; j < boardNm; ++j) {
                thUn[j].join();
            }
            deviceCounter += boardNm;
            oplouvainmod->freeLouvainModularity();
        }
#endif        
    }
    //TODO: the following line crashes GPE.
    //xrm->freeXRM();
};

void Handle::loadXclbin(unsigned int deviceId, char* xclbinName) {
    xrm->loadXclbin(deviceId, xclbinName);
};

/*
std::thread Handle::loadXclbinNonBlock(unsigned int deviceId, std::string& xclbinPath) {
#ifndef NDEBUG    
    std::cout << "DEBUG: " << __FUNCTION__ << " xclbinPath=" << xclbinPath << std::endl;
#endif
    return xrm->loadXclbinNonBlock(deviceId, xclbinPath);
};
*/

std::future<int> Handle::loadXclbinAsync(unsigned int deviceId, std::string& xclbinPath) {
    return xrm->loadXclbinAsync(deviceId, xclbinPath);
};

} // L3
} // graph
} // xf
