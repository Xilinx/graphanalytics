
#include "api.hpp"
#include <iostream>

class Impl : public ImplBase {
    int arg1_ = 0;
public:

    Impl(int arg1) : arg1_(arg1) {}

    virtual void func() {
        std::cout << "Arg1 is " << arg1_ << std::endl;
    }
};


extern "C" {

ImplBase *createImpl(int arg1) {
    return new Impl(arg1);
}

void destroyImpl(ImplBase *pImpl) {
    delete pImpl;
}

}
