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

# Install dir for TigerGraph plugins
tg_udf_dir=$tg_root_dir/dev/gdk/gsql/src/QueryUdf
tg_udf_xclbin_dir=$tg_udf_dir/xclbin

# Locate the right Cosinesim to use: installed package or local git sandbox

installedCosineSimPath=/opt/xilinx/apps/graphanalytics/cosinesim/1.0
cosineSimPath=$installedCosineSimPath
targetCosineSimLibPath=$cosineSimPath/lib

xclbinName=cosinesim_32bit_xilinx_u50_gen3x16_xdma_201920_3.xclbin
xclbinPath=$installedCosineSimPath/xclbin/$xclbinName
cosinesimNeedsTgInstall=0
targetXclbinPath=$xclbinPath

localCosineSimPath=$SCRIPTPATH/../../../cosinesim/staging
if [ -d $localCosineSimPath ]; then
    cosinesimNeedsTgInstall=1
    cosineSimPath=$localCosineSimPath;
    targetCosineSimLibPath=$tg_udf_dir
    xclbinPath=$localCosineSimPath/xclbin/$xclbinName
    targetXclbinPath=$tg_udf_xclbin_dir/$xclbinName
fi

# Make sure the XCLBIN exists (unless we're uninstalling)

if [ $uninstall -eq 0 ] && [ ! -f $xclbinPath ]; then
    printf "${RED}ERROR: Xilinx Cosine Similarity Alveo product not found.${NC}\n"
    printf "INFO: Please download Xilinx Cosine Similarity Alveo product installation package "
    printf "from Xilinx Database PoC site: https://www.xilinx.com/member/dba_poc.html\n"
    exit 1
fi
