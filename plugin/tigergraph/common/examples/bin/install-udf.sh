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

if [ "$USER" == "tigergraph" ]; then
    $SCRIPTPATH/install-udf-cluster.sh $verbose_flag $force_clean_flag
else
    echo "INFO: This example application requires components to be installed into TigerGraph."
    echo "INFO: Please enter the password for user \"tigergraph\" if prompted."
    ssh $ssh_key_flag tigergraph@$hostname $SCRIPTPATH/install-udf-cluster.sh \
        $verbose_flag $force_clean_flag $uninstall_flag
fi

