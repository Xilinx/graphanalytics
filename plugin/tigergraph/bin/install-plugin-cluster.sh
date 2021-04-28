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
NC='\033[0m' # No Color

if [ "$USER" != "tigergraph" ]; then
    echo "ERROR: This script must be run as user tigergraph."
    exit 1
fi

xrtPath=/opt/xilinx/xrt
xrmPath=/opt/xilinx/xrm
mem_alloc="jemalloc"
dev_mode=0
debug_flag=0
xrt_profiling=0
uninstall=0
verbose=0
node_flags=
while getopts ":a:dfgm:pr:uv" opt
do
case $opt in
    a) mem_alloc=$OPTARG;;
    d) dev_mode=1; node_flags+=" -d";;
    f) node_flags+=" -f";;
    g) debug_flag=1; node_flags+=" -g";;
    m) xrmPath=$OPTARG; node_flags+=" -m $OPTARG";;
    p) xrt_profiling=1;;
    r) xrtPath=$OPTARG; node_flags+=" -r $OPTARG";;
    u) uninstall=1; node_flags+=" -u";;
    v) verbose=1; node_flags+=" -v";;
    ?) echo "ERROR: Unknown option -$OPTARG"; exit 1;;  # pass through to sub-script
esac
done

. $SCRIPTPATH/common.sh

if ! [ -x "$(command -v gadmin)" ]; then
    echo "ERROR: Cannot find TigerGraph installation. Please add the gadmin command to user tigergraph's path."
    exit 2
fi

if [ $verbose -eq 1 ]; then
    echo "INFO: Cluster script is running with the settings below:"
    echo "      mem_alloc=$mem_alloc"
    echo "      dev_mode=$dev_mode"
    echo "      xrtPath=$xrtPath"
    echo "      xrmPath=$xrmPath"
    echo "      force_clean=$force_clean"
    echo "      debug_flag=$debug_flag"
    echo "      xrt_profiling=$xrt_profiling"
    echo "      uninstall=$uninstall"
fi

echo ""
echo "INFO: Found TigerGraph installation in $tg_root_dir"
echo "INFO: TigerGraph TEMP root is $tg_temp_root"
echo "INFO: Home is $HOME"


grun all "${SCRIPTPATH}/install-plugin-node.sh $*"

if [ $dev_mode -eq 1 ]; then
    echo "INFO: Development mode -- skipping TigerGraph configuration."
else
    if [ $uninstall -eq 1 ]; then
        echo "INFO: Restarting GPE service"
        gadmin restart gpe -y
    else
        echo "INFO: Apply environment changes to TigerGraph installation"
        gadmin start all

        ld_preload="$targetCosineSimLibPath/libXilinxCosineSim.so"
        if [ "$mem_alloc" == "tcmalloc" ]; then
            ld_preload="$ld_preload:$tg_root_dir/dev/gdk/gsdk/include/thirdparty/prebuilt/dynamic_libs/gmalloc/tcmalloc/libtcmalloc.so"
        fi

        ld_lib_path="$HOME/libstd:$targetCosineSimLibPath:/opt/xilinx/xrt/lib:/opt/xilinx/xrm/lib:/usr/lib/x86_64-linux-gnu/"

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
fi

# Install or uninstall Recommendation Engine tuples
if [ $uninstall -eq 1 ]; then
    echo "INFO: Removing Recommendation Engine tuples"
    if [ $(gsql "LS" | grep -c XilCosinesimMatch) -gt 0 ]; then
        gsql "DROP TUPLE XilCosinesimMatch"
    fi
    echo ""
    echo "INFO: Xilinx FPGA acceleration plugin for Tigergraph has been uninstalled."
else
    echo "INFO: Adding Recommendation Engine tuples"
    if [ $(gsql "LS" | grep -c XilCosinesimMatch) -lt 1 ]; then
        gsql "TYPEDEF TUPLE<Id VERTEX, score double> XilCosinesimMatch"
    fi
    echo ""
    echo "INFO: Xilinx FPGA acceleration plugin for Tigergraph has been installed."
fi

