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

function usage() {
    echo "Usage: $0 -u TG-username -p TG-password [optional options]"
    echo "Optional options:"
    echo "  -c 0|1              : 0=Do not load cache; 1=Load cache(default)"
    echo "  -d numDevices       : number of FPGAs needed (default=1)"
    echo "  -n iterations       : number of iterations to run (default=1)"
    echo "  -g graphName        : graph name (default=xgraph_<username>"
    echo "  -l 0|1              : 0: Do not load FPGA; 1: Load FPGA(default)>"
    echo "  -s dataSourcePath   : path containing input data. default=./1000_patients/csv"
    echo "  -c 0|1              : 0=Do not load cache; 1=Load cache(default)"
    echo "  -h                  : Print this help message"
}

# default values for optional options
username=$USER
password=Xilinx123
data_root="$script_dir/1000_patients/csv"
load_cache=1
load_fpga=1
devices_needed=1
iterations=1
while getopts ":c:d:g:l:n:p:s:u:h" opt
do
case $opt in
    c) load_cache=$OPTARG;;
    d) devices_needed=$OPTARG;;
    g) xgraph=$OPTARG;;
    l) load_fpga=$OPTARG;;
    n) iterations=$OPTARG;;
    p) password=$OPTARG;;
    s) data_root=$OPTARG;;
    u) username=$OPTARG;;
    h) usage; exit 1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

if [ -z "$username" ] || [ -z "$password" ]; then
    echo "ERROR: username and password are required."
    usage
    exit 2
fi

# $4 graph-name
if [ -z "$xgraph" ]; then
    xgraph="xgraph_$username"
fi

echo "INFO: username=$username"
echo "      password=$password"
echo "      data_root=$data_root"
echo "      xgraph=$xgraph"
echo "      load_cache=$load_cache"
echo "      load_fpga=$load_fpga"
echo "      devices_needed=$devices_needed"
echo "      iterations=$iterations"

if [ ! -f "$HOME/gsql-client/gsql_client.jar" ]; then
    mkdir -p $HOME/gsql-client
    wget -o wget.log -O $HOME/gsql-client/gsql_client.jar 'https://dl.bintray.com/tigergraphecosys/tgjars/com/tigergraph/client/gsql_client/3.1.0/gsql_client-3.1.0.jar'
    echo "INFO: Downloaded the latest gsql client"
fi


