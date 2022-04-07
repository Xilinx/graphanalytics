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

SHELL := /bin/bash

# Run make using "make DEBUG=1" to build debuggable executables

# Location of mis Alveo product
ifndef XILINX_MIS
    XILINX_MIS = /opt/xilinx/apps/graphanalytics/mis
    export XILINX_MIS
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

#the default PRODUCT_VER as the latest version
# Product version
PRODUCT_VER=$(strip $(shell cat ../VERSION))

# Location of Alveo product if installed from sources
MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))
XF_PROJ_ROOT ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%/examples/cpp/Makefile}')

LIB_PATH = $(XF_PROJ_ROOT)/lib
INCLUDE_PATH = $(XF_PROJ_ROOT)/include
XCLBIN_PATH = $(XF_PROJ_ROOT)/xclbin

ifeq ($(wildcard $(LIB_PATH)/*.so),)
    LIB_PATH = $(XILINX_MIS)/${PRODUCT_VER}/lib
    INCLUDE_PATH = $(XILINX_MIS)/${PRODUCT_VER}/include
    XCLBIN_PATH = $(XILINX_MIS)/${PRODUCT_VER}/xclbin
endif

CXXFLAGS = -std=c++11 -fPIC -pthread -I$(INCLUDE_PATH) -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-label \
    -Wno-unused-variable -Wno-unused-parameter -Wno-missing-field-initializers -Wno-deprecated-declarations

deviceNames=xilinx_u50_gen3x16_xdma_201920_3
ifeq ($(deviceNames),xilinx_u50_gen3x16_xdma_201920_3)
	XCLBIN_FILE := $(XCLBIN_PATH)/mis_xilinx_u50_gen3x16_xdma_201920_3.xclbin
else ifeq ($(deviceNames), xilinx_aws-vu9p-f1_shell-v04261818_201920_3)
    XCLBIN_FILE := $(XCLBIN_PATH)/mis_xilinx_aws-vu9p-f1_shell-v04261818_201920_3.awsxclbin
else ifeq ($(deviceNames),xilinx_u55c_gen3x16_xdma_base_2)
	XCLBIN_FILE := $(XCLBIN_PATH)/mis_xilinx_u55c_gen3x16_xdma_2_202110_1.xclbin
endif
#else
#$(warning [WARNING]: This project has not been tested for $(deviceNames). It may or may not work.)
#endif


EXE_FILE := misdemo

CXX := g++

ifdef DEBUG
CPP_BUILD_DIR = Debug
CXXFLAGS += -O0 -g
else
CPP_BUILD_DIR = Release
CXXFLAGS += -O3 -DNDEBUG
#CXXFLAGS += -O3 -g
endif

all: cppdemo

cppdemo: $(EXE_FILE)
$(EXE_FILE): misdemo.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ \
		-I$(XILINX_XRT)/include \
		-L$(LIB_PATH) -lXilinxMis \
		-L$(XILINX_XRT)/lib -lOpenCL -lpthread -lxrt_coreutil -lrt -m64

.PHONY: run clean

run: $(EXE_FILE)
	set -e; \
	. $(XILINX_XRT)/setup.sh; \
	LD_LIBRARY_PATH=$(LIB_PATH):$$LD_LIBRARY_PATH; \
	$(abspath $^) -xclbin $(XCLBIN_FILE) -d ../data --devices $(deviceNames) 
		

clean:
	rm -f $(EXE_FILE)

help:
	@echo "Makefile usages:"
	@echo "  make run [deviceNames=xilinx_u50_gen3x16_xdma_201920_3]"
	@echo "    Run standalone test with statically linked executable"
	@echo ""
	@echo "  Run options:"
	@echo "  deviceNames  : Specify Alveo devices to use for computation. Supported devices:"
	@echo "                 xilinx_u50_gen3x16_xdma_201920_3 "
	@echo "                 xilinx_aws-vu9p-f1_shell-v04261818_201920_3"
	@echo "                 xilinx_u55c_gen3x16_xdma_base_2"
	@echo ""
	@echo "  make test"
	@echo "    Compile mis test exe"