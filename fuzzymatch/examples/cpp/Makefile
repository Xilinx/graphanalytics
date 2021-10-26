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

# Location of cosine similarity Alveo product 
ifndef XILINX_FUZZYMATCH
    XILINX_FUZZYMATCH = /opt/xilinx/apps/graphanalytics/fuzzymatch
    export XILINX_FUZZYMATCH
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

ifeq ($(wildcard $(LIB_PATH)/*.so),)
    LIB_PATH = $(XILINX_FUZZYMATCH)/${PRODUCT_VER}/lib
    INCLUDE_PATH = $(XILINX_FUZZYMATCH)/${PRODUCT_VER}/include
endif

CXXFLAGS = -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -fPIC -pthread -I$(INCLUDE_PATH) -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-label \
    -Wno-unused-variable -Wno-unused-parameter -Wno-missing-field-initializers -Wno-deprecated-declarations

ifdef U50
    deviceNames ?= xilinx_u50_gen3x16_xdma_201920_3
else
    deviceNames ?= xilinx_aws-vu9p-f1_dynamic_5_0
endif
XDEVICE = $(basename $(notdir $(deviceNames)))


XF_LIB_DIR := $(abspath $(XF_PROJ_ROOT))/examples

ifneq (,$(shell echo $(XDEVICE) | awk '/u50/'))
XCLBIN_FILE := $(XF_PROJ_ROOT)/xclbin/fuzzy_xilinx_u50_gen3x16_xdma_201920_3.xclbin
else ifneq (,$(shell echo $(XDEVICE) | awk '/u200/'))
XCLBIN_FILE := $(XF_PROJ_ROOT)/xclbin/fuzzy_xilinx_u200_xdma_201830_2.xclbin
else ifneq (,$(shell echo $(XDEVICE) | awk '/u250_xdma/'))
XCLBIN_FILE := $(XF_PROJ_ROOT)/xclbin/fuzzy_xilinx_u250_xdma_201830_2.xclbin
else ifneq (,$(shell echo $(XDEVICE) | awk '/u250_gen3x16/'))
XCLBIN_FILE := $(XF_PROJ_ROOT)/xclbin/fuzzy_xilinx_u250_gen3x16_xdma_2_1_202010_1.azure.xclbin
else ifneq (,$(shell echo $(XDEVICE) | awk '/aws-vu9p-f1/'))
XCLBIN_FILE := $(XF_PROJ_ROOT)/xclbin/fuzzy_xilinx_aws-vu9p-f1_shell-v04261818_201920_2.awsxclbin 
else
$(warning [WARNING]: This project has not been tested for $(deviceNames). It may or may not work.)
endif

#LD_LIBRARY_PATH := $(LD_LIBRARY_PATH):$(LIB_PATH)

EXE_FILE := fmtest

CXX := g++

ifdef DEBUG
CPP_BUILD_DIR = Debug
CXXFLAGS += -O0 -g
else
CPP_BUILD_DIR = Release
CXXFLAGS += -O3 -DNDEBUG
#CXXFLAGS += -O3 -g
endif

ifneq ($(shell expr $(shell g++ -dumpversion) \>= 5), 1)
$(error [ERROR]: g++ version too old. Please use 5.0 or above)
endif
$(EXE_FILE): fuzzymatch_test.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ \
		-I$(XILINX_XRT)/include \
		-L$(LIB_PATH) -lXilinxFuzzyMatch \
		-L$(XILINX_XRT)/lib -lOpenCL -lpthread -m64

.PHONY: run clean

runMode=0
totalEntities=10000000
numEntities=100
run: $(EXE_FILE)
	set -e; \
	. $(XILINX_XRT)/setup.sh; \
	LD_LIBRARY_PATH=$(LIB_PATH):$$LD_LIBRARY_PATH; \
	$(abspath $^)  -xclbin $(XCLBIN_FILE) -d ../data -c $(runMode) --devices $(deviceNames) \
		--total_entities $(totalEntities) --num_entities $(numEntities) 

clean:
	rm -f $(EXE_FILE)
