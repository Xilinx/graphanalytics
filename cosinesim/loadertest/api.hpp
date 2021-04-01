#ifndef API_HPP
#define API_HPP

#include <string>

#ifdef API_INLINE_IMPL
#define API_IMPL_DECL inline
#else
#define API_IMPL_DECL extern
#endif

struct Options {
    int intOpt;
    std::string strOpt;
};


class ImplBase {
public:
    virtual void func() = 0;
};


extern "C" {
API_IMPL_DECL
ImplBase *createImpl(const Options &arg1);

API_IMPL_DECL
void destroyImpl(ImplBase *pImpl);
}

class Api {
    ImplBase *pImpl_ = nullptr;
public:

    Api(const Options &arg1) : pImpl_(createImpl(arg1)) {}
    ~Api() { destroyImpl(pImpl_); }

    void func() {
        pImpl_->func();
    }
};

#endif /* API_HPP */

