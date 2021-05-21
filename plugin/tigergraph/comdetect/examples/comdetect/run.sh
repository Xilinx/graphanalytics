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
echo "-------------------------------------------------------------------------"
echo "Running nsert dummy nodes for distributed alveo computing"
gsql -u $username -p $password -g $xgraph "RUN QUERY insert_dummy_nodes($num_nodes)"
echo "-------------------------------------------------------------------------"

echo "Running louvain_distributed_q_cpu"
echo gsql -u $username -p $password -g $xgraph \'run query louvain_distributed_q_cpu\([\"Person\"], [\"Coworker\"],\"weight\",20,1,0.0001,FALSE,FALSE,\"\",\"/home2/tigergraph/output_cpu.txt\",TRUE,FALSE\)\'
echo "-------------------------------------------------------------------------"
START=$(date +%s%3N)
time gsql -u $username -p $password -g $xgraph "run query louvain_distributed_q_cpu([\"Person\"], [\"Coworker\"], \
     \"weight\",20,1,0.0001,FALSE,FALSE,\"\",\"/home2/tigergraph/output_cpu.txt\",TRUE,FALSE)"
TOTAL_TIME=$(($(date +%s%3N) - START))

echo "louvain_distributed_cpu runtime: " $TOTAL_TIME

run_alveo=1
if [ "$run_alveo" -eq 1 ]; then
    START=$(date +%s%3N)
    echo "Running open_alveo"
    echo gsql -u $username -p $password -g $xgraph \'run query open_alveo\(\)\'
    time gsql -u $username -p $password -g $xgraph "run query open_alveo()"
    TOTAL_TIME=$(($(date +%s%3N) - START))
    echo "open_alveo: " $TOTAL_TIME

    START=$(date +%s%3N)
    echo "Running load_alveo"
    echo gsql -u $username -p $password -g $xgraph \'run query load_alveo\([\"Person\"], [\"Coworker\"], \"weight\", FALSE, FALSE, \"$data_source\", \"$partition_prj\", \"9\", \"3\"\)\'
    time gsql -u $username -p $password -g $xgraph "run query load_alveo([\"Person\"], [\"Coworker\"], \
         \"weight\", FALSE, FALSE, \"$data_source\", \"$partition_prj\", \"9\", \"3\")"
    TOTAL_TIME=$(($(date +%s%3N) - START))
    echo "load_alveo: " $TOTAL_TIME

    START=$(date +%s%3N)
    echo "Running louvain_alveo"
    echo gsql -g $xgraph \'run query louvain_alveo\([\"Person\"], [\"Coworker\"], \"weight\",10,1,0.00001,FALSE,FALSE,\"\",\"/home2/tigergraph/output_alveo.txt\",TRUE,FALSE\)\'
    time gsql -g $xgraph "run query louvain_alveo([\"Person\"], [\"Coworker\"], \
         \"weight\",10,1,0.00001,FALSE,FALSE,\"\",\"/home2/tigergraph/output_alveo.txt\",TRUE,FALSE)"
    TOTAL_TIME=$(($(date +%s%3N) - START))
    echo "louvain_alveo: " $TOTAL_TIME
fi
