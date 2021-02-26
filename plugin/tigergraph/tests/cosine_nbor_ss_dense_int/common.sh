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
    java -DGSQL_CLIENT_VERSION=v3_1_0 -jar ../gsql_client.jar -u $username -p $password "$@"
}

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 TG-username TG-password other-options"
    exit 1
fi
username=$1
password=$2
xgraph="xgraph_$username"

if [ ! -f "../gsql_client.jar" ]; then
    wget -o wget.log -O ../gsql_client.jar 'https://dl.bintray.com/tigergraphecosys/tgjars/com/tigergraph/client/gsql_client/3.1.0/gsql_client-3.1.0.jar'
    echo "INFO: Downloaded the latest gsql client"
fi


