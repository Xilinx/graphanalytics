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
SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

###############################################################################

function usage() {
    echo "Usage: $0 [optional options]"
    echo "Optional options:"
    echo "  -d               : Install plugin in development mode and bypass resetting Tigergraph services."
    echo "  -f               : Force cleaning plugin libraries"
    echo "  -g               : Build plugin libraries with __DEBUG__"
    echo "  -m xrm-lib-path  : Path to XRM libraries. default=/opt/xilinx/xrm"
    echo "  -p               : Turn on XRT profiling and timetrace"
    echo "  -r xrt-lib-path  : Path to XRT libraries. default=/opt/xilinx/xrt"
    echo "  -h               : Print this help message"
}

# script options processing
xrtPath=/opt/xilinx/xrt
xrmPath=/opt/xilinx/xrm

cosineSimPath=$SCRIPTPATH/../cosinesim/staging
# TODO: add option to switch between local sandbox and official installation

force_clean=0
xrt_profiling=0
dev_mode=0

while getopts ":r:m:dfghp" opt
do
case $opt in
    d) dev_mode=1; echo "INFO: Option set: Install plugin in development mode";;
    f) force_clean=1; echo "INFO: Option set: Force rebuidling plugin libraries";;
    g) debug_flag="DEBUG=1"; echo "INFO: debug_flag=$debug_flag";;
    m) xrmPath=$OPTARG;;
    r) xrtPath=$OPTARG;;
    p) xrt_profiling=1;;
    h) usage; exit 1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

echo "INFO: Script is running with the settings below:"
echo "      xrtPath=$xrtPath"
echo "      xrmPath=$xrmPath"
echo "      force_clean=$force_clean"
echo "      debug_flag=$debug_flag"
echo "      xrt_profiling=$xrt_profiling"

###############################################################################

echo "INFO: Checking TigerGraph installation version and directory"
if ! [ -x "$(command -v jq)" ]; then
    echo "ERROR: The program jq is required. Please follow the instructions below to install it:"
    echo "       RedHat/CentOS: sudo yum install jq"
    echo "       Ubuntu: sudo apt-get install jq"
    exit 1
fi

if ! [ -x "$(command -v gadmin)" ]; then
    echo "ERROR: Cannot find TigerGraph installation. Please run this install script as the user for the TigerGraph installation."
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
echo "INFO: Found TigerGraph installation in $tg_root_dir"
echo "INFO: TigerGraph TEMP root is $tg_temp_root"
echo "INFO: Home is $HOME"

###############################################################################
echo "INFO: Checking Xilinx library installation"
if [ ! -f "$tg_root_dir/dev/gdk/gsql/src/QueryUdf/xclbin/denseSimilarityKernel.xclbin" ]; then
    printf "${RED}ERROR: Xilinx library and XCLBIN files not found.\n"
    printf "${YELLOW}INFO: Please download Xilinx library installation package "
    printf "from Xilinx Database PoC site: https://www.xilinx.com/member/dba_poc.html${NC}\n"
    exit 1
fi
echo "INFO: Found Xilinx XCLBIN files installed in $tg_root_dir/dev/gdk/gsql/src/QueryUdf/xclbin/"

# Prepare UDF files
mkdir -p $tg_temp_root/QueryUdf
rm -rf $tg_temp_root/QueryUdf/tgFunctions.hpp
rm -rf $tg_temp_root/QueryUdf/ExprFunctions.hpp
rm -rf $tg_temp_root/QueryUdf/ExprUtil.hpp
rm -rf $tg_temp_root/QueryUdf/graph.hpp

# save a copy of the original UDF Files
if [ ! -d "$tg_root_dir/dev/gdk/gsql/src/QueryUdf.orig" ]; then
    cp -r $tg_root_dir/dev/gdk/gsql/src/QueryUdf $tg_root_dir/dev/gdk/gsql/src/QueryUdf.orig
    echo "Saved a copy of the original QueryUdf files in $tg_root_dir/gdk/gsql/src/QueryUdf.orig"
