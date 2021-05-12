#!/bin/bash

# Product version
test -d "$PRODUCT_VER" || export PRODUCT_VER=1.0

# Location of cosine similarity Alveo product
export XF_PROJ_ROOT=$PWD/../../staging
test -d "$XILINX_COSINESIM" || XILINX_COSINESIM=/opt/xilinx/apps/graphanalytics/cosinesim

# Location of XRT and XRM
test -d "$XILINX_XRT" || export XILINX_XRT=/opt/xilinx/xrt
test -d "$XILINX_XRM" || export XILINX_XRM=/opt/xilinx/xrm

# Location of the Python wrappers
export LIB_PATH=$XF_PROJ_ROOT/lib
test -d "$LIB_PATH" || export LIB_PATH=$XILINX_COSINESIM/$PRODUCT_VER/lib
export PYTHONPATH=$LIB_PATH:$PYTHONPATH

# Setup Xilinx Tools
. $XILINX_XRT/setup.sh
. $XILINX_XRM/setup.sh

# Location of the C++ library
export LD_LIBRARY_PATH=$LIB_PATH:$LD_LIBRARY_PATH

# Run the command
$*
