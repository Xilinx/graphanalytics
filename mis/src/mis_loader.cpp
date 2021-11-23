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
 * WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Thanks to Aaron Isotton for his dynamic loading ideas in https://tldp.org/HOWTO/pdf/C++-dlopen.pdf

#include <string>
#include <dlfcn.h>
#include <iostream>
#include <sstream>

#include "xilinxmis.hpp"

namespace {

void *getDynamicFunction(const std::string &funcName) {
    // open the library
    
    std::string SOFILEPATH = "libXilinxMis.so";
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
            throw xilinx_apps::mis::Exception(oss.str());
        }
    }

    // load the symbol
    std::cout << "DEBUG: after handle==nullptr check" << std::endl;
    dlerror();  // reset errors
    std::cout << "DEBUG: before dlsym" << std::endl;
    void *pFunc = dlsym(handle, funcName.c_str());
    std::cout << "DEBUG: after dlsym" << std::endl;
    const char* dlsym_error2 = dlerror();
    if (dlsym_error2) {
        std::cout << "DEBUG: inside dlsym_error2" << std::endl;
        std::ostringstream oss;
        oss << "Cannot load symbol '" << funcName << "': " << dlsym_error2
                << ".  Possibly an older version of library " << SOFILEPATH
                << " is in use.  Please install the correct version." << std::endl;
        std::cout << "DEBUG: after 2nd oss filling" << std::endl;
        throw xilinx_apps::mis::Exception(oss.str());
    }
    std::cout << "DEBUG: before return" << std::endl;
    return pFunc;
}

}  // namespace <anonymous>

//#####################################################################################################################

extern "C" {

#ifdef XILINX_MIS_INLINE_IMPL
#define XILINX_MIS_IMPL_DEF inline
#else
#define XILINX_MIS_IMPL_DEF
#endif
    
XILINX_MIS_IMPL_DEF
xilinx_apps::mis::MisImpl *xilinx_mis_createImpl(const xilinx_apps::mis::Options& options)
{
    typedef xilinx_apps::mis::MisImpl * (*CreateFunc)(const xilinx_apps::mis::Options &);
    CreateFunc pCreateFunc = (CreateFunc) getDynamicFunction("xilinx_mis_createImpl");
    std::cout << "DEBUG: createImpl handle " << (void *) pCreateFunc << std::endl;
    return pCreateFunc(options);
}

XILINX_MIS_IMPL_DEF
void xilinx_mis_destroyImpl(xilinx_apps::mis::MisImpl *pImpl) {
    typedef xilinx_apps::mis::MisImpl * (*DestroyFunc)(xilinx_apps::mis::MisImpl *);
    DestroyFunc pDestroyFunc = (DestroyFunc) getDynamicFunction("xilinx_mis_destroyImpl");
    pDestroyFunc(pImpl);
}

}  // extern "C"