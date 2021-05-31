#!/bin/bash
# Copyright 2021 Xilinx, Inc.
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

echo "INFO: Running $0"

. env.sh

if [ "$#" -ne 3 ]; then
    echo $#
    echo "$0 in-mtx-file in-cluster-info-file offset"
    exit 1
fi

in_file=$1
in_cluster_info_file=$2
offset=$3

exe_dir="Release"
if [ "$DEBUG" == "1" ]; then
    exe_dir="Debug"
fi 

cmd="../$exe_dir/louvainmod_test -compute_modularity $in_file $in_cluster_info_file $offset"
echo $cmd
$cmd
