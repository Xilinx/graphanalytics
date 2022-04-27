#
# Copyright 2021 Xilinx, Inc.
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

SHELL := /bin/bash

# Run make using "make DEBUG=1" to build debuggable executables

# Location of cosine similarity Alveo product 
ifndef XILINX_COSINESIM
    XILINX_COSINESIM = /opt/xilinx/apps/graphanalytics/cosinesim
    export XILINX_COSINESIM
endif

# Location of XRT and XRM (for "run" target only)
ifndef XILINX_XRT
    XILINX_XRT = /opt/xilinx/xrt
    export XILINX_XRT
endif

ifndef XILINX_XRM
    XILINX_XRM=/opt/xilinx/xrm
    export XILINX_XRM
endif

#the default PROD_VERSION as the latest version
# Product version
PRODUCT_VER=$(strip $(shell cat ../VERSION))

# Location of Alveo product if installed from sources
MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))
XF_PROJ_ROOT ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%/examples/cpp/Makefile}')

$(info XF_PROJ_ROOT is $(XF_PROJ_ROOT))
LIB_PATH = $(XF_PROJ_ROOT)/lib
INCLUDE_PATH = $(XF_PROJ_ROOT)/include
XCLBIN_PATH = $(XF_PROJ_ROOT)/xclbin

$(info LIB_PATH is $(LIB_PATH))

ifeq ($(wildcard $(LIB_PATH)/*.so),)
    LIB_PATH = $(XILINX_COSINESIM)/${PROD_VERSION}/lib
    INCLUDE_PATH = $(XILINX_COSINESIM)/${PROD_VERSION}/include
    XCLBIN_PATH = $(XILINX_COSINESIM)/${PRODUCT_VER}/xclbin
endif

$(info XCLBIN_PATH is $(XCLBIN_PATH))

CXXFLAGS = -std=c++11 -fPIC -pthread -I$(INCLUDE_PATH) -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-label \
    -Wno-unused-variable -Wno-unused-parameter -Wno-missing-field-initializers -Wno-deprecated-declarations

ifdef DEBUG
CXXFLAGS += -O0 -g
else
CXXFLAGS += -O3
endif


LDFLAGS = -L$(LIB_PATH) -lXilinxCosineSim -lpthread -m64

#$(info PROJ_ROOT="$(XF_PROJ_ROOT)")
#$(info LIB_PATH="$(LIB_PATH)")
#$(info INCLUDE_PATH="$(INCLUDE_PATH)")
deviceNames=xilinx_u50_gen3x16_xdma_201920_3
XDEVICE = $(basename $(notdir $(deviceNames)))
ifeq ($(XDEVICE), xilinx_u50_gen3x16_xdma_201920_3)
	XCLBIN_FILE := $(XCLBIN_PATH)/cosinesim_32bit_xilinx_u50_gen3x16_xdma_201920_3.xclbin
else ifeq ($(XDEVICE), xilinx_aws-vu9p-f1_shell-v04261818_201920_2)
    XCLBIN_FILE := $(XCLBIN_PATH)/cosinesim_xilinx_aws-vu9p-f1_shell-v04261818_201920_2.awsxclbin 
else ifeq ($(XDEVICE),xilinx_u55c_gen3x16_xdma_base_2)
    XCLBIN_FILE := $(XCLBIN_PATH)/cosinesim_32bit_4pu_xilinx_u55c_gen3x16_xdma_base_2.xclbin 
else
    $(warning [WARNING]: This project has not been tested for $(deviceNames). It may or may not work.)
endif

demo: cppdemo

cppdemo: cppdemo.cpp
	g++ $^ -o $@ $(CXXFLAGS) $(LDFLAGS)

.PHONY: run clean

ITERATIONS=1
NUMVECTORS=5000

run:
	@set -e; \
	. $(XILINX_XRT)/setup.sh; \
	. $(XILINX_XRM)/setup.sh; \
	export LD_LIBRARY_PATH=$(LIB_PATH):$$LD_LIBRARY_PATH; \
	./cppdemo --xclbin ${XCLBIN_FILE} -i $(ITERATIONS) -n $(NUMVECTORS) -d $(deviceNames)

clean:
	rm -f cppdemo *txt *log

help:
	@echo "Makefile usages:"
	@echo "  make run  [deviceNames=xilinx_u50_gen3x16_xdma_201920_3]"
	@echo "    Run standalone test with statically linked executable"
	@echo ""
	@echo "  Run options:"
	@echo "  deviceNames  : Specify Alveo devices to use for computation"
	@echo "                 Supported devices:"
	@echo "                 xilinx_u50_gen3x16_xdma_201920_3(default) "
	@echo "                 xilinx_aws-vu9p-f1_shell-v04261818_201920_2"
	@echo "                 xilinx_u55c_gen3x16_xdma_base_2"
	@echo ""
	@echo "  make demo"
	@echo "    Compile cosinesim demo executable"
