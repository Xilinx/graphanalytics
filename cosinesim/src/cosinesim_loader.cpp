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

#include "cosinesim.hpp"
#include <string>
#include <dlfcn.h>
#include <iostream>

namespace {

void *getDynamicFunction(const std::string &funcName) {
    // open the library
    
    std::string SOFILEPATH = "libXilinxCosineSim.so";
    void* handle = dlopen(SOFILEPATH.c_str(), RTLD_LAZY | RTLD_NOLOAD);
    if (handle == nullptr) {
        std::cout << "INFO: " << SOFILEPATH << " not loaded. Loading now..." << std::endl;
        handle = dlopen(SOFILEPATH.c_str(), RTLD_LAZY | RTLD_GLOBAL);
        if (handle == nullptr) {
            std::cerr << "ERROR: Cannot open library: " << dlerror() << '\n';
            return nullptr;
        }
    }

    // load the symbol
    dlerror();  // reset errors
    void *pFunc = dlsym(handle, funcName.c_str());
    const char* dlsym_error2 = dlerror();
    if (dlsym_error2) {
        std::cerr << "ERROR: Cannot load symbol '" << funcName << "': " << dlsym_error2 << std::endl;
        return nullptr;
    }
    return pFunc;
}

}  // namespace <anonymous>

//#####################################################################################################################

extern "C" {

#ifdef XILINX_COSINESIM_INLINE_IMPL
#define XILINX_COSINESIM_IMPL_DEF inline
#else
#define XILINX_COSINESIM_IMPL_DEF
#endif
    
XILINX_COSINESIM_IMPL_DEF
xilinx_apps::cosinesim::ImplBase *xilinx_cosinesim_createImpl(const xilinx_apps::cosinesim::Options& options,
        unsigned valueSize)
{
    typedef xilinx_apps::cosinesim::ImplBase * (*CreateFunc)(const xilinx_apps::cosinesim::Options &, unsigned);
    CreateFunc pCreateFunc = (CreateFunc) getDynamicFunction("xilinx_cosinesim_createImpl");
    if (pCreateFunc == nullptr)
        return nullptr;  // TODO: throw exception?
    return pCreateFunc(options, valueSize);
}

XILINX_COSINESIM_IMPL_DEF
void xilinx_cosinesim_destroyImpl(xilinx_apps::cosinesim::ImplBase *pImpl) {
    typedef xilinx_apps::cosinesim::ImplBase * (*DestroyFunc)(xilinx_apps::cosinesim::ImplBase *);
    DestroyFunc pDestroyFunc = (DestroyFunc) getDynamicFunction("xilinx_cosinesim_destroyImpl");
    if (pDestroyFunc == nullptr)
        return;
    pDestroyFunc(pImpl);
}

}  // extern "C"