fi

# prepare UDF files
cp $tg_root_dir/dev/gdk/gsql/src/QueryUdf.orig/ExprFunctions.hpp $tg_temp_root/QueryUdf/tgFunctions.hpp
cp $tg_root_dir/dev/gdk/gsql/src/QueryUdf.orig/ExprUtil.hpp $tg_temp_root/QueryUdf/
cp $SCRIPTPATH/tigergraph/QueryUdf/xilinxUdf.hpp $tg_temp_root/QueryUdf/ExprFunctions.hpp
cp $SCRIPTPATH/../L3/include/graph.hpp $tg_temp_root/QueryUdf/

source $xrtPath/setup.sh
source $xrmPath/setup.sh

# make L3 wrapper library
if [ "$force_clean" -eq 1 ]
then
    echo "INFO: make TigerGraphPath=$tg_root_dir TigerGraphTemp=$tg_temp_root clean"
    cd $SCRIPTPATH && make TigerGraphPath=$tg_root_dir TigerGraphTemp=$tg_temp_root clean
fi

cd $SCRIPTPATH && make $debug_flag TigerGraphPath=$tg_root_dir \
                       TigerGraphTemp=$tg_temp_root libgraphL3wrapper

# copy files to $tg_rrot_dir UDF area
mkdir -p $tg_temp_root/gsql/codegen/udf
timestamp=$(date "+%Y%m%d-%H%M%S")
#rm -rf $tg_install_dir/tigergraph/dev/gdk/gsql/src/QueryUdf
cp $tg_temp_root/QueryUdf/ExprFunctions.hpp $tg_root_dir/dev/gdk/gsql/src/QueryUdf/
cp $tg_temp_root/QueryUdf/ExprUtil.hpp $tg_root_dir/dev/gdk/gsql/src/QueryUdf/

cp $SCRIPTPATH/tigergraph/QueryUdf/codevector.hpp $tg_root_dir/dev/gdk/gsql/src/QueryUdf/
cp $SCRIPTPATH/tigergraph/QueryUdf/loader.hpp $tg_root_dir/dev/gdk/gsql/src/QueryUdf/
cp $tg_temp_root/QueryUdf/tgFunctions.hpp $tg_root_dir/dev/gdk/gsql/src/QueryUdf/
cp $tg_temp_root/QueryUdf/graph.hpp $tg_root_dir/dev/gdk/gsql/src/QueryUdf/
cp $SCRIPTPATH/tigergraph/QueryUdf/core.cpp $tg_root_dir/dev/gdk/gsql/src/QueryUdf/

tg_xclbin_path=$tg_root_dir/dev/gdk/gsql/src/QueryUdf/xclbin/denseSimilarityKernel.xclbin
cp $SCRIPTPATH/tigergraph/QueryUdf/xilinxRecomEngine.hpp $tg_root_dir/dev/gdk/gsql/src/QueryUdf/
sed -i "s|TG_COSINESIM_XCLBIN|$tg_xclbin_path|" $tg_root_dir/dev/gdk/gsql/src/QueryUdf/xilinxRecomEngine.hpp

cp $cosineSimPath/include/cosinesim.hpp $tg_root_dir/dev/gdk/gsql/src/QueryUdf/
cp $cosineSimPath/src/cosinesim_loader.cpp $tg_root_dir/dev/gdk/gsql/src/QueryUdf/

