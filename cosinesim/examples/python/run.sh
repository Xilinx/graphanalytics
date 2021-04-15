#!/bin/bash

# Location of cosine similarity Alveo product
export XILINX_COSINESIM=$PWD/../../staging
test -d "$XILINX_COSINESIM" || XILINX_COSINESIM=/opt/xilinx/apps/graphanalytics/cosinesim

# Location of XRT and XRM
test -d "$XILINX_XRT" || export XILINX_XRT=/opt/xilinx/xrt
test -d "$XILINX_XRM" || export XILINX_XRM=/opt/xilinx/xrm

# Location of the Python wrappers
export PYTHONPATH=$XILINX_COSINESIM/lib:$PYTHONPATH

# Setup Xilinx Tools
. $XILINX_XRT/setup.sh
. $XILINX_XRM/setup.sh

# Location of the C++ library
export LD_LIBRARY_PATH=$XILINX_COSINESIM/lib:$LD_LIBRARY_PATH
