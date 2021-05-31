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

