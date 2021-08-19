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

function usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -f                  : Force (re)install"
    echo "  -i sshKey           : SSH key for user tigergraph"    
    echo "  -u                  : Uninstall"
    echo "  -v                  : Print verbose messages"
    echo "  -h                  : Print this help message"
}

# default values for optional options
hostname=$(hostname)
force_clean=0
force_clean_flag=
ssh_key_flag=
uninstall=0
uninstall_flag=
verbose=0
verbose_flag=

# set default ssh_key for tigergraph
if [ -f ~/.ssh/tigergraph_rsa ]; then
    ssh_key_flag="-i ~/.ssh/tigergraph_rsa"
fi

while getopts ":fi:uvh" opt
do
case $opt in
    f) force_clean=1; force_clean_flag=-f;;
    i) ssh_key=$OPTARG; ssh_key_flag="-i $ssh_key";;
    u) uninstall=1; uninstall_flag=-u;;
    v) verbose=1; verbose_flag=-v;;
    h) usage; exit 1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

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

# set up PATH for tigergraph commands
export PATH=$tg_root_dir/../cmd:$PATH

. $SCRIPTPATH/set-plugin-vars.sh
