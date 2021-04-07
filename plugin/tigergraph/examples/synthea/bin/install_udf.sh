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

#
# Process command line options
#

function usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -f : Force (re)install"
    echo "  -u : Uninstall Synthea demo"
    echo "  -v : Verbose output"
    echo "  -h : Print this help message"
}

force_clean=0
uninstall=0
verbose=0
verbose_flag=
while getopts "fuv" opt
do
case $opt in
    f) force_clean=1;;
    u) uninstall=1;;
    v) verbose=1; verbose_flag=-v;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

#
# Check for prerequisite software and determine where TigerGraph is installed
#

if [ $verbose -eq 1 ]; then
    echo "INFO: Checking TigerGraph installation version and directory"
fi

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

TGHOME=~tigergraph

if [ ! -f "$TGHOME/.tg.cfg" ]; then
    echo "ERROR: This script only supports TigerGraph version 3.x"
    exit 1
fi

if [ ! -r "$TGHOME/.tg.cfg" ]; then
    echo "ERROR: TigerGraph configuration file $HOME/.tg.cfg is not readable"
    exit 1
fi

tg_root_dir=$(cat $TGHOME/.tg.cfg | jq .System.AppRoot | tr -d \")
tg_temp_root=$(cat $TGHOME/.tg.cfg | jq .System.TempRoot | tr -d \")
if [ $verbose -eq 1 ]; then
    echo "INFO: Found TigerGraph installation in $tg_root_dir"
    echo "INFO: TigerGraph TEMP root is $tg_temp_root"
fi

# Install dir for TigerGraph plugins
tg_udf_dir=$tg_root_dir/dev/gdk/gsql/src/QueryUdf

# Source directory for Synthea demo UDFs
plugin_src_dir=$SCRIPTPATH/../udf

#
# If uninstalling, clean up via tigergraph-enabled sub-script
#

if [ $uninstall -eq 1 ]; then
    echo "Uninstalling Synthea demo UDFs as user tigergraph..."
    su --login tigergraph -c "$SCRIPTPATH/install_udf_tg.sh -u $verbose_flag"
    exit 0
fi

if [ ! -f $tg_udf_dir/ExprFunctions.hpp ] \
        || [ $(grep -c 'mergeHeaders.*xilinxRecomEngine' $tg_udf_dir/ExprFunctions.hpp) -eq 0 ] \
        || [ ! -x $tg_udf_dir/mergeHeaders.py ]
then
    echo "ERROR: Xilinx Recommendation Engine appears not to be installed.  Please install that package first."
    exit 1
fi

#
# Check whether the Synthea demo UDF sources are newer than their installed counterparts.
# If not, don't bother reinstalling
#

if [ $force_clean -eq 0 ]; then
    if [ $verbose -eq 1 ]; then
        echo "INFO: Forcing installation"
    fi
    if [ ! $plugin_src_dir/syntheaDemo.hpp -nt $tg_udf_dir/syntheaDemo.hpp ] \
        && [ ! $plugin_src_dir/codevector.hpp -nt $tg_udf_dir/codevector.hpp ]; then
        echo "INFO: Synthea demo UDFs are up to date.  Skipping installation."
        exit 0
    fi
fi

#
# Install the UDF files as user tigergraph
#

echo "Installing Synthea demo UDFs as user tigergraph..."
su --login tigergraph -c "$SCRIPTPATH/install_udf_tg.sh $verbose_flag"
