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
use_tcmalloc=0
uninstall=0
verbose=0

while getopts ":huv" opt
do
case $opt in
    u) uninstall=1;;
    v) verbose=1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; exit 1;;
esac
done

. $SCRIPTPATH/common.sh

. $SCRIPTPATH/set-plugin-vars.sh

if [ $verbose -eq 1 ]; then
    echo "INFO: Script is running with the settings below:"
    echo "      uninstall=$uninstall"
fi

###############################################################################

if [ $verbose -eq 1 ]; then
    echo "INFO: Checking TigerGraph installation version and directory"
fi

# Temporary directory of include files to be included into main UDF header (ExprFunctions.hpp)
tg_version=$(basename $tg_root_dir)
if [ "$tg_version" == "3.1.0" ]; then
    tg_temp_include_dir=$tg_temp_root/gsql/codegen/udf
else
    tg_temp_include_dir=$tg_temp_root/gsql/codegen/QueryUdf
fi
echo "INFO: UDF codegen directory is $tg_temp_include_dir"

# Source directory for TigerGraph plugin
plugin_dir=$SCRIPTPATH/..
plugin_udf_dir=$plugin_dir/udf

# Files to install into TigerGraph application area
app_plugin_files="$pluginMainUdf $pluginHeaders $pluginExtraFiles"
app_alveo_product_files=$pluginAlveoProductHeaders

# Files to install into TigerGraph compilation area
compile_plugin_files="$pluginAlveoProductHeaders $pluginHeaders"

###############################################################################

#
# If uninstalling, remove UDFs from UDF file ExprFunctions.hpp and delete auxiliary files
#

if [ $uninstall -eq 1 ]; then
    # If there are Recom Engine UDFs in the UDF file, uninstall them
    if [ -f $tg_udf_dir/ExprFunctions.hpp ] \
            && [ $(grep -c "mergeHeaders.*$pluginName" $tg_udf_dir/ExprFunctions.hpp) -gt 0 ]
    then
        if [ ! -f "$tg_udf_dir/mergeHeaders.py" ]; then
            cp $plugin_udf_dir/mergeHeaders.py $tg_udf_dir
        fi
        echo "INFO: Uninstalling Xilinx Recommendation Engine UDFs"
        mv $tg_udf_dir/ExprFunctions.hpp $tg_udf_dir/ExprFunctions.hpp.prev
        python3 $tg_udf_dir/mergeHeaders.py -u $tg_udf_dir/ExprFunctions.hpp.prev $pluginName \
             > $tg_udf_dir/ExprFunctions.hpp
    else
        if [ $verbose -eq 1 ]; then
            echo "INFO: Xilinx Recommendation Engine UDFs not found in UDF file ExprFunctions.hpp"
        fi
    fi

    echo "INFO: Uninstalling Xilinx Recommendation Engine auxiliary files"
    for i in $app_plugin_files $app_alveo_product_files; do
        rm -f $tg_udf_dir/${i##*/}
    done

    for i in $compile_plugin_files; do
        rm -f $tg_temp_include_dir/${i##*/}
    done

    rm -f $tg_udf_dir/$pluginLibName
    rm -rf $tg_udf_xclbin_dir

    cp $tg_udf_dir/ExprFunctions.hpp $tg_temp_include_dir

    exit 0
fi

###############################################################################

#
# Install the plugin files to the current TigerGraph node
#


# If the XCLBIN is from sandbox, copy it to TigerGraph

if [ $pluginAlveoProductNeedsInstall -eq 1 ]; then
    if [ $verbose -eq 1 ]; then
        echo "INFO: Installing local Alveo Product from $pluginAlveoProductPath into TigerGraph"
    fi
    mkdir -p $tg_udf_xclbin_dir
    cp -f $pluginAlveoProductXclbinPath $tg_udf_xclbin_dir
    cp -f $pluginAlveoProductLibDir/$pluginLibName $tg_udf_dir
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
    cp -f $plugin_udf_dir/ExprFunctions.hpp $tg_udf_dir
fi


#source $xrtPath/setup.sh
#source $xrmPath/setup.sh

# Copy files to TigerGraph UDF area

echo "INFO: Installing Xilinx Recommendation Engine auxiliary files"
for i in $app_plugin_files; do
    cp -f $plugin_dir/$i $tg_udf_dir
done

for i in $app_alveo_product_files; do
    cp -f $pluginAlveoProductPath/$i $tg_udf_dir
done

# Substitute the XCLBIN path for PLUGIN_XCLBIN_PATH in all files that need the substitution

for i in $pluginXclbinPathFiles; do
    sed -i "s|PLUGIN_XCLBIN_PATH|\"$runtimeXclbinPath\"|" $tg_udf_dir/${i##*/}
done

# Install plugin to ExprFunctions.hpp file

echo "INFO: Installing Xilinx Recommendation Engine UDFs"
cp $plugin_udf_dir/mergeHeaders.py $tg_udf_dir
mv $tg_udf_dir/ExprFunctions.hpp $tg_udf_dir/ExprFunctions.hpp.prev
python3 $tg_udf_dir/mergeHeaders.py $tg_udf_dir/ExprFunctions.hpp.prev $tg_udf_dir/${pluginMainUdf##*/} \
     > $tg_udf_dir/ExprFunctions.hpp

# Copy include files used by UDFs to TG build directory for TigerGraph to build the UDF library

mkdir -p $tg_temp_include_dir
for i in $compile_plugin_files; do
    cp -f $tg_udf_dir/${i##*/} $tg_temp_include_dir
done
cp $tg_udf_dir/ExprFunctions.hpp $tg_temp_include_dir
cp $tg_udf_dir/ExprUtil.hpp $tg_temp_include_dir

