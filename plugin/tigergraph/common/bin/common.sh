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

set -x 
SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

RED='\033[0;31m'
NC='\033[0m' # No Color

if ! [ -x "$(command -v jq)" ]; then
    echo "ERROR: The program jq is required. Please follow the instructions below to install it:"
    echo "       RedHat/CentOS: sudo yum install jq"
    echo "       Ubuntu: sudo apt-get install jq"
    exit 1
fi

if ! [ -x "$(command -v python3)" ]; then
    echo "ERROR: Cannot find python3 in path."
    exit 1
fi

if [ ! -f "$HOME/.tg.cfg" ]; then
    echo "ERROR: This script only supports TigerGraph version 3.x"
    echo "INFO: Installed version:"
    gadmin version | grep TigerGraph
    exit 1
fi
tg_root_dir=$(cat $HOME/.tg.cfg | jq .System.AppRoot | tr -d \")
tg_temp_root=$(cat $HOME/.tg.cfg | jq .System.TempRoot | tr -d \")
tg_data_root=$(cat $HOME/.tg.cfg | jq .System.DataRoot | tr -d \")

# set up PATH for tigergraph commands
export PATH=$tg_root_dir/../cmd:$PATH

# TigerGraph UDF directory
tg_app_udf_dir=$tg_root_dir/dev/gdk/gsql/src/QueryUdf
# Plugin UDF installation directory
tg_udf_dir=$tg_data_root/gsql/udf
mkdir -p $tg_udf_dir
tg_udf_xclbin_dir=$tg_udf_dir/xclbin

plugin_ld_preload=

# Set up variables and checks specific to the plugin

. $SCRIPTPATH/set-plugin-vars.sh

# Use the local git repo for Alveo Product artifacts if the repo dir exists;
# otherwise, use the artifacts from the installed Alveo Product (typically under /opt/xilinx)

pluginAlveoProductNeedsInstall=0
pluginAlveoProductPath=$pluginInstalledAlveoProductPath

if [ -d $pluginLocalAlveoProductPath ]; then
    pluginAlveoProductNeedsInstall=1
    pluginAlveoProductPath=$pluginLocalAlveoProductPath;
fi

# Assume that the Alveo Product XCLBIN and .so will be used directly from their directories

pluginAlveoProductXclbinPath=$pluginAlveoProductPath/xclbin/$pluginXclbinName
pluginAlveoProductXclbinPathU55C=$pluginAlveoProductPath/xclbin/$pluginXclbinNameU55C
pluginAlveoProductLibDir=$pluginAlveoProductPath/lib

runtimeXclbinPath=$pluginAlveoProductXclbinPath
runtimeXclbinPathU55C=$pluginAlveoProductXclbinPathU55C
runtimeLibDir=$pluginAlveoProductLibDir

# if the Alveo Product artifacts need installing (because the local repo may not be
# accessible from the TigerGraph server), the point to the installed location instead

if [ $pluginAlveoProductNeedsInstall -eq 1 ]; then
    pluginAlveoProductLibPath=$tg_udf_dir
    runtimeXclbinPath=$tg_udf_xclbin_dir/$pluginXclbinName
    runtimeXclbinPathU55C=$tg_udf_xclbin_dir/$pluginXclbinNameU55C
    runtimeLibDir=$tg_udf_dir
fi

# Make sure the XCLBIN exists (unless we're uninstalling)

if [ $uninstall -eq 0 ] && [ ! -f $pluginAlveoProductXclbinPath ]; then
    printf "${RED}ERROR: $standaloneAlveoProductName Alveo U50 product not found.${NC}\n"
    printf "INFO: Please download $standaloneAlveoProductName Alveo product installation package "
    printf "from Xilinx Database PoC site: https://www.xilinx.com/member/dba_poc.html\n"
    exit 1
fi

if [ $uninstall -eq 0 ] && [ ! -z "$pluginXclbinNameU55C" ] && [ ! -f $pluginAlveoProductXclbinPathU55C ]; then
    printf "${RED}ERROR: $standaloneAlveoProductName Alveo U55C product not found.${NC}\n"
    printf "INFO: Please download $standaloneAlveoProductName Alveo product installation package "
    printf "from Xilinx Database PoC site: https://www.xilinx.com/member/dba_poc.html\n"
    exit 1
fi
