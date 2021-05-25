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

gsql_command="java -jar $HOME/gsql_client/gsql_client.jar"
function gsql () {
     $gsql_command "$@"
}

function usage() {
    echo "Usage: $0 -u TG-username -p TG-password [optional options]"
    echo "Optional options:"
    echo "  -a alveoProject     : Alveo partition project basename "
    echo "  -c compileMode      : 0: recreate database and compile all; 1: only compile query gsql; 2: skip database creation and gsql compilation "
    echo "  -d numDevices       : number of FPGAs needed (default=1)"
    echo "  -f                  : Force (re)install"
    echo "  -g graphName        : graph name (default=social_<username>"
    echo "  -i sshKey           : SSH key for user tigergraph"    
    echo "  -l 0|1              : 0: Do not load FPGA; 1: Load FPGA(default)>"
    echo "  -m numNodes         : Number of nodes in Tigergraph cluster"
    echo "  -n numPartitions    : Number of Alveo partitions "
    echo "  -r runMode          : 0: Run only on CPU; 1: Run only on Alveo; 2: Run on both CPU and Alveo "
    echo "  -s dataSource       : A .mtx file containing input graph. default=./as-Skitter/as-Skitter-wt-e110k.mtx"
    echo "  -v                  : Print verbose messages"
    echo "  -x partitionMode    : 0: from TigerGraph memory; 1: from dataSource (.mtx); 2: load saved (alveo project)"
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
num_partitions=1
num_nodes=1
alveo_prj="$script_dir/as-skitter/as-skitter-partitions/louvain_partitions"
verbose=0
xgraph="social_$username"
force_clean=0
run_mode=0
compile_mode=0
partition_mode=1
force_clean_flag=
verbose_flag=

# set default ssh_key for tigergraph
if [ -f ~/.ssh/tigergraph_rsa ]; then
    ssh_key_flag="-i ~/.ssh/tigergraph_rsa"
fi

while getopts "a:c:d:fg:i:l:m:n:p:r:s:u:x:vh" opt
do
case $opt in
    a) alveo_prj=$OPTARG;;
    c) compile_mode=$OPTARG;;
    d) num_devices=$OPTARG;;
    f) force_clean=1; force_clean_flag=-f;;
    g) xgraph=$OPTARG;;
    i) ssh_key=$OPTARG; ssh_key_flag="-i $ssh_key";;
    l) load_fpga=$OPTARG;;
    m) num_nodes=$OPTARG;;
    n) num_partitions=$OPTARG;;
    p) password=$OPTARG;;
    r) run_mode=$OPTARG;;
    s) data_source=$OPTARG; data_source_set=1;;
    u) username=$OPTARG;;
    v) verbose=1; verbose_flag=-v;;
    x) partition_mode=$OPTARG;;
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
#if [ $data_source_set -eq 0 ]; then
#    if [ ! -f "$script_dir/as-skitter/as-skitter-partitions/louvain_partitions.par.proj" ]; then
#        echo "INFO: Creating graph partitions"
#        ../../../../../L3/tests/Louvain/create_partitions.sh $script_dir/as-skitter/as-skitter-wt-e110k.mtx as-skitter-partitions 9
#    fi
#fi

# need to download gsql client first before using it to check for other error conditions
#if [ ! -f "$HOME/gsql-client/gsql_client.jar" ]; then
#    mkdir -p $HOME/gsql-client
#    wget -o wget.log -O $HOME/gsql-client/gsql_client.jar \
#        'https://dl.bintray.com/tigergraphecosys/tgjars/com/tigergraph/client/gsql_client/3.1.0/gsql_client-3.1.0.jar'
#    echo "INFO: Downloaded the latest gsql client"
#fi

if [ $(gsql -u $username -p $password "show user" | grep -c $username) -lt 1 ]; then
    echo "ERROR: TigerGraph user $username does not exist."
    echo "Please create the user by logging in as user tigergraph and doing:"
    echo "    gsql \"create user\""
    echo "supplying $username for User Name."
    echo "Additionally, if you plan on creating graphs and installing queries, please also do:"
    echo "    gsql \"grant role globaldesigner to $username\""
    exit 3
fi

if [ $verbose -eq 1 ]; then
    echo "INFO: username=$username"
    echo "      password=$password"
    echo "      data_source=$data_source"
    echo "      alveo_prj=$alveo_prj"
    echo "      num_partitions=$num_partitions"
    echo "      xgraph=$xgraph"
    echo "      load_fpga=$load_fpga"
    echo "      num_nodes=$num_nodes"
    echo "      num_devices=$num_devices"
    echo "      run_mode=$run_mode"
    echo "      compile_mode=$compile_mode"
    echo "      partition_mode=$partition_mode"
    echo "      ssh_key=$ssh_key"
    echo "      hostname=$hostname"
fi




