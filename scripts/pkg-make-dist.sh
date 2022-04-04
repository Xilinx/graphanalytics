#!/bin/bash

# Copyright 2022 Xilinx, Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.


set -e
set -x

cd ../cosinesim && make cleanall && make dist DIST_RELEASE=1 && cd -
cd ../plugin/tigergraph/recomengine/ && make clean && make dist DIST_RELEASE=1 && cd -


cd ../louvainmod && make cleanall && make dist DIST_RELEASE=1 && cd -
cd ../plugin/tigergraph/comdetect/ && make clean && make dist DIST_RELEASE=1 && cd -

cd ../fuzzymatch && make cleanall && make dist DIST_RELEASE=1 && cd -
cd ../plugin/tigergraph/fuzzymatch/ && make clean && make dist DIST_RELEASE=1 && cd -

cd ../mis && make cleanall && make dist DIST_RELEASE=1 && cd -
cd ../plugin/tigergraph/mis/ && make clean && make dist DIST_RELEASE=1 && cd -

OSDIST=`lsb_release -i |awk -F: '{print tolower($2)}' | tr -d ' \t'`

if [[ $OSDIST == "ubuntu" ]]; then
    find . -name "xilinx*.deb"
elif [[ $OSDIST == "centos" ]]; then
    find . -name "xilinx*.rpm"
else 
    echo "ERROR: only Ubuntu and Centos are supported."
    exit 3
fi





