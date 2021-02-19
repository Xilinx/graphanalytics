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
echo "Caching cosine similarity vectors to patient vertices..."
time gsql -g xgraph "set query_timeout=240000000 run query cosinesim_cache_to_vertices()"
echo "Caching cosine similarity vectors to C++ map..."
time gsql -g xgraph "set query_timeout=240000000 run query cosinesim_cache_to_cppmap()"
echo "Running query cosinesim_ss_tg..."
for i in {1..1}
do
#    echo "Restarting TigerGraph..."
#    gadmin restart all -y
    for j in {1..3}
    do
        echo "Running cosine similarity match with patient vertex cache..."
        time gsql -g xgraph "set query_timeout=240000000 run query cosinesim_vertices_sw(\"$PWD/vertices_sw.txt\")"
        echo "Running cosine similarity match with C++ cache..."
        time gsql -g xgraph "set query_timeout=240000000 run query cosinesim_cppmap_sw(\"$PWD/cppmap_sw.txt\")"
    done
done
