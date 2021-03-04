
#include "api.hpp"
#include <string>
#include <dlfcn.h>
#include <iostream>

namespace {

void *getDynamicFunction(const std::string &funcName) {
    // open the library
    
    std::string SOFILEPATH = "libapi.so";
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

ImplBase *createImpl(int arg1) {
    typedef ImplBase * (*CreateFunc)(int);
    CreateFunc pCreateFunc = (CreateFunc) getDynamicFunction("createImpl");
    if (pCreateFunc == nullptr)
        return nullptr;
    return pCreateFunc(arg1);
}

void destroyImpl(ImplBase *pImpl) {
    typedef ImplBase * (*DestroyFunc)(ImplBase *);
    DestroyFunc pDestroyFunc = (DestroyFunc) getDynamicFunction("destroyImpl");
    if (pDestroyFunc == nullptr)
        return;
    pDestroyFunc(pImpl);
}

}
