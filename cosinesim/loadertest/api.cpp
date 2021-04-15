
#include "api.hpp"
#include <iostream>

class Impl : public ImplBase {
    Options arg1_;
public:

    Impl(const Options &arg1) : arg1_(arg1) {}

    virtual void func() {
        std::cout << "intOpt is " << arg1_.intOpt
                << " and strOpt is " << arg1_.strOpt
                << std::endl;
    }
};


extern "C" {

ImplBase *createImpl(const Options &arg1) {
    return new Impl(arg1);
}

void destroyImpl(ImplBase *pImpl) {
    delete pImpl;
}

}
