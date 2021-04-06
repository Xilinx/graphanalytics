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
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

###############################################################################

function usage() {
    echo "Usage: $0 [optional options]"
    echo "Optional options:"
    echo "  -a mem_alloc     : Change memory allocator default=jemalloc. " 
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

cosineSimPath=$SCRIPTPATH/../../cosinesim/staging
# TODO: add option to switch between local sandbox and official installation

force_clean=0
xrt_profiling=0
dev_mode=0
use_tcmalloc=0
mem_alloc="jemalloc"

while getopts ":r:m:a:dfghp" opt
do
case $opt in
    a) mem_alloc=$OPTARG;;
    d) dev_mode=1;;
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
echo "      mem_alloc=$mem_alloc"
echo "      dev_mode=$dev_mode"
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
echo "INFO: Found TigerGraph installation in $tg_root_dir"
echo "INFO: TigerGraph TEMP root is $tg_temp_root"
echo "INFO: Home is $HOME"

# Install dir for TigerGraph plugins
tg_udf_dir=$tg_root_dir/dev/gdk/gsql/src/QueryUdf

# Temporary directory of include files to be included into main UDF header (ExprFunctions.hpp)
tg_temp_include_dir=$tg_temp_root/gsql/codegen/udf

# Source directory for TigerGraph plugin
plugin_src_dir=$SCRIPTPATH/QueryUdf

###############################################################################
echo "INFO: Checking Xilinx library installation"
if [ ! -f "$tg_udf_dir/xclbin/denseSimilarityKernel.xclbin" ]; then
    printf "${RED}ERROR: Xilinx library and XCLBIN files not found.\n"
    printf "${YELLOW}INFO: Please download Xilinx library installation package "
    printf "from Xilinx Database PoC site: https://www.xilinx.com/member/dba_poc.html${NC}\n"
    exit 1
fi
echo "INFO: Found Xilinx XCLBIN files installed in $tg_udf_dir/xclbin/"

# save a copy of the original UDF Files

if [ ! -d "$tg_udf_dir.orig" ]; then
    cp -r $tg_udf_dir $tg_udf_dir.orig
    echo "Saved a copy of the original QueryUdf files in $tg_udf_dir.orig"
fi

# If the existing ExprFunctions.hpp has not been prepared for plugins, replace it
# with the base prepared version (containing just TG-supplied UDFs)

if [ ! -f $tg_udf_dir/ExprFunctions.hpp ] || [ $(grep -c mergeHeaders $tg_udf_dir/ExprFunctions.hpp) -eq 0 ]; then
    cp -f $plugin_src_dir/ExprFunctions.hpp $tg_udf_dir
fi


source $xrtPath/setup.sh
source $xrmPath/setup.sh

# Install plugin to ExprFunctions.hpp file

#if [ ! -f "$tg_udf_dir/mergeHeaders.py" ]; then
    cp $plugin_src_dir/mergeHeaders.py $tg_udf_dir
#fi
mv $tg_udf_dir/ExprFunctions.hpp $tg_udf_dir/ExprFunctions.hpp.prev
python3 $tg_udf_dir/mergeHeaders.py $tg_udf_dir/ExprFunctions.hpp.prev $plugin_src_dir/xilinxRecomEngineNew.hpp \
     > $tg_udf_dir/ExprFunctions.hpp

# Copy files to TigerGraph UDF area

cp $cosineSimPath/lib/libXilinxCosineSim.so $tg_udf_dir
cp $cosineSimPath/include/cosinesim.hpp $tg_udf_dir
cp $cosineSimPath/src/cosinesim_loader.cpp $tg_udf_dir
cp $plugin_src_dir/codevector.hpp $tg_udf_dir
cp $plugin_src_dir/xilinxRecomEngineImpl.hpp $tg_udf_dir

tg_xclbin_path=$tg_udf_dir/xclbin/denseSimilarityKernel.xclbin
sed -i "s|TG_COSINESIM_XCLBIN|$tg_xclbin_path|" $tg_udf_dir/xilinxRecomEngine.hpp

# Copy include files used by UDFs to TG build directory for TigerGraph to build the UDF library

mkdir -p $tg_temp_include_dir
cp $tg_udf_dir/ExprFunctions.hpp $tg_temp_include_dir
cp $tg_udf_dir/ExprUtil.hpp $tg_temp_include_dir
cp $tg_udf_dir/cosinesim.hpp $tg_temp_include_dir
cp $tg_udf_dir/cosinesim_loader.cpp $tg_temp_include_dir
cp $tg_udf_dir/codevector.hpp $tg_temp_include_dir
cp $tg_udf_dir/xilinxRecomEngineImpl.hpp $tg_temp_include_dir


# Remove old objects
# TODO: remove this after all Xilinx lab machines have been updated with this new install script
#rm -f $tg_root_dir/dev/gdk/objs/gq_*.o
#rm -f $tg_root_dir/bin/libudf/*.so
#timestamp=$(date "+%Y%m%d-%H%M%S")


if [ "$dev_mode" -eq 0 ]; then
    echo "INFO: Apply environment changes to TigerGraph installation"
    gadmin start all

    ld_preload="$tg_udf_dir/libXilinxCosineSim.so"
    if [ "$mem_alloc" == "tcmalloc" ]; then
        ld_preload="$ld_preload:$tg_root_dir/dev/gdk/gsdk/include/thirdparty/prebuilt/dynamic_libs/gmalloc/tcmalloc/libtcmalloc.so"
    fi

    ld_lib_path="$HOME/libstd:$tg_udf_dir:/opt/xilinx/xrt/lib:/opt/xilinx/xrm/lib:/usr/lib/x86_64-linux-gnu/"

    if [ "$xrt_profiling" -eq 1 ]; then
        gpe_config="LD_PRELOAD=\$LD_PRELOAD:$ld_preload;LD_LIBRARY_PATH=$ld_lib_path:\$LD_LIBRARY_PATH;CPUPROFILE=/tmp/tg_cpu_profiler;CPUPROFILESIGNAL=12;MALLOC_CONF=prof:true,prof_active:false;XILINX_XRT=/opt/xilinx/xrt;XILINX_XRM=/opt/xilinx/xrm;XRT_INI_PATH=$PWD/../scripts/debug/xrt-profile.ini"
    else
        gpe_config="LD_PRELOAD=\$LD_PRELOAD:$ld_preload;LD_LIBRARY_PATH=$ld_lib_path:\$LD_LIBRARY_PATH;CPUPROFILE=/tmp/tg_cpu_profiler;CPUPROFILESIGNAL=12;MALLOC_CONF=prof:true,prof_active:false;XILINX_XRT=/opt/xilinx/xrt;XILINX_XRM=/opt/xilinx/xrm"
    fi

    gadmin config set GPE.BasicConfig.Env "$gpe_config"

    echo "INFO: Apply the new configurations to $gpe_config"
    gadmin config apply -y
    gadmin restart gpe -y
    gadmin config get GPE.BasicConfig.Env
fi

echo ""
echo "INFO: Xilinx FPGA acceleration plugin for Tigergraph has been installed."

