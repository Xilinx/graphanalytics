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
# common.sh sets up gsql client and gets username and passowrd
. common.sh

if [ "$3" != "-noload" ]
then
    echo "Caching cosine similarity vectors to patient vertices..."
    time gsql -g xgraph "set query_timeout=240000000 run query cosinesim_cache_to_vertices()"
fi

echo "Run query loadgraph_cosinesim_ss_fpga"
time gsql -g xgraph "set query_timeout=240000000 run query loadgraph_cosinesim_ss_fpga()"
for j in {1..3}
do
    echo "Run query cosinesim_ss_tg"
    time gsql -g xgraph "set query_timeout=240000000 run query cosinesim_ss_tg(\"$PWD/tg.txt\")"
    echo "Run query cosinesim_ss_fpga"
    time gsql -g xgraph "set query_timeout=240000000 run query cosinesim_ss_fpga(\"$PWD/fpga.txt\")"
done
echo "Run query close_fpga"
time gsql -g xgraph "set query_timeout=240000000 run query close_fpga()"
