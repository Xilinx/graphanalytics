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

    START=$(date +%s%3N)
    echo "Running louvain_alveo"
    echo gsql -u $username -p $password -g $xgraph \'run query louvain_alveo\([\"Person\"], [\"Coworker\"], \"weight\",20,20,0.0001,FALSE,FALSE,\"\",\"/home2/tigergraph/output_alveo.txt\",TRUE,FALSE\)\'
    time gsql -u $username -p $password -g $xgraph "run query louvain_alveo([\"Person\"], [\"Coworker\"], \
         \"weight\",20,20,0.0001,FALSE,FALSE,\"\",\"/home2/tigergraph/output_alveo.txt\",TRUE,FALSE)"
    TOTAL_TIME=$(($(date +%s%3N) - START))
    echo "louvain_alveo: " $TOTAL_TIME
fi

