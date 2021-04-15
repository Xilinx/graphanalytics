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
# Make sure that this script is running as user tigergraph
#

if [ "$USER" != "tigergraph" ]; then
    echo "ERROR: You must be logged in as user tigergraph to run this script"
    exit 1
fi

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
force_clean_flag=
uninstall=0
verbose=0
verbose_flag=
while getopts "fuv" opt
do
case $opt in
    f) force_clean=1; force_clean_flag=-f;;
    u) uninstall=1;;
    v) verbose=1; verbose_flag=-v;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

#
# If uninstalling, clean up on all nodes
#

if [ $uninstall -eq 1 ]; then
    echo "Uninstalling Synthea demo UDFs on all TigerGraph nodes..."
    grun all "$SCRIPTPATH/install_udf_tg.sh -u $verbose_flag"
    echo "INFO: Restarting GPE service"
    gadmin restart gpe -y
    exit 0
fi

#
# Install the UDF files on all nodes
#

echo "Installing Synthea demo UDFs on all TigerGraph nodes..."
grun all "$SCRIPTPATH/install_udf_tg.sh $verbose_flag $force_clean_flag"

#
# Restart GPE to make sure changes go through
#

echo "INFO: Restarting GPE service"
gadmin restart gpe -y
