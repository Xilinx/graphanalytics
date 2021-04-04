#!/bin/bash
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

script_dir=$(dirname "$0")
# common.sh sets up gsql client, gets username, passowrd, xgraph name
. $script_dir/bin/common.sh

mkdir -p log
if ! chmod 777 log ; then
    echo "Unable to make a 'log' directory writable by all."
    exit 1
fi


if [ "$load_cache" -eq 1 ]
then
    echo "Caching cosine similarity vectors to patient vertices..."
    time gsql -g $xgraph "set query_timeout=240000000 run query cosinesim_cache_to_vertices()"
fi

if [ "$load_fpga" -eq 1 ]; then
    echo "Run query load_cu_cosinesim_ss_fpga and load_graph_cosinesim_ss_fpga"
    gsql -g $xgraph "run query cosinesim_set_num_devices($devices_needed)"
    time gsql -g $xgraph "run query load_cu_cosinesim_ss_fpga($devices_needed)"
    time gsql -g $xgraph "run query load_graph_cosinesim_ss_fpga($devices_needed)"
fi

for ((j = 0 ; j < $iterations ; ++j))
do
    rm -f log/fpga.txt log/tg.txt

    echo "------ iteration $j --------"
    echo "Run query cosinesim_ss_tg"
    time gsql -g $xgraph "run query cosinesim_ss_tg(\"$PWD/log/tg.txt\")"
    echo "Run query cosinesim_ss_fpga"
    time gsql -g $xgraph "run query cosinesim_ss_fpga(\"$PWD/log/fpga.txt\", $devices_needed)"
    
    # basic checking of the result
    diff log/fpga.txt log/tg.txt
done



