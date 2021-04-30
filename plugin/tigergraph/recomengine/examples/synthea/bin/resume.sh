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
. $script_dir/common.sh

echo "Installing queries"
gsql "$(cat $script_dir/../query/resume.gsql | sed "s/@graph/$xgraph/")"
echo "Counting patient vertices without vectors"
gsql -g $xgraph "run query cosinesim_count_uncached_vertices()"
echo "Resuming caching of vectors"
gsql -g $xgraph "set query_timeout=240000000 run query cosinesim_resume_cache_to_vertices()"
echo "Counting patient vertices without vectors"
gsql -g $xgraph "run query cosinesim_count_uncached_vertices()"
