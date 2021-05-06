#!/usr/bin/env bash

#
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
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#
# Copyright 2020-2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

SCRIPT=$(readlink -f $0)
script_dir=`dirname $SCRIPT`

. $script_dir/bin/common.sh

#temporary protection for old install method
if [ -x $script_dir/bin/install-udf.sh ]
then
echo "Installing UDF files"
$script_dir/bin/install-udf.sh $ssh_key_flag $verbose_flag $force_clean_flag
else
echo "Skipping UDF installation.  ExprFunctions.hpp is expected to have these UDFs already."
fi

#files=/proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx 

echo "Running schema.gsql"
gsql "$(cat $script_dir/schema.gsql | sed "s/@graph/$xgraph/")"


echo "Installing load.gsql"
gsql "$(cat $script_dir/load.gsql | sed "s/@graph/$xgraph/")"

echo "Loading $files"
time gsql -g $xgraph "run loading job load_job USING file_name = \"$data_source\""

echo "Installing louvain_distributed_cpu query"
time gsql -g $xgraph louvain_distributed_cpu.gsql

echo "Installing louvain_alveo queries"
gsql "$(cat $script_dir/louvain_alveo.gsql | sed "s/@graph/$xgraph/")"

# IMPORTANT: DO NOT USE A NETWORK DRIVE FOR LOG FILES IN DISTRIBUTED QUERIES.
# OTHERWISE EACH NODE WILL OVERWRITE IT
echo "Running louvain_distributed_cpu"
START=$(date +%s%3N)
time gsql -g $xgraph "run query louvain_distributed_cpu(20, [\"Person\"], [\"Coworker\"], \"/tmp/cpu_out.txt\")"
TOTAL_TIME=$(($(date +%s%3N) - START))

echo "louvain_distributed_cpu runtime: " $TOTAL_TIME

run_alveo=0
if [ "$run_alveo" -eq 1 ]; then
    START=$(date +%s%3N)
    time gsql -g $xgraph 'run query load_alveo()'
    TOTAL_TIME=$(($(date +%s%3N) - START))
    echo "load_alveo: " $TOTAL_TIME
    
    START=$(date +%s%3N)
    #command example
    #time gsql -g $xgraph "run query louvain_alveo(10, [\"Person\"], [\"Coworker\"], \"$PWD/log/alveo_out.txt\", \"$PWD/as-skitter/as-skitter-wt-e110k.mtx\", \"$PWD/as-skitter/as-skitter-partitions/louvain_partitions\")"

    time gsql -g $xgraph "run query louvain_alveo(10, [\"Person\"], [\"Coworker\"], \
                                                 \"/tmp/log/alveo_out.txt\", \
                                                 \"$data_source\", \
                                                 \"$partition_prj\")"
    TOTAL_TIME=$(($(date +%s%3N) - START))
    echo "louvain_alveo: " $TOTAL_TIME
fi

