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

# Install or uninstall Recommendation Engine tuples
gsql="java -jar $HOME/gsql_client/gsql_client.jar"

read -p "Enter user tigergraph's password for gsql client:" password 
if [ $uninstall -eq 1 ]; then
    echo "INFO: Removing Recommendation Engine tuples"
    if [ $(gsql "LS" | grep -c XilCosinesimMatch) -gt 0 ]; then
        gsql "DROP TUPLE XilCosinesimMatch"
    fi
    echo ""
    echo "INFO: Xilinx FPGA acceleration plugin for Tigergraph has been uninstalled."
else
    echo "INFO: Adding Recommendation Engine tuples"
    if [ $(gsql "LS" -p $password | grep -c XilCosinesimMatch) -lt 1 ]; then
        gsql -p $password "TYPEDEF TUPLE<Id VERTEX, score double> XilCosinesimMatch"
    fi
    echo ""
    echo "INFO: Xilinx FPGA acceleration plugin for Tigergraph has been installed."
fi

