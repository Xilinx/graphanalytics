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

tg_partition="FALSE"
use_saved_partition="FALSE"
if [ "$partition_mode" -eq 0 ]; then
   tg_partition="TRUE"
elif [ "$partition_mode" -eq 1 ]; then
   tg_partition="FALSE"
else 
   use_saved_partition="TRUE"
fi
node_names="xsj-dxgradb01 xsj-dxgradb02 xsj-dxgradb03"
node_ips="192.168.1.11 192.168.1.21 192.168.1.31"    


if [ "$compile_mode" -eq 0 ]; then
    echo "-------------------------------------------------------------------------"
    echo "Running schema.gsql"
    echo "gsql -u $username -p $password \"\$(cat $script_dir/query/schema.gsql | sed \"s/@graph/$xgraph/\")\""
    echo "-------------------------------------------------------------------------"
    gsql -u $username -p $password "$(cat $script_dir/query/schema.gsql | sed "s/@graph/$xgraph/")"

    echo "-------------------------------------------------------------------------"
    echo "Installing load.gsql"
    echo "gsql -u $username -p $password \"\$(cat $script_dir/query/load.gsql | sed \"s/@graph/$xgraph/\")\""
    echo "-------------------------------------------------------------------------"
    gsql -u $username -p $password "$(cat $script_dir/query/load.gsql | sed "s/@graph/$xgraph/")"

    echo "-------------------------------------------------------------------------"
    echo "Loading $files"
    echo "gsql -u $username -p $password -g $xgraph \"run loading job load_job USING file_name = \"$data_source\"\""
    echo "-------------------------------------------------------------------------"
    gsql -u $username -p $password -g $xgraph "run loading job load_job USING file_name = \"$data_source\""

    echo "-------------------------------------------------------------------------"
    echo "Install base queries"
    echo "gsql -u $username -p $password \"\$(cat $script_dir/query/base.gsql | sed \"s/@graph/$xgraph/\")\""
    echo "-------------------------------------------------------------------------"
    gsql -u $username -p $password "$(cat $script_dir/query/base.gsql | sed "s/@graph/$xgraph/")"

    echo "-------------------------------------------------------------------------"
    echo "Running insert dummy nodes for distributed alveo computing"
    gsql -u $username -p $password -g $xgraph "RUN QUERY insert_dummy_nodes($num_nodes)"
fi

if [ "$compile_mode" -eq 0 ] || [ "$compile_mode" -eq 1 ]; then
    echo "-------------------------------------------------------------------------"
    echo "Installing louvain_distributed_cpu query"
    echo "gsql -u $username -p $password -g $xgraph \"$script_dir/query/louvain_distributed_q_cpu.gsql\""
    echo "-------------------------------------------------------------------------"
    gsql -u $username -p $password -g $xgraph "$script_dir/query/louvain_distributed_q_cpu.gsql"

    echo "-------------------------------------------------------------------------"
    echo "Installing Louvain Alveo queries"
    echo "gsql -u $username -p $password \"\$(cat $script_dir/query/louvain_alveo.gsql | sed \"s/@graph/$xgraph/\")\""
    echo "-------------------------------------------------------------------------"
    gsql -u $username -p $password "$(cat $script_dir/query/louvain_alveo.gsql | sed "s/@graph/$xgraph/")"

    # IMPORTANT: DO NOT USE A NETWORK DRIVE FOR LOG FILES IN DISTRIBUTED QUERIES.
    # OTHERWISE EACH NODE WILL OVERWRITE IT
fi

echo "-------------------------------------------------------------------------"
echo "Run mode: $run_mode"

if [ "$run_mode" -eq 0 ] || [ "$run_mode" -eq 2 ]; then
   echo "Running louvain_distributed_q_cpu"
   echo gsql -u $username -p $password -g $xgraph \'run query louvain_distributed_q_cpu\([\"Person\"], [\"Coworker\"],\"weight\",20,1,0.0001,FALSE,FALSE,\"\",\"/home2/tigergraph/output_cpu.txt\",TRUE,FALSE\)\'
   echo "-------------------------------------------------------------------------"
   START=$(date +%s%3N)
   time gsql -u $username -p $password -g $xgraph "run query louvain_distributed_q_cpu([\"Person\"], [\"Coworker\"], \
        \"weight\",20,1,0.0001,FALSE,FALSE,\"\",\"/home2/tigergraph/output_cpu.txt\",TRUE,FALSE)"
   TOTAL_TIME=$(($(date +%s%3N) - START))
   echo "louvain_distributed_cpu runtime: " $TOTAL_TIME
fi

if [ "$run_mode" -eq 1 ] || [ "$run_mode" -eq 2 ]; then
    # no need to run open_alveo in production flow. The context is automatically created when needed
    START=$(date +%s%3N)
    echo "Running load_alveo"
    echo gsql -u $username -p $password -g $xgraph \'run query load_alveo\([\"Person\"], [\"Coworker\"], \"weight\", $tg_partition, $use_saved_partition, \"$data_source\", \"$alveo_prj\", \"$node_names\", \"$node_ips\", $num_partitions, $num_devices\)\'
    time gsql -u $username -p $password -g $xgraph "run query load_alveo([\"Person\"], [\"Coworker\"], \
         \"weight\", $tg_partition, $use_saved_partition, \"$data_source\", \"$alveo_prj\", \"$node_names\", \"$node_ips\", $num_partitions, $num_devices)"
    TOTAL_TIME=$(($(date +%s%3N) - START))
    echo "load_alveo: " $TOTAL_TIME

    START=$(date +%s%3N)
    echo "Running louvain_alveo"
    echo gsql -u $username -p $password -g $xgraph \'run query louvain_alveo\([\"Person\"], [\"Coworker\"], \"weight\",20,10,0.0001,FALSE,FALSE,\"\",\"/home2/tigergraph/output_alveo.txt\",TRUE,FALSE\)\'
    time gsql -u $username -p $password -g $xgraph "run query louvain_alveo([\"Person\"], [\"Coworker\"], \
         \"weight\",20,1,0.0001,FALSE,FALSE,\"\",\"/home2/tigergraph/output_alveo.txt\",TRUE,FALSE)"
    TOTAL_TIME=$(($(date +%s%3N) - START))
    echo "louvain_alveo: " $TOTAL_TIME
fi
