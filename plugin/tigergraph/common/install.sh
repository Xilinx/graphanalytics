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
PRODUCT_VER=`cat $SCRIPTPATH/VERSION`

# Read plugin specific variables
. $SCRIPTPATH/bin/set-plugin-vars.sh

function usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -i sshKey        : SSH key for user tigergraph"    
    echo "  -a mem_alloc     : Change memory allocator default=jemalloc. " 
    echo "  -d device-name   : Specify Alveo device name. Supported devices: "

    for dev in $pluginSupportedDevices
    do
        echo "                     $dev"
    done

    echo "  -f               : Force installation"
    echo "  -g               : Build plugin libraries with __DEBUG__"
    echo "  -p               : Turn on XRT profiling and timetrace"
    echo "  -u               : Uninstall $pluginAlveoProductName"
    echo "  -v               : Verbose output"
    echo "  -h               : Print this help message"
}

hostname=$(hostname)
# set default ssh_key for tigergraph
if [ -f ~/.ssh/tigergraph_rsa ]; then
    ssh_key_flag="-i ~/.ssh/tigergraph_rsa"
fi

flags=
uninstall=0
device="notset"
while getopts ":i:uha:d:fghpuv" opt
do
case $opt in
    i) ssh_key=$OPTARG; ssh_key_flag="-i $ssh_key";;
    a) flags="$flags -$opt $OPTARG";;
    d) device=$OPTARG; flags="$flags -$opt $device";;
    f|g|p|v) flags="$flags -$opt";;
    u) uninstall=1; flags="$flags -$opt";;
    h) usage; exit 1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

if [[ $uninstall -eq 1 ]]; then
    echo "INFO: Uninstall $pluginAlveoProductName"
elif [[ "$device" == "notset" ]] ; then
    echo "ERROR: Alveo device name must be set via -d option."
    usage
    exit 2
else
    echo "--------------------------------------------------------------------------------"
    echo "INFO: Install $pluginAlveoProductName plugin targetting $device"
    echo "--------------------------------------------------------------------------------"
fi

if [[ "$USER" == "tigergraph" ]]; then
    ${SCRIPTPATH}/bin/install-plugin-cluster.sh $flags
else
    echo "--------------------------------------------------------------------------------"
    echo "INFO: Running installation as user \"tigergraph\" with $flags. "
    echo "Enter password for \"tigergraph\" if prompted."
    echo "INFO: command=ssh $ssh_key_flag tigergraph@$hostname ${SCRIPTPATH}/bin/install-plugin-cluster.sh $flags"    
    echo "--------------------------------------------------------------------------------"
    ssh $ssh_key_flag tigergraph@$hostname ${SCRIPTPATH}/bin/install-plugin-cluster.sh $flags
fi

if [[ $uninstall -ne 1 ]]; then
    # copy gsql_client.jar to current user's home
    mkdir -p $HOME/gsql_client
    cp /tmp/gsql_client.jar.tmp $HOME/gsql_client/gsql_client.jar
    echo "INFO: Remote GSQL client has been installed in $HOME/gsql_client"
fi

if [ -r $SCRIPTPATH/bin/install-plugin-cluster-custom.sh ]; then
    echo "INFO: Installing product specific features: $SCRIPTPATH/bin/install-plugin-cluster-custom.sh"
    $SCRIPTPATH/bin/install-plugin-cluster-custom.sh
fi