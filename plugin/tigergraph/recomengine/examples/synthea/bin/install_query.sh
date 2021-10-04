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
SCRIPTPATH=`dirname $SCRIPT`
ROOT=`dirname $SCRIPTPATH`

# common.sh sets up gsql client and gets username and passowrd
. $SCRIPTPATH/common.sh
# if -i option is set in common.sh, ssh_key_flag is set to -i $ssh_key, 
# otherwise it's an empty string
echo " "
gsql -u $username -p $password "$(cat $ROOT/query/base.gsql | sed "s/@graph/$xgraph/")"
gsql -u $username -p $password -g $xgraph "RUN QUERY insert_dummy_nodes($num_nodes)"
gsql -u $username -p $password "$(cat $ROOT/query/query.gsql | sed "s/@graph/$xgraph/")"

