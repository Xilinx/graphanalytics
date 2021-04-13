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
set -x 

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

#
# Process command line options
#

uninstall=0
verbose=0
while getopts "uv" opt
do
case $opt in
    u) uninstall=1;;
    v) verbose=1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; exit 1;;
esac
done

if ! [ -x "$(command -v gadmin)" ]; then
    # Try picking up gadmin from .bashrc
    if [ -r $HOME/.bashrc ]; then
        . $HOME/.bashrc
    fi
    if ! [ -x "$(command -v gadmin)" ]; then
        echo "ERROR: Cannot find TigerGraph installation. Please run this install script as the user for the TigerGraph installation."
        exit 1
    fi
fi


tg_root_dir=$(cat $HOME/.tg.cfg | jq .System.AppRoot | tr -d \")
tg_temp_root=$(cat $HOME/.tg.cfg | jq .System.TempRoot | tr -d \")

# Install dir for TigerGraph plugins
tg_udf_dir=$tg_root_dir/dev/gdk/gsql/src/QueryUdf

# Source directory for Synthea demo UDFs
plugin_src_dir=$SCRIPTPATH/../udf

# Temporary directory of include files to be included into main UDF header (ExprFunctions.hpp)
tg_temp_include_dir=$tg_temp_root/gsql/codegen/udf

#
# If uninstalling, clean up installed files and uninstall the demo UDFs
#
if [ $uninstall -eq 1 ]; then
    echo "INFO: Uninstalling Synthea demo UDFs"
    if [ -f $tg_udf_dir/ExprFunctions.hpp ] \
            && [ $(grep -c 'mergeHeaders.*syntheaDemo' $tg_udf_dir/ExprFunctions.hpp) -gt 0 ]
    then
        if [ -x $tg_udf_dir/mergeHeaders.py ]; then
            grun all mv $tg_udf_dir/ExprFunctions.hpp $tg_udf_dir/ExprFunctions.hpp.prev
            grun all python3 $tg_udf_dir/mergeHeaders.py -u $tg_udf_dir/ExprFunctions.hpp.prev syntheaDemo \
                 > $tg_udf_dir/ExprFunctions.hpp
        else
            echo "ERROR: mergeHeaders.py is missing from $tg_udf_dir.  Can't uninstall UDFs."
        fi
    fi

    echo "INFO: Uninstalling Synthea demo auxiliary files"
    rm -f $tg_udf_dir/codevector.hpp
    rm -f $tg_udf_dir/syntheaDemo.hpp
    rm -f $tg_temp_include_dir/codevector.hpp
    echo "INFO: Restarting GPE service"
    gadmin restart gpe -y
    exit 0
fi

#
# Install Synthea demo UDFs to ExprFunctions.hpp
#

echo "INFO: Installing Synthea demo UDFs"
grun all "mv $tg_udf_dir/ExprFunctions.hpp $tg_udf_dir/ExprFunctions.hpp.prev"
grun all "python3 $tg_udf_dir/mergeHeaders.py $tg_udf_dir/ExprFunctions.hpp.prev $plugin_src_dir/syntheaDemo.hpp \
     > $tg_udf_dir/ExprFunctions.hpp"

#
# Install other Synthea demo headers to TigerGraph installation
#

echo "INFO: Installing Synthea demo auxiliary files"
grun all "cp -f $plugin_src_dir/codevector.hpp $tg_udf_dir"
grun all "cp -f $plugin_src_dir/syntheaDemo.hpp $tg_udf_dir"

grun all "mkdir -p $tg_temp_include_dir"
grun all "cp -f $tg_udf_dir/codevector.hpp $tg_temp_include_dir"

#
# Restart GPE to make sure changes go through
#

echo "INFO: Restarting GPE service"
gadmin restart gpe -y
