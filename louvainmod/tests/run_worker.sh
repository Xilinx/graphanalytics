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

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`
echo Running $SCRIPT
if [ "$#" -lt 5 ]; then
    echo "$0 <.mtx file> <partition project base name> <number of partitions> <max_level> <worker_number>. Example: $0 /proj/isimsj/graphdb/louvain/data/europe_osm-wt900M.mtx 900 18 4 1"
    echo "worker_number should be different for each worker. Give a number starting 1. Give 1 for first worker, 2 for second and so forth."
    exit 1
fi

. env.sh

graph=$1
projdir=$2.par.proj
par=$3
numlevel=$4
worker_number=$5
num_dev=3
num_workers=2
workers="tcp://192.168.1.21:5555 tcp://192.168.1.31:5555"
xclbinfile=$SCRIPTPATH/../staging/xclbin/louvainmod_pruning_xilinx_u50_gen3x16_xdma_201920_3.xclbin

exe_dir="Release"
if [ "$DEBUG" == "1" ]; then
    exe_dir="Debug"
fi 
cmd="../$exe_dir/louvainmod_test -x $xclbinfile $graph -fast -dev $num_dev -num_level $numlevel -par_num $par \
          -load_alveo_partitions $projdir -setwkr $num_workers $workers -workerAlone $worker_number"
echo $cmd
$cmd

