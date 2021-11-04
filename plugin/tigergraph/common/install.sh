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

set -e

# Turn on tracing for debugging this script
#set -x

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

function usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -i sshKey        : SSH key for user tigergraph"    
    echo "  -a mem_alloc     : Change memory allocator default=jemalloc. " 
    echo "  -d device-name   : Specify Alveo device name. Valid values: "
    echo "                     xilinx_u50_gen3x16_xdma_201920_3"
	echo "                     xilinx_u55c_gen3x16_xdma_base_2"
    echo "                     xilinx_aws-vu9p-f1_shell-v04261818_201920_2"
    echo "  -f               : Force installation"
    echo "  -g               : Build plugin libraries with __DEBUG__"
    echo "  -m xrm-lib-path  : Path to XRM libraries. default=/opt/xilinx/xrm"
    echo "  -p               : Turn on XRT profiling and timetrace"
    echo "  -r xrt-lib-path  : Path to XRT libraries. default=/opt/xilinx/xrt"
    echo "  -u               : Uninstall Xilinx Recommendation Engine"
    echo "  -v               : Verbose output"
    echo "  -h               : Print this help message"
}

hostname=$(hostname)
# set default ssh_key for tigergraph
if [ -f ~/.ssh/tigergraph_rsa ]; then
    ssh_key_flag="-i ~/.ssh/tigergraph_rsa"
fi

flags=
while getopts ":i:uhr:m:a:d:fghpuv" opt
do
case $opt in
    i) ssh_key=$OPTARG; ssh_key_flag="-i $ssh_key";;
    a|d|m|r) flags="$flags -$opt $OPTARG";;
    f|g|p|u|v) flags="$flags -$opt";;
    h) usage; exit 1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

if [ "$USER" == "tigergraph" ]; then
    ${SCRIPTPATH}/bin/install-plugin-cluster.sh $flags
else
    echo "------------------------------------------------------------------------------------------------------------"
    echo "INFO: Running installation as user \"tigergraph\" with $flags. Enter password for \"tigergraph\" if prompted."
    echo "INFO: command=ssh $ssh_key_flag tigergraph@$hostname ${SCRIPTPATH}/bin/install-plugin-cluster.sh $flags"    
    echo "------------------------------------------------------------------------------------------------------------"
    ssh $ssh_key_flag tigergraph@$hostname ${SCRIPTPATH}/bin/install-plugin-cluster.sh $flags
fi

# copy gsql_client.jar to current user's home
mkdir -p $HOME/gsql_client
cp /tmp/gsql_client.jar.tmp $HOME/gsql_client/gsql_client.jar
echo "INFO: Remote GSQL client has been installed in $HOME/gsql_client"

if [ -r $SCRIPTPATH/bin/install-plugin-cluster-custom.sh ]; then
    echo "INFO: Installing product specific features: $SCRIPTPATH/bin/install-plugin-cluster-custom.sh"
    $SCRIPTPATH/bin/install-plugin-cluster-custom.sh
fi
