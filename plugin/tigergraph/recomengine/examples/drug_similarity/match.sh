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

set -e
SCRIPT=$(readlink -f $0)
script_dir=$(dirname $SCRIPT)
# common.sh sets up gsql client, gets username, passowrd, xgraph name
. $script_dir/bin/common.sh

mkdir -p log
if ! chmod 777 log ; then
    echo "Unable to make a 'log' directory writable by all."
    exit 1
fi

# cosinesim_clear_embeddings: base, dist
# cosinesim_embed_vectors: base, dist
# cosinesim_embed_normals: base, dist
# cosinesim_set_num_devices: base, dist
# load_graph_cosinesim_ss_fpga_core: base
# cosinesim_ss_tg: query
# cosinesim_ss_fpga: query

if [ "$load_cache" -eq 1 ]
then
    echo "Clearing embeddings..."
    time gsql -g $xgraph "set query_timeout=240000000 run query cosinesim_clear_embeddings()"
    echo "Caching cosine similarity vectors to Atom vertices..."
    time gsql -g $xgraph "set query_timeout=240000000 run query cosinesim_embed_vectors()"
    time gsql -g $xgraph "set query_timeout=240000000 run query cosinesim_embed_normals()"
fi

if [ "$load_fpga" -eq 1 ]; then
    echo "Run query load_graph_cosinesim_ss_fpga"
    gsql -g $xgraph "run query cosinesim_set_num_devices($num_devices)"
    time gsql -g $xgraph "run query load_graph_cosinesim_ss_fpga_core()"
fi

for ((j = 0 ; j < $iterations ; ++j))
do
    echo "------ iteration $j --------"
    echo "Run query cosinesim_ss_tg"
    time gsql -g $xgraph "run query cosinesim_ss_tg(\"/tmp/tg.txt\")"
    echo "Run query cosinesim_ss_fpga"
    time gsql -g $xgraph "run query cosinesim_ss_fpga(\"/tmp/fpga.txt\")"
done



