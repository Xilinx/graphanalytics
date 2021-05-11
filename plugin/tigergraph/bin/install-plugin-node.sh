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

# script options processing
xrtPath=/opt/xilinx/xrt
xrmPath=/opt/xilinx/xrm
force_clean=0
dev_mode=0
use_tcmalloc=0
uninstall=0
verbose=0

while getopts ":r:m:dfghuv" opt
do
case $opt in
    d) dev_mode=1;;
    f) force_clean=1; echo "INFO: Option set: Force rebuidling plugin libraries";;
    g) debug_flag="DEBUG=1"; echo "INFO: debug_flag=$debug_flag";;
    m) xrmPath=$OPTARG;;
    r) xrtPath=$OPTARG;;
    u) uninstall=1;;
    v) verbose=1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; exit 1;;
esac
done

. $SCRIPTPATH/common.sh

if [ $verbose -eq 1 ]; then
    echo "INFO: Script is running with the settings below:"
    echo "      mem_alloc=$mem_alloc"
    echo "      dev_mode=$dev_mode"
    echo "      xrtPath=$xrtPath"
    echo "      xrmPath=$xrmPath"
    echo "      force_clean=$force_clean"
    echo "      debug_flag=$debug_flag"
    echo "      xrt_profiling=$xrt_profiling"
    echo "      uninstall=$uninstall"
fi

###############################################################################

if [ $verbose -eq 1 ]; then
    echo "INFO: Checking TigerGraph installation version and directory"
fi

# Temporary directory of include files to be included into main UDF header (ExprFunctions.hpp)
tg_version=$(basename $tg_root_dir)
if [ "tg_version" == "3.1.0" ]; then
    tg_temp_include_dir=$tg_temp_root/gsql/codegen/udf
else
    tg_temp_include_dir=$tg_temp_root/gsql/codegen/QueryUdf
fi
echo "INFO: UDF codegen directory is $tg_temp_include_dir"

# Source directory for TigerGraph plugin
plugin_src_dir=$SCRIPTPATH/../udf

###############################################################################

#
# If uninstalling, remove UDFs from UDF file ExprFunctions.hpp and delete auxiliary files
#

if [ $uninstall -eq 1 ]; then
    # If there are Recom Engine UDFs in the UDF file, uninstall them
    if [ -f $tg_udf_dir/ExprFunctions.hpp ] \
            && [ $(grep -c 'mergeHeaders.*xilinxRecomEngine' $tg_udf_dir/ExprFunctions.hpp) -gt 0 ]
    then
        if [ ! -f "$tg_udf_dir/mergeHeaders.py" ]; then
            cp $plugin_src_dir/mergeHeaders.py $tg_udf_dir
        fi
        echo "INFO: Uninstalling Xilinx Recommendation Engine UDFs"
        mv $tg_udf_dir/ExprFunctions.hpp $tg_udf_dir/ExprFunctions.hpp.prev
        python3 $tg_udf_dir/mergeHeaders.py -u $tg_udf_dir/ExprFunctions.hpp.prev xilinxRecomEngine \
             > $tg_udf_dir/ExprFunctions.hpp
    else
        if [ $verbose -eq 1 ]; then
            echo "INFO: Xilinx Recommendation Engine UDFs not found in UDF file ExprFunctions.hpp"
        fi
    fi

    echo "INFO: Uninstalling Xilinx Recommendation Engine auxiliary files"
    rm -f $tg_udf_dir/libXilinxCosineSim.so
    rm -f $tg_udf_dir/cosinesim.hpp
    rm -f $tg_udf_dir/cosinesim_loader.cpp
    rm -f $tg_udf_dir/xilinxRecomEngineImpl.hpp
    rm -rf $tg_udf_xclbin_dir

    rm -f $tg_temp_include_dir/ExprFunctions.hpp
    rm -f $tg_temp_include_dir/ExprUtil.hpp
    rm -f $tg_temp_include_dir/cosinesim.hpp
    rm -f $tg_temp_include_dir/cosinesim_loader.cpp
    rm -f $tg_temp_include_dir/xilinxRecomEngineImpl.hpp
    
    exit 0
fi

###############################################################################

#
# Install the plugin files to the current TigerGraph node
#


# If the XCLBIN is from sandbox, copy it to TigerGraph

if [ $cosinesimNeedsTgInstall -eq 1 ]; then
    if [ $verbose -eq 1 ]; then
        echo "INFO: Installing local CosineSim from $cosinesimPath into TigerGraph"
    fi
    mkdir -p $tg_udf_xclbin_dir
    cp -f $xclbinPath $tg_udf_xclbin_dir
    cp $cosineSimPath/lib/libXilinxCosineSim.so $tg_udf_dir
fi

# save a copy of the original UDF Files

if [ ! -d "$tg_udf_dir.orig" ]; then
    cp -r $tg_udf_dir $tg_udf_dir.orig
    echo "INFO: Saved a copy of the original QueryUdf files in $tg_udf_dir.orig"
fi

# If the existing ExprFunctions.hpp has not been prepared for plugins, replace it
# with the base prepared version (containing just TG-supplied UDFs)

if [ ! -f $tg_udf_dir/ExprFunctions.hpp ] || [ $(grep -c mergeHeaders $tg_udf_dir/ExprFunctions.hpp) -eq 0 ]; then
    echo "INFO: TigerGraph UDF file ExprFunctions.hpp has no plugin tags.  Installing base UDF file with tags"
    cp -f $plugin_src_dir/ExprFunctions.hpp $tg_udf_dir
fi


#source $xrtPath/setup.sh
#source $xrmPath/setup.sh

# Install plugin to ExprFunctions.hpp file

echo "INFO: Installing Xilinx Recommendation Engine UDFs"
cp $plugin_src_dir/mergeHeaders.py $tg_udf_dir
mv $tg_udf_dir/ExprFunctions.hpp $tg_udf_dir/ExprFunctions.hpp.prev
python3 $tg_udf_dir/mergeHeaders.py $tg_udf_dir/ExprFunctions.hpp.prev $plugin_src_dir/xilinxRecomEngine.hpp \
     > $tg_udf_dir/ExprFunctions.hpp

# Copy files to TigerGraph UDF area

echo "INFO: Installing Xilinx Recommendation Engine auxiliary files"
cp $cosineSimPath/include/cosinesim.hpp $tg_udf_dir
cp $cosineSimPath/src/cosinesim_loader.cpp $tg_udf_dir
cp $plugin_src_dir/xilinxRecomEngineImpl.hpp $tg_udf_dir

tg_xclbin_path=$tg_udf_dir/xclbin/denseSimilarityKernel.xclbin
sed -i "s|TG_COSINESIM_XCLBIN|$targetXclbinPath|" $tg_udf_dir/xilinxRecomEngineImpl.hpp

# Copy include files used by UDFs to TG build directory for TigerGraph to build the UDF library

mkdir -p $tg_temp_include_dir
cp $tg_udf_dir/ExprFunctions.hpp $tg_temp_include_dir
cp $tg_udf_dir/ExprUtil.hpp $tg_temp_include_dir
cp $tg_udf_dir/cosinesim.hpp $tg_temp_include_dir
cp $tg_udf_dir/cosinesim_loader.cpp $tg_temp_include_dir
cp $tg_udf_dir/xilinxRecomEngineImpl.hpp $tg_temp_include_dir

