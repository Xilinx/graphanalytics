#ifndef API_HPP
#define API_HPP

class ImplBase {
public:
    virtual void func() = 0;
};


extern "C" {
ImplBase *createImpl(int arg1);
void destroyImpl(ImplBase *pImpl);
}

class Api {
    ImplBase *pImpl_ = nullptr;
public:

    Api(int arg1) : pImpl_(createImpl(arg1)) {}
    ~Api() { destroyImpl(pImpl_); }

    void func() {
        pImpl_->func();
    }
};

#endif /* API_HPP */

