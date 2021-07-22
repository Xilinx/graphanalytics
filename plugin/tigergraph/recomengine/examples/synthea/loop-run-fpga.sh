#!/bin/bash

# Copyright 2020 Xilinx, Inc.
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

# This script runs multiple iterations of cosine similarity kernel on FPGA 
# to test consistentcy of the result and reliabiity of Alveo cards

script_dir=$(dirname "$0")

. $script_dir/bin/common.sh

if [ "$load_fpga" -eq 1 ]; then
    echo "Load FPGA with accelerators"
    time gsql -g $xgraph "set query_timeout=240000000 run query loadgraph_cosinesim_ss_fpga($devices_needed)"
fi

# do a basic multi-user test

for ((i=1; i <= $iterations ; i++)); do
    echo "------------------ iteration $i -------------------------------"
    for ((u=1; u <= 8; u++)); do
        echo "################ user $u ################"
        gsql -g $xgraph "set query_timeout=240000000 run query cosinesim_ss_fpga(\"$PWD/log/fpgai$u.txt\", $devices_needed)" &
    done
    wait
done 
