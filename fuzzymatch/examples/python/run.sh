#!/bin/bash

# Copyright 2021 Xilinx, Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# Product version
test -d "$PRODUCT_VER" || export PRODUCT_VER=0.1

# Location of cosine similarity Alveo product
export XF_PROJ_ROOT=$PWD/../../staging
test -d "$XILINX_FUZZYMATCH" || $XILINX_LOUVAINMOD=/opt/xilinx/apps/graphanalytics/fuzzymatch

# Location of XRT and XRM
test -d "$XILINX_XRT" || export XILINX_XRT=/opt/xilinx/xrt
test -d "$XILINX_XRM" || export XILINX_XRM=/opt/xilinx/xrm

# Location of the Python wrappers
export LIB_PATH=$XF_PROJ_ROOT/lib
test -d "$LIB_PATH" || export LIB_PATH=$XILINX_FUZZYMATCH/$PRODUCT_VER/lib
export PYTHONPATH=$LIB_PATH:$PYTHONPATH

# Setup Xilinx Tools
. $XILINX_XRT/setup.sh
. $XILINX_XRM/setup.sh

# Location of the C++ library
export LD_LIBRARY_PATH=$LIB_PATH:$LD_LIBRARY_PATH

# Run the command

python3 pythondemo.py  --deviceNames xilinx_u50_gen3x16_xdma_201920_3  --xclbin $XF_PROJ_ROOT/xclbin/fuzzy_xilinx_u50_gen3x16_xdma_201920_3.xclbin