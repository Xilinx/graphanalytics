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
echo $@

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

RED='\033[0;31m'
NC='\033[0m' # No Color

if [ "$USER" != "tigergraph" ]; then
    echo "ERROR: This script must be run as user tigergraph."
    exit 1
fi

mem_alloc="tcmalloc"
dev_mode=0
debug_flag=0
xrt_profiling=0
uninstall=0
verbose=0
node_flags=
force=0
while getopts ":a:dfgpuv" opt
do
case $opt in
    a) mem_alloc=$OPTARG;;
    d) dev_mode=1;;
    f) force=1; node_flags+=" -f";;
    g) debug_flag=1;;
    p) xrt_profiling=1;;
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
    echo "      force=$force"
    echo "      debug_flag=$debug_flag"
    echo "      xrt_profiling=$xrt_profiling"
    echo "      uninstall=$uninstall"
fi

echo ""
echo "INFO: TigerGraph installation info:"
echo "    APP root is $tg_root_dir"
echo "    TEMP root is $tg_temp_root"
echo "    DATA root is $tg_data_root"
echo "    UDF source directory is $tg_udf_dir"
echo "INFO: Home is $HOME"

# run installation script on each node
echo "Running installation script on each node with option $node_flags"
grun all "${SCRIPTPATH}/install-plugin-node.sh $node_flags"

if [ $dev_mode -eq 1 ]; then
    echo "INFO: Development mode -- skipping TigerGraph configuration."
else
    if [ $uninstall -eq 1 ]; then
        echo "INFO: Restarting GPE service"
        gadmin restart gpe -y
    else
        # copy gsql-client.jar to tmp directory
        cp $tg_root_dir/dev/gdk/gsql/lib/gsql_client.jar /tmp/gsql_client.jar.tmp

        echo "INFO: Apply environment changes to TigerGraph installation"
        gadmin start all

        ld_preload=
        if [ $pluginAlveoProductLibNeedsPreload -eq 1 ]; then
            ld_preload="$runtimeLibDir/$pluginLibName";
        fi
        if [ "$mem_alloc" == "tcmalloc" ]; then
            ld_preload="$ld_preload:$tg_root_dir/dev/gdk/gsdk/include/thirdparty/prebuilt/dynamic_libs/gmalloc/tcmalloc/libtcmalloc.so"
        fi

        ld_lib_path="$HOME/libstd:$runtimeLibDir:/opt/xilinx/xrt/lib:/opt/xilinx/xrm/lib:/usr/lib/x86_64-linux-gnu/"

        if [ "$xrt_profiling" -eq 1 ]; then
            gpe_config="LD_PRELOAD=\$LD_PRELOAD:$ld_preload;LD_LIBRARY_PATH=$ld_lib_path:\$LD_LIBRARY_PATH;CPUPROFILE=/tmp/tg_cpu_profiler;CPUPROFILESIGNAL=12;MALLOC_CONF=prof:true,prof_active:false;XILINX_XRT=/opt/xilinx/xrt;XILINX_XRM=/opt/xilinx/xrm;XRT_INI_PATH=$PWD/../scripts/debug/xrt-profile.ini"
        else
            gpe_config="LD_PRELOAD=\$LD_PRELOAD:$ld_preload;LD_LIBRARY_PATH=$ld_lib_path:\$LD_LIBRARY_PATH;CPUPROFILE=/tmp/tg_cpu_profiler;CPUPROFILESIGNAL=12;MALLOC_CONF=prof:true,prof_active:false;XILINX_XRT=/opt/xilinx/xrt;XILINX_XRM=/opt/xilinx/xrm"
        fi

        gadmin config set GPE.BasicConfig.Env "$gpe_config"
        # increase RESTPP timeout
        gadmin config set RESTPP.Factory.DefaultQueryTimeoutSec 36000

        echo "INFO: Apply the new configurations to $gpe_config"
        gadmin config apply -y
        gadmin restart GPE RESTPP -y
        gadmin config get GPE.BasicConfig.Env
        gadmin config get RESTPP.Factory.DefaultQueryTimeoutSec
    fi
fi

if [ -r $SCRIPTPATH/install-plugin-cluster-custom.sh ]; then
    . $SCRIPTPATH/install-plugin-cluster-custom.sh
fi
