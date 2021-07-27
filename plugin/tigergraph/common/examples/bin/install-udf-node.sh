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
#set -x

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

. $SCRIPTPATH/common-udf.sh

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

tg_root_dir=$(cat $HOME/.tg.cfg | jq .System.AppRoot | tr -d \")
tg_temp_root=$(cat $HOME/.tg.cfg | jq .System.TempRoot | tr -d \")
tg_data_root=$(cat $HOME/.tg.cfg | jq .System.DataRoot | tr -d \")
# Install dir for TigerGraph plugins
#tg_udf_dir=$tg_root_dir/dev/gdk/gsql/src/QueryUdf
tg_udf_dir=$tg_data_root/gsql/udf

if [ $verbose -eq 1 ]; then
    echo "INFO: TigerGraph installation info:"
    echo "    APP root is $tg_root_dir"
    echo "    TEMP root is $tg_temp_root"
    echo "    DATA root is $tg_data_root"
    echo "    UDF source directory is $tg_udf_dir"
fi

# Source directory for plugin
plugin_src_dir=$SCRIPTPATH/..

# Temporary directory of include files to be included into main UDF header (ExprFunctions.hpp)
tg_version=$(basename $tg_root_dir)
if [ "$tg_version" == "3.1.0" ]; then
    tg_temp_include_dir=$tg_temp_root/gsql/codegen/udf
else
    tg_temp_include_dir=$tg_temp_root/gsql/codegen/QueryUdf
fi
echo "INFO: UDF codegen directory is $tg_temp_include_dir"

#
# If uninstalling, clean up installed files and uninstall the plugin's UDFs
#

if [ $uninstall -eq 1 ]; then
    echo "INFO: Uninstalling plugin UDFs from node $HOSTNAME"
    if [ -f $tg_udf_dir/ExprFunctions.hpp ] \
            && [ $(grep -c "mergeHeaders.*$PLUGIN_NAME" $tg_udf_dir/ExprFunctions.hpp) -gt 0 ]
    then
        if [ -x $tg_udf_dir/mergeHeaders.py ]; then
            mv $tg_udf_dir/ExprFunctions.hpp $tg_udf_dir/ExprFunctions.hpp.prev
            python3 $tg_udf_dir/mergeHeaders.py -u $tg_udf_dir/ExprFunctions.hpp.prev $PLUGIN_NAME \
                 > $tg_udf_dir/ExprFunctions.hpp
        else
            echo "ERROR: mergeHeaders.py is missing from $tg_udf_dir.  Can't uninstall UDFs."
        fi
    fi

    echo "INFO: Uninstalling plugin auxiliary files"
    for i in $PLUGIN_HEADERS $PLUGIN_MAIN_UDF; do
        rm -f $tg_udf_dir/$i
    done

    for i in $PLUGIN_HEADERS; do
        rm -f $tg_temp_include_dir/$i
    done

    cp $tg_udf_dir/ExprFunctions.hpp $tg_temp_include_dir
    exit 0
fi

#
# Installation
#

#
# Check that the plugin's dependencies are installed first
#

if [ "$PLUGIN_DEPENDENCIES" != "" ]; then
    if [ ! -f $tg_udf_dir/ExprFunctions.hpp ] || [ ! -x $tg_udf_dir/mergeHeaders.py ]; then
        echo "ERROR: Plugin dependencies appear not to have been installed (missing ExprFunctions.hpp or mergeHeaders.py)."
        exit 1
    fi

    for i in $PLUGIN_DEPENDENCIES; do
        if [ $(grep -c "mergeHeaders.*$i" $tg_udf_dir/ExprFunctions.hpp) -eq 0 ]; then
            echo "ERROR: Plugin dependency $i appears not to have been installed.  Please install that plugin first."
            exit 2
        fi
    done
fi

#
# Now remove other plugin UDFs from ExprFunctions.hpp if they exist
#

demo_name_regex="mergeHeaders.*? ([^ ]*)Demo"
while :
do
	expr_content="$(<$tg_udf_dir/ExprFunctions.hpp)"
	if [[ "$expr_content" =~ $demo_name_regex ]]; then
		remove_plugin_name=${BASH_REMATCH[1]}Demo
		echo "INFO: Found $remove_plugin_name UDFs installed, removing that first"
		mv $tg_udf_dir/ExprFunctions.hpp $tg_udf_dir/ExprFunctions.hpp.prev
		python3 $tg_udf_dir/mergeHeaders.py -u $tg_udf_dir/ExprFunctions.hpp.prev "$remove_plugin_name" \
		 > $tg_udf_dir/ExprFunctions.hpp
	else
		break
	fi
done

#
# Install plugin UDFs to ExprFunctions.hpp
#

echo "INFO: Installing plugin UDFs to node $HOSTNAME"
mv $tg_udf_dir/ExprFunctions.hpp $tg_udf_dir/ExprFunctions.hpp.prev
python3 $tg_udf_dir/mergeHeaders.py $tg_udf_dir/ExprFunctions.hpp.prev $plugin_src_dir/udf/$PLUGIN_MAIN_UDF \
     > $tg_udf_dir/ExprFunctions.hpp

#
# Install other plugin headers to TigerGraph installation
#

echo "INFO: Installing plugin auxiliary files"
for i in $PLUGIN_HEADERS $PLUGIN_MAIN_UDF; do
    cp -f $plugin_src_dir/udf/$i $tg_udf_dir
done

mkdir -p $tg_temp_include_dir
for i in $PLUGIN_HEADERS; do
    cp -f $tg_udf_dir/$i $tg_temp_include_dir
done
