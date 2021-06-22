# Xilinx Single Source Cosine Similarity Demo

This is a demo of running Graph L3 library. It has two main steps, firstly `Build dynamic library and xclbin`, it may take several hours. Once the xclbin is ready, users should follow steps in the `Run demo`. The demo is set to run in one Alveo U50 board by default. But if users want to use two U50 boards, this can be achieved by simply setting `deviceNeeded` in ../../tests/cosineSimilaritySSDenseIntBench/test_cosineSimilaritySSDense.cpp to 2. 

## Build dynamic library and xclbin
    ./build.sh

## Run demo
    change PROJECTPATH in ../../tests/cosineSimilaritySSDenseIntBench/config.json to graph library's absolute path 
    ./run.sh

#
# Copyright 2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#      http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