# Include files for TigerGraph to build the UDF library
cp $tg_temp_root/QueryUdf/tgFunctions.hpp $tg_temp_root/gsql/codegen/udf
##cp $SCRIPTPATH/tigergraph/QueryUdf/loader.hpp $tg_temp_root/gsql/codegen/udf
#cp $tg_temp_root/QueryUdf/graph.hpp $tg_temp_root/gsql/codegen/udf
cp $cosineSimPath/include/cosinesim.hpp $tg_temp_root/gsql/codegen/udf
cp $SCRIPTPATH/tigergraph/QueryUdf/codevector.hpp $tg_temp_root/gsql/codegen/udf
cp $cosineSimPath/src/cosinesim_loader.cpp $tg_temp_root/gsql/codegen/udf
cp $SCRIPTPATH/tigergraph/QueryUdf/xilinxRecomEngine.hpp $tg_temp_root/gsql/codegen/udf
sed -i "s|TG_COSINESIM_XCLBIN|$tg_xclbin_path|" $tg_temp_root/gsql/codegen/udf/xilinxRecomEngine.hpp


cp $SCRIPTPATH/tigergraph/QueryUdf/*.json $tg_root_dir/dev/gdk/gsql/src/QueryUdf/
cp $tg_temp_root/QueryUdf/libgraphL3wrapper.so $tg_root_dir/dev/gdk/gsql/src/QueryUdf/
cp $cosineSimPath/lib/libXilinxCosineSim.so $tg_root_dir/dev/gdk/gsql/src/QueryUdf/

# Use default MakeUdf since we have no .o's to include
# TODO: remove this after no branch is left that installs a custom MakeUdf
cp $SCRIPTPATH/tigergraph/MakeUdf.orig $tg_root_dir/dev/gdk/MakeUdf
## cp $tg_root_dir/dev/gdk/MakeUdf $tg_root_dir/dev/gdk/MakeUdf-$timestamp
## cp $SCRIPTPATH/tigergraph/MakeUdf $tg_root_dir/dev/gdk/

# update files with tg_root_dir
for f in $tg_root_dir/dev/gdk/gsql/src/QueryUdf/*.json; do
    # use | as the demiliter since tg_root_dir has / in it
    sed -i "s|TG_ROOT_DIR|$tg_root_dir|" $f 
done
## sed -i "s|TG_ROOT_DIR|$tg_root_dir|" $tg_root_dir/dev/gdk/MakeUdf 

if [ "$dev_mode" -eq 0 ]; then
    echo "INFO: Apply environment changes to TigerGraph installation"
    gadmin start all
    if [ "$xrt_profiling" -eq 1 ]; then
        gpe_config="LD_PRELOAD=\$LD_PRELOAD;LD_LIBRARY_PATH=$HOME/libstd:$tg_root_dir/dev/gdk/gsql/src/QueryUdf/:/opt/xilinx/xrt/lib:/opt/xilinx/xrm/lib:/usr/lib/x86_64-linux-gnu/:\$LD_LIBRARY_PATH;CPUPROFILE=/tmp/tg_cpu_profiler;CPUPROFILESIGNAL=12;MALLOC_CONF=prof:true,prof_active:false;XILINX_XRT=/opt/xilinx/xrt;XILINX_XRM=/opt/xilinx/xrm;XRT_INI_PATH=$PWD/../scripts/debug/xrt-profile.ini"
    else
        gpe_config="LD_PRELOAD=\$LD_PRELOAD;LD_LIBRARY_PATH=$HOME/libstd:$tg_root_dir/dev/gdk/gsql/src/QueryUdf/:/opt/xilinx/xrt/lib:/opt/xilinx/xrm/lib:/usr/lib/x86_64-linux-gnu/:\$LD_LIBRARY_PATH;CPUPROFILE=/tmp/tg_cpu_profiler;CPUPROFILESIGNAL=12;MALLOC_CONF=prof:true,prof_active:false;XILINX_XRT=/opt/xilinx/xrt;XILINX_XRM=/opt/xilinx/xrm"
    fi
    gadmin config set GPE.BasicConfig.Env "$gpe_config"

    echo "Apply the new configurations"
    gadmin config apply -y
    gadmin restart gpe -y
    gadmin config get GPE.BasicConfig.Env
fi

echo ""
echo "INFO: Xilinx FPGA acceleration plugin for Tigergraph has been installed."

