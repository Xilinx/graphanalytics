#!/bin/bash

#
# Copyright 2020 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Product version
PRODUCT_VER=$(cat ../VERSION)

# Location of Maximal Independent Set Alveo product
export XF_PROJ_ROOT=$PWD/../..
test -d "$XILINX_MIS" || XILINX_MIS=/opt/xilinx/apps/graphanalytics/mis

# Location of XRT and XRM
test -d "$XILINX_XRT" || export XILINX_XRT=/opt/xilinx/xrt
test -d "$XILINX_XRM" || export XILINX_XRM=/opt/xilinx/xrm

# Location of the Python wrappers
export LIB_PATH=$XF_PROJ_ROOT/lib
test -d "$LIB_PATH" || export LIB_PATH=$XILINX_MIS/$PRODUCT_VER/lib
export PYTHONPATH=$LIB_PATH:$PYTHONPATH

# Set run options
# xcbin
export XCLBIN_PATH=$XF_PROJ_ROOT/xclbin
test -d "$XCLBIN_PATH" || export XCLBIN_PATH=$XILINX_MIS/$PRODUCT_VER/xclbin
# device name
DEV_NAME=xilinx_u50_gen3x16_xdma_201920_3
XCLBIN_FILE=$XCLBIN_PATH/mis_xilinx_u50_gen3x16_xdma_201920_3.xclbin
if [[ "$DEV_NAME" == "xilinx_u55c_gen3x16_xdma_base_2" ]]
then
  XCLBIN_FILE=$XCLBIN_PATH/mis_xilinx_u55c_gen3x16_xdma_2_202110_1.xclbin
fi

# Setup Xilinx Tools
. $XILINX_XRT/setup.sh
. $XILINX_XRM/setup.sh

# Location of the C++ library
export LD_LIBRARY_PATH=$LIB_PATH:$LD_LIBRARY_PATH

# Run the command
echo "INFO: misdemo --xclbin $XCLBIN_FILE --data_dir ../data --deviceNames $DEV_NAME"
$* --xclbin $XCLBIN_FILE --data_dir ../data --deviceNames $DEV_NAME
