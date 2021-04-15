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
script_dir=$(dirname "$0")
# common.sh sets up things like gsql client, username and passowrd, graph name, etc
. $script_dir/bin/common.sh

echo "data_root=$data_root"

time gsql "$(cat $script_dir/query/schema_xgraph.gsql | sed "s/@graph/$xgraph/")"
time gsql "SET sys.data_root=\"$data_root\" $(cat $script_dir/query/load_xgraph.gsql | sed "s/@graph/$xgraph/")"

# set timeout of loading job to 1 hour
time gsql -g $xgraph "SET QUERY_TIMEOUT=3600000 SET sys.data_root=\"$data_root\" RUN LOADING JOB load_xgraph"
time gsql -g $xgraph "DROP JOB load_xgraph"
echo "INFO: -------- $(date) load_xgraph completed. --------"
