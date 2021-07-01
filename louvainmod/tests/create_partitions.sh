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

script=$(readlink -f $0)
script_dir=`dirname $script`

echo Running $script
if [ "$#" -lt 3 ]; then
    echo "$script <.mtx file> <partition project base name> <number of partitions>."
    echo "Example: $0 /proj/autoesl/ryanw/graph/Demo_For_webinary_WT/as-Skitter-wt.mtx skitter 9 -server_par $4"
    exit 1
fi

graph=$1
projbasename=$2
partitions=$3

exe_dir="Release"
if [ "$DEBUG" == "1" ]; then
    exe_dir="Debug"
fi 

export LD_LIBRARY_PATH=$script_dir/../$exe_dir/:$LD_LIBRARY_PATH
cmd="$script_dir/../$exe_dir/louvainmod_test  $1 -fast -par_num $partitions -create_alveo_partitions -name $projbasename -server_par $4"
echo $cmd
$cmd

echo "*************************************************************************"
echo "INFO: $partitions partitions saved to $subdir"
echo "*************************************************************************"





