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

set -x 

function gsql () {
    java  -DGSQL_CLIENT_VERSION=v3_1_0 -jar ../gsql_client.jar -u $username -p $password $@
}

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 TG-username TG-password other-options"
    exit 1
fi
username=$1
password=$2

if [ ! -f "../gsql_client.jar" ]; then
    wget -o wget.log -O ../gsql_client.jar 'https://bintray.com/tigergraphecosys/tgjars/download_file?file_path=com%2Ftigergraph%2Fclient%2Fgsql_client%2F3.1.0%2Fgsql_client-3.1.0.jar'
    echo "INFO: Downloaded the latest gsql client"
fi


