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

# UDF module name for this plugin.  This value is used by mergeHeaders to identify the plugin in ExprFunctions.hpp
PLUGIN_NAME=travelPlanDemo

# Header file containing UDFs for the plugin
PLUGIN_MAIN_UDF=travelPlanDemo.hpp

# Header files used by the plugin
PLUGIN_HEADERS=travelPlanDemoImpl.hpp

# List of plugins that this plugin depends on.  Install script errors out if one or more dependencies is found
# not to have been installed.
PLUGIN_DEPENDENCIES=xilinxMis

