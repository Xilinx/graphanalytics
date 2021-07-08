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
