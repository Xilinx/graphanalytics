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
test -d "$XILINX_LOUVAINMOD" || $XILINX_LOUVAINMOD=/opt/xilinx/apps/graphanalytics/louvainmod

# Location of XRT and XRM
test -d "$XILINX_XRT" || export XILINX_XRT=/opt/xilinx/xrt
test -d "$XILINX_XRM" || export XILINX_XRM=/opt/xilinx/xrm

# Location of the Python wrappers
export LIB_PATH=$XF_PROJ_ROOT/lib
test -d "$LIB_PATH" || export LIB_PATH=$XILINX_LOUVAINMOD/$PRODUCT_VER/lib
export PYTHONPATH=$LIB_PATH:$PYTHONPATH

# Setup Xilinx Tools
. $XILINX_XRT/setup.sh
. $XILINX_XRM/setup.sh

# Location of the C++ library
export LD_LIBRARY_PATH=$LIB_PATH:$LD_LIBRARY_PATH

# Run the command
# Create partitions on a single node
python3 pythondemo.py --mode partition --graph ../data/as-Skitter-wt-r100.mtx --name as-Skitter-wt-r100 --num_pars 1

# Load partition and compute Louvain modularity on a single node
python3 pythondemo.py --mode load --numDevices 1 --deviceNames xilinx_u50_gen3x16_xdma_201920_3 --alveoProject as-Skitter-wt-r100.par.proj --xclbin /opt/xilinx/apps/graphanalytics/louvainmod/0.1/xclbin/louvainmod_pruning_xilinx_u50_gen3x16_xdma_201920_3.xclbin --num_level 100 --num_iter 100 --mode_zmq driver