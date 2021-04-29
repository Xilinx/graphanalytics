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

gsql_command="java -Droot.log.level=INFO -DGSQL_CLIENT_VERSION=v3_1_0 -jar $HOME/gsql-client/gsql_client.jar"
function gsql () {
     $gsql_command -u $username -p $password "$@"
}

function usage() {
    echo "Usage: $0 -u TG-username -p TG-password [optional options]"
    echo "Optional options:"
    echo "  -d numDevices       : number of FPGAs needed (default=1)"
    echo "  -g graphName        : graph name (default=xgraph_<username>"
    echo "  -i sshKey           : SSH key for user tigergraph"    
    echo "  -l 0|1              : 0: Do not load FPGA; 1: Load FPGA(default)>"
    echo "  -m numNodes         : Number of nodes in Tigergraph cluster"
    echo "  -n partitionProject : Graph partition project basename. "
    echo "  -s dataSource       : .mtx containing input graph. default=./as-Skitter/as-Skitter-wt-e110k.mtx"
    echo "  -v                  : Print verbose messages"
    echo "  -h                  : Print this help message"
}

# default values for optional options
hostname=$(hostname)
username=$USER
password=Xilinx123
data_source="$script_dir/as-skitter/as-skitter-wt-e110k.mtx"
data_source_set=0
load_fpga=1
num_devices=1
num_nodes=1
partition_prj="$script_dir/as-skitter/as-skitter-partitions/louvain_partitions"
verbose=0


# set default ssh_key for tigergraph
if [ -f ~/.ssh/tigergraph_rsa ]; then
    ssh_key_flag="-i ~/.ssh/tigergraph_rsa"
fi

while getopts ":d:g:i:l:m:n:p:s:u:vh" opt
do
case $opt in
    d) num_devices=$OPTARG;;
    g) xgraph=$OPTARG;;
    i) ssh_key=$OPTARG; ssh_key_flag="-i $ssh_key";;
    l) load_fpga=$OPTARG;;
    m) num_nodes=$OPTARG;;
    n) partition_prj=$OPTARG;;
    p) password=$OPTARG;;
    s) data_source=$OPTARG; data_source_set=1;;
    u) username=$OPTARG;;
    v) verbose=1; verbose_flag=-v;;
    h) usage; exit 1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

if [ -z "$username" ] || [ -z "$password" ]; then
    echo "ERROR: username and password are required."
    usage
    exit 2
fi

# create partitions if needed
if [ $data_source_set -eq 0 ]; then
    if [ ! -f "$script_dir/as-skitter/as-skitter-partitions/louvain_partitions.par.proj" ]; then
        echo "INFO: Creating graph partitions"
        ../../../../L3/tests/Louvain/create_partitions.sh $script_dir/as-skitter/as-skitter-wt-e110k.mtx as-skitter-partitions 9
    fi
fi

# need to download gsql client first before using it to check for other error conditions
if [ ! -f "$HOME/gsql-client/gsql_client.jar" ]; then
    mkdir -p $HOME/gsql-client
    wget -o wget.log -O $HOME/gsql-client/gsql_client.jar \
        'https://dl.bintray.com/tigergraphecosys/tgjars/com/tigergraph/client/gsql_client/3.1.0/gsql_client-3.1.0.jar'
    echo "INFO: Downloaded the latest gsql client"
fi

if [ $($gsql_command "show user" | grep -c $username) -lt 1 ]; then
    echo "ERROR: TigerGraph user $username does not exist."
    echo "Please create the user by logging in as user tigergraph and doing:"
    echo "    gsql \"create user\""
    echo "supplying $username for User Name."
    echo "Additionally, if you plan on creating graphs and installing queries, please also do:"
    echo "    gsql \"grant role globaldesigner to $username\""
    exit 3
fi

# $4 graph-name
if [ -z "$xgraph" ]; then
    xgraph="xgraph_$username"
fi

if [ $verbose -eq 1 ]; then
    echo "INFO: username=$username"
    echo "      password=$password"
    echo "      data_source=$data_source"
    echo "      partition_prj=$partition_prj"
    echo "      xgraph=$xgraph"
    echo "      load_cache=$load_cache"
    echo "      load_fpga=$load_fpga"
    echo "      num_nodes=$num_nodes"
    echo "      num_devices=$num_devices"
    echo "      ssh_key=$ssh_key"
    echo "      hostname=$hostname"
fi




