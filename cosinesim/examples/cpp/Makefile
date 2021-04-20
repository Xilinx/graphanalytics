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
ifndef PROD_VERSION 
    PROD_VERSION = 1.0
endif

# Location of Alveo product if installed from sources
MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))
XF_PROJ_ROOT ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%/examples/cpp/Makefile}')

LIB_PATH = $(XF_PROJ_ROOT)/staging/lib
INCLUDE_PATH = $(XF_PROJ_ROOT)/include

ifeq ($(wildcard $(LIB_PATH)/*.so),)
    LIB_PATH = $(XILINX_COSINESIM)/${PROD_VERSION}/lib
    INCLUDE_PATH = $(XILINX_COSINESIM)/${PROD_VERSION}/include
endif


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



all: cppdemo

cppdemo: cppdemo.cpp
	g++ $^ -o $@ $(CXXFLAGS) $(LDFLAGS)

.PHONY: run clean

run:
	@set -e; \
	. $(XILINX_XRT)/setup.sh; \
	. $(XILINX_XRM)/setup.sh; \
	export LD_LIBRARY_PATH=$(LIB_PATH):$$LD_LIBRARY_PATH; \
	./cppdemo

clean:
	rm -f cppdemo *txt *log
