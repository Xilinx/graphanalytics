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

gsql_command="java -jar $HOME/gsql_client/gsql_client.jar"
function gsql () {
     $gsql_command "$@"
}

function usage() {
    echo "Usage: $0 -u TG-username -p TG-password [optional options]"
    echo "Optional options:"
    echo "  -c compileMode       : 0: skip database creation and gsql compilation"
    echo "                         1: recreate database and compile all (default)"
    echo "                         2: only compile query gsql"
    echo "  -r runMode           : 0: Skip both CPU and Alveo run (i.e. only run partition)"
    echo "                         1: Run only on CPU"
    echo "                         2: Run only on Alveo (default)"
    echo "                         3: Run on both CPU and Alveo"
    echo "  -d numDevices        : number of FPGAs needed (default=1)"
    echo "  -f                   : Force (re)install"
    echo "  -g graphName         : graph name (default=travelplan_<username>)"
    echo "  -i sshKey            : SSH key for user tigergraph"    
    echo "  -t truckData         : A csv file with travelPlan and truck data. default=../data/HasTravelPlans.csv"
    echo "  -w woData            : A csv file with travelPlan and work order data. default=../data/HasWorkOrder.csv"
    echo "  -v                   : Print verbose messages"
    echo "  -h                   : Print this help message"
}

tg_home=$(readlink -f ~tigergraph)
tg_data_root=$(cat $tg_home/.tg.cfg | jq .System.DataRoot | tr -d \")

# default values for optional options
username=$USER
password=Xilinx123
tp2tr_data="$script_dir/../data/travelplan2trucks100.csv"
tp2wo_data="$script_dir/../data/travelplan2workorders100.csv"
verbose=0
xgraph="travelplan_$username"
force_clean=0
compile_mode=1
run_mode=2
force_clean_flag=
verbose_flag=

# set default ssh_key for tigergraph
if [ -f ~/.ssh/tigergraph_rsa ]; then
    ssh_key_flag="-i ~/.ssh/tigergraph_rsa"
fi

while getopts "c:fg:i:m:p:r:t:w:u:vh" opt
do
case $opt in
    c) compile_mode=$OPTARG;;
    f) force_clean=1; force_clean_flag=-f;;
    g) xgraph=$OPTARG;;
    i) ssh_key=$OPTARG; ssh_key_flag="-i $ssh_key";;
    r) run_mode=$OPTARG;;
    p) password=$OPTARG;;
    t) tp2tr_data=$OPTARG;;
    w) tp2wo_data=$OPTARG;;
    u) username=$OPTARG;;
    v) verbose=1; verbose_flag=-v;;
    h) usage; exit 0;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

if [ -z "$username" ] || [ -z "$password" ]; then
    echo "ERROR: username and password are required."
    usage
    exit 2
fi

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
    echo "      tp2tr_data=$tp2tr_data"
    echo "      tp2wo_data=$tp2wo_data"
    echo "      xgraph=$xgraph"
    echo "      compileMode=$compile_mode"
    echo "      runMode=$run_mode"    
    echo "      sshKey=$ssh_key"
fi
