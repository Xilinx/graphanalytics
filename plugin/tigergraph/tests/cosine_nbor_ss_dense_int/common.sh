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

function gsql () {
    java -DGSQL_CLIENT_VERSION=v3_1_0 -jar $HOME/gsql-client/gsql_client.jar \
        -u $username -p $password "$@"
}

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 TG-username TG-password [load-option graph-name data-root]"
    exit 1
fi
username=$1
password=$2
# $3 is -noload or default
# $4 graph-name
if [ "$#" -gt 3 ]; then
    xgraph=$4
else
    xgraph="xgraph_$username"
fi
if [ "$#" -gt 4 ]; then
    data_root=$5
else
    data_root="./1000_patients/csv"
fi
echo "INFO: data_root=$data_root"

if [ ! -f "$HOME/gsql-client/gsql_client.jar" ]; then
    mkdir -p $HOME/gsql-client
    wget -o wget.log -O $HOME/gsql-client/gsql_client.jar 'https://dl.bintray.com/tigergraphecosys/tgjars/com/tigergraph/client/gsql_client/3.1.0/gsql_client-3.1.0.jar'
    echo "INFO: Downloaded the latest gsql client"
fi


