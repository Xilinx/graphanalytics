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

#set -x 
#
# Alveo Product Variables
#
# The plugin depends on its corresponding Alveo Product.  The following variables
# determine the names and locations of the components of that Alveo Product.
#
# Alveo product names
pluginAlveoProductName="Xilinx Community Detection Engine"
standaloneAlveoProductName="Xilinx Louvain Modularity"

# The usual place where the Alveo Product is installed
pluginInstalledAlveoProductPath=/opt/xilinx/apps/graphanalytics/louvainmod/0.5

# Where to find the git repo for the Alveo Product if it exists
pluginLocalAlveoProductPath=$SCRIPTPATH/../../../../../louvainmod/staging

# The name of the Alveo Product's XCLBIN file
pluginXclbinName=louvainmod_pruning_xilinx_u50_gen3x16_xdma_201920_3.xclbin

# The name of the Alveo Product's .so file
pluginLibName=libXilinxLouvain.so

# Set to 1 if the Alveo Product's .so needs to be added to LD_PRELOAD
pluginAlveoProductLibNeedsPreload=1

#
# Plugin Variables
#
# Variables for identifying the artifacts of the plug-in
#

# Name of the plugin, as found in mergeHeaders comments
pluginName=xilinxComDetect

# The main UDF file.  This file gets installed into ExprFunctions.hpp.
# It must contain mergeHeaders comments.  Also, any headers that this file
# depends on must be present in $pluginAlveoProductHeaders or $pluginHeaders.
# The path of this file is relative to the plugin top directory
pluginMainUdf=udf/xilinxComDetect.hpp

# List of header files to copy from the Alveo Product into the TigerGraph application area
# and UDF compilation area.  The paths are relative to $pluginAlveoProductPath.
pluginAlveoProductHeaders="include/xilinxlouvain.h include/xilinx_apps_common.hpp src/louvainmod_loader.cpp"

# List of header files to copy from the plugin into the TigerGraph application area
# and UDF compilation area.  The paths are relative to the plugin top directory
# (the parent of this script's directory).
pluginHeaders=udf/xilinxComDetectImpl.hpp

# List of extra files to copy from the plugin into the TigerGraph application area.
# Unlike $pluginHeaders, these files are not copied to the compilation area.
# The paths are relative to the plugin top directory (parent of this script's directory).
pluginExtraFiles=

# List of files (name only, no path) that need to have the identifier PLUGIN_XCLBIN_PATH
# replaced with the XCLBIN path.
pluginXclbinPathFiles="xilinxComDetect.hpp xilinxComDetectImpl.hpp"

# List of files (name only, no path) that need to have the identifier PLUGIN_CONFIG_PATH
# replaced with the config path.
pluginConfigPathFiles="xilinxComDetectImpl.hpp xilinxComDetect.hpp"
