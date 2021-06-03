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


#ifdef HYBRID_DEMO
#define API_INLINE_IMPL
#include "api_loader.cpp"
#endif

#include "api.hpp"

Api *makeApi() {
    Options options;
    options.intOpt = 10;
    options.strOpt = "hello world this is a very long string to make sure that it uses dynamic space instead of static";
    Api *pApi = new Api(options);
    return pApi;
}

int main(int, char **) {
    Api *pApi = makeApi();
    pApi->func();
    return 0;
}
