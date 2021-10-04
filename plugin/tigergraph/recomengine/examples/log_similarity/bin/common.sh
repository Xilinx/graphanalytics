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
    echo "  -c 0|1              : 0=Do not load cache; 1=Load cache(default)"
    echo "  -d numDevices       : number of FPGAs needed (default=1)"
    echo "  -f                  : Force (re)install"
    echo "  -n iterations       : number of iterations to run (default=1)"
    echo "  -g graphName        : graph name (default=xgraph_<username>"
    echo "  -i sshKey           : SSH key for user tigergraph"    
    echo "  -l 0|1              : 0: Do not load FPGA; 1: Load FPGA(default)>"
    echo "  -m numNodes         : Number of nodes in Tigergraph cluster"
    echo "  -s dataSourcePath   : path containing input data. default=./data"
    echo "  -v                  : Print verbose messages"
    echo "  -h                  : Print this help message"
}

# default values for optional options
hostname=$(hostname)
username=$USER
password=Xilinx123
data_root="$script_dir/data"
load_cache=1
load_fpga=1
num_devices=1
num_nodes=1
iterations=1
verbose=0
xgraph="xgraph_$username"


# set default ssh_key for tigergraph
if [ -f ~/.ssh/tigergraph_rsa ]; then
    ssh_key_flag="-i ~/.ssh/tigergraph_rsa"
fi

while getopts ":c:d:fg:i:l:m:n:p:s:u:vh" opt
do
case $opt in
    c) load_cache=$OPTARG;;
    d) num_devices=$OPTARG;;
    f) force_clean=1; force_clean_flag=-f;;
    g) xgraph=$OPTARG;;
    i) ssh_key=$OPTARG; ssh_key_flag="-i $ssh_key";;
    l) load_fpga=$OPTARG;;
    m) num_nodes=$OPTARG;;
    n) iterations=$OPTARG;;
    p) password=$OPTARG;;
    s) data_root=$OPTARG;;
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

# need to download gsql client first before using it to check for other error conditions
#if [ ! -f "$HOME/gsql_client/gsql_client.jar" ]; then
#    mkdir -p $HOME/gsql_client
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
    echo "      data_root=$data_root"
    echo "      xgraph=$xgraph"
    echo "      load_cache=$load_cache"
    echo "      load_fpga=$load_fpga"
    echo "      load_fpga=$num_nodes"
    echo "      num_devices=$num_devices"
    echo "      iterations=$iterations"
    echo "      ssh_key=$ssh_key"
    echo "      hostname=$hostname"
    echo "      force_clean=$force_clean"
fi




