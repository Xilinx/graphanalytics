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
 * WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Thanks to Aaron Isotton for his dynamic loading ideas in https://tldp.org/HOWTO/pdf/C++-dlopen.pdf

#include "xilinxlouvain.h"
#include <string>
#include <dlfcn.h>
#include <iostream>
#include <sstream>

namespace {

void *getDynamicFunction(const std::string &funcName) {
    // open the library
    
    std::string SOFILEPATH = "libXilinxLouvain.so";
    void* handle = dlopen(SOFILEPATH.c_str(), RTLD_LAZY | RTLD_NOLOAD);
    if (handle == nullptr) {
        std::cout << "INFO: " << SOFILEPATH << " not loaded. Loading now..." << std::endl;
        handle = dlopen(SOFILEPATH.c_str(), RTLD_LAZY | RTLD_GLOBAL);
        std::cout << "DEBUG: after dlopen" << std::endl;
        if (handle == nullptr) {
            std::cout << "DEBUG: inside handle==nullptr" << std::endl;
            std::ostringstream oss;
            oss << "Cannot open library " << SOFILEPATH << ": " << dlerror()
                    << ".  Please ensure that the library's path is in LD_LIBRARY_PATH."  << std::endl;
            std::cout << "DEBUG: after oss filling" << std::endl;
            throw xilinx_apps::louvainmod::Exception(oss.str());
        }
    }

    // load the symbol
#ifndef NDEBUG    
    std::cout << "DEBUG: after handle==nullptr check" << std::endl;
#endif    
    dlerror();  // reset errors
#ifndef NDEBUG        
    std::cout << "DEBUG: before dlsym" << std::endl;
#endif     

    void *pFunc = dlsym(handle, funcName.c_str());
#ifndef NDEBUG        
    std::cout << "DEBUG: after dlsym" << std::endl;
#endif 

    const char* dlsym_error2 = dlerror();
    if (dlsym_error2) {
#ifndef NDEBUG            
        std::cout << "DEBUG: inside dlsym_error2" << std::endl;
#endif 
        std::ostringstream oss;
        oss << "Cannot load symbol '" << funcName << "': " << dlsym_error2
                << ".  Possibly an older version of library " << SOFILEPATH
                << " is in use.  Please install the correct version." << std::endl;
        std::cout << "DEBUG: after 2nd oss filling" << std::endl;
        throw xilinx_apps::louvainmod::Exception(oss.str());
    }
    std::cout << "DEBUG: before return" << std::endl;
    return pFunc;
}

}  // namespace <anonymous>

//#####################################################################################################################

extern "C" {

#ifdef XILINX_LOUVAINMOD_INLINE_IMPL
#define XILINX_LOUVAINMOD_IMPL_DEF inline
#else
#define XILINX_LOUVAINMOD_IMPL_DEF
#endif
    
//XILINX_LOUVAINMOD_IMPL_DEF
//xilinx_apps::louvainmod::ImplBase *xilinx_louvainmod_createImpl(const xilinx_apps::louvainmod::Options& options,
//        unsigned valueSize)
//{
//    typedef xilinx_apps::louvainmod::ImplBase * (*CreateFunc)(const xilinx_apps::louvainmod::Options &, unsigned);
//    CreateFunc pCreateFunc = (CreateFunc) getDynamicFunction("xilinx_louvainmod_createImpl");
//    std::cout << "DEBUG: createImpl handle " << (void *) pCreateFunc << std::endl;
//    return pCreateFunc(options, valueSize);
//}
//
//XILINX_LOUVAINMOD_IMPL_DEF
//void xilinx_louvainmod_destroyImpl(xilinx_apps::louvainmod::ImplBase *pImpl) {
//    typedef xilinx_apps::louvainmod::ImplBase * (*DestroyFunc)(xilinx_apps::louvainmod::ImplBase *);
//    DestroyFunc pDestroyFunc = (DestroyFunc) getDynamicFunction("xilinx_louvainmod_destroyImpl");
//    pDestroyFunc(pImpl);
//}

XILINX_LOUVAINMOD_IMPL_DEF
int load_alveo_partitions(int argc, char *argv[]) {
    typedef int (*CreatePartitionsFunc)(int, char *[]);
    CreatePartitionsFunc pCreateFunc = (CreatePartitionsFunc) getDynamicFunction("load_alveo_partitions");
    return pCreateFunc(argc, argv);
}

XILINX_LOUVAINMOD_IMPL_DEF
int create_and_load_alveo_partitions(int argc, char *argv[]) {
    typedef int (*CreatePartitionsFunc)(int, char *[]);
    CreatePartitionsFunc pCreateFunc = (CreatePartitionsFunc) getDynamicFunction("create_and_load_alveo_partitions");
    return pCreateFunc(argc, argv);
}

XILINX_LOUVAINMOD_IMPL_DEF
float compute_louvain_alveo(    
    char* xclbinPath, bool flowFast, unsigned numDevices, 
    unsigned num_par, char* alveoProject, 
    unsigned mode_zmq, unsigned numPureWorker, char* nameWorkers[128], unsigned nodeID,
    char* opts_outputFile, int64_t max_iter, int64_t max_level, float tolerence, bool intermediateResult,
    bool verbose, bool final_Q, bool all_Q) 
{
    typedef float (*LoadPartitionsFunc)(char*, bool, unsigned, 
                                        unsigned, char*, 
                                        unsigned, unsigned, char* [], unsigned,
                                        char*, unsigned, unsigned, float, bool, bool, bool, bool);
    LoadPartitionsFunc pLoadFunc = (LoadPartitionsFunc) getDynamicFunction("compute_louvain_alveo");
    return pLoadFunc(xclbinPath, flowFast, numDevices, 
                     num_par, alveoProject, 
                     mode_zmq, numPureWorker, nameWorkers, nodeID,
                     opts_outputFile, max_iter, max_level, tolerence, intermediateResult,
		     verbose, final_Q, all_Q);
}

}  // extern "C"


