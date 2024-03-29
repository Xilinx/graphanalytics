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
ifndef XILINX_LOUVAIN
    XILINX_LOUVAIN = /opt/xilinx/apps/graphanalytics/louvainmod
    export XILINX_LOUVAIN
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

LIB_PATH = $(XF_PROJ_ROOT)/staging/lib
INCLUDE_PATH = $(XF_PROJ_ROOT)/include

ifeq ($(wildcard $(LIB_PATH)/*.so),)
    LIB_PATH = $(XILINX_LOUVAIN)/${PRODUCT_VER}/lib
    INCLUDE_PATH = $(XILINX_LOUVAIN)/${PRODUCT_VER}/include
endif

CXXFLAGS = -std=c++11 -fPIC -pthread -I$(INCLUDE_PATH) -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-label \
    -Wno-unused-variable -Wno-unused-parameter -Wno-missing-field-initializers -Wno-deprecated-declarations

ifdef DEBUG
CXXFLAGS += -O0 -g
else
CXXFLAGS += -O3
endif


LDFLAGS = -L$(LIB_PATH) -lXilinxLouvain -lpthread -m64 -lzmq

all: cppdemo

cppdemo: cppdemo.cpp
	g++ $^ -o $@ $(CXXFLAGS) $(LDFLAGS)

.PHONY: run clean

graph = ../data/as-Skitter-wt-r100.mtx
alveoProject = $(basename $(notdir $(graph)))
numNodes = 1
numPars = 1
numDevices = 1
deviceNames=xilinx_u50_gen3x16_xdma_201920_3
ifeq ($(deviceNames),xilinx_u50_gen3x16_xdma_201920_3)
    xclbin = $(XILINX_LOUVAIN)/${PRODUCT_VER}/xclbin/louvainmod_pruning_xilinx_u50_gen3x16_xdma_201920_3.xclbin
	KERNEL_MODE = -kernel_mode 2
endif
ifeq ($(deviceNames),xilinx_u55c_gen3x16_xdma_base_2)
    xclbin = $(XILINX_LOUVAIN)/${PRODUCT_VER}/xclbin/louvainmod_2cu_xilinx_u55c_gen3x16_xdma_2_202110_1.xclbin
	KERNEL_MODE = -kernel_mode 4
endif
workers=192.168.1.21 192.168.1.31

.PHONY: run run-create-partitions run-load-compute

run: run-create-partitions run-load-compute

run-create-partitions: cppdemo
	@echo "------------------------------------------------------------------------"
	@echo "-                       Create Partitions                              -"
	@echo "------------------------------------------------------------------------"
	set -e; \
	. $(XILINX_XRT)/setup.sh; \
	. $(XILINX_XRM)/setup.sh; \
	LD_LIBRARY_PATH=$(LIB_PATH):$(LD_LIBRARY_PATH); \
	./cppdemo $(graph) $(KERNEL_MODE) -num_pars $(numPars) -create_alveo_partitions \
	    -name $(alveoProject)

run-load-compute: cppdemo
	@echo "------------------------------------------------------------------------"
	@echo "-                    Load Partitions and Compute                       -"
	@echo "------------------------------------------------------------------------"
	set -e; \
	. $(XILINX_XRT)/setup.sh; \
	. $(XILINX_XRM)/setup.sh; \
	LD_LIBRARY_PATH=$(LIB_PATH):$$LD_LIBRARY_PATH; \
	./cppdemo -x $(xclbin) $(KERNEL_MODE) -num_devices $(numDevices) -devices $(deviceNames) \
	    -num_level 100 -num_iter 100 -load_alveo_partitions \
	    $(alveoProject).par.proj -setwkr 0 -driverAlone

run-load-compute-driver: cppdemo
	@echo "------------------------------------------------------------------------"
	@echo "-              Load Partitions and Compute on Driver                   -"
	@echo "------------------------------------------------------------------------"
	set -e; \
	. $(XILINX_XRT)/setup.sh; \
	. $(XILINX_XRM)/setup.sh; \
	LD_LIBRARY_PATH=$(LIB_PATH):$$LD_LIBRARY_PATH; \
	./cppdemo -x $(xclbin) $(KERNEL_MODE) -num_nodes $(numNodes) -num_devices $(numDevices) -devices $(deviceNames) \
	    -num_level 100 -num_iter 100 -load_alveo_partitions \
	    $(alveoProject).par.proj -setwkr 2 $(workers) -driverAlone

run-load-compute-worker1: cppdemo
	@echo "------------------------------------------------------------------------"
	@echo "-              Load Partitions and Compute on Worker1                  -"
	@echo "------------------------------------------------------------------------"
	set -e; \
	. $(XILINX_XRT)/setup.sh; \
	. $(XILINX_XRM)/setup.sh; \
	LD_LIBRARY_PATH=$(LIB_PATH):$$LD_LIBRARY_PATH; \
	./cppdemo -x $(xclbin) $(KERNEL_MODE) -num_nodes $(numNodes) -num_devices $(numDevices) -devices $(deviceNames) \
	    -num_level 100 -num_iter 100 -load_alveo_partitions \
	    $(alveoProject).par.proj -workerAlone 1

run-load-compute-worker2: cppdemo
	@echo "------------------------------------------------------------------------"
	@echo "-              Load Partitions and Compute on Worker 2                 -"
	@echo "------------------------------------------------------------------------"
	set -e; \
	. $(XILINX_XRT)/setup.sh; \
	. $(XILINX_XRM)/setup.sh; \
	LD_LIBRARY_PATH=$(LIB_PATH):$$LD_LIBRARY_PATH; \
	./cppdemo -x $(xclbin) $(KERNEL_MODE) -num_nodes $(numNodes) -num_devices $(numDevices) -devices $(deviceNames) \
	    -num_level 100 -num_iter 100 -load_alveo_partitions \
	    $(alveoProject).par.proj -workerAlone 2

clean:
	rm -f cppdemo

clean-par:
	rm -f $(alveoProject)*

help:
	@echo "Makefile usages:"
	@echo "  make run-create-partitions [graph=/path/to/graph.mtx] [numPars=1]"
	@echo "  Create partitions on a single node"
	@echo ""
	@echo "  make run [graph=/path/to/graph.mtx] [numPars=1] [numDevices=1] [deviceNames=xilinx_u50_gen3x16_xdma_201920_3]"
	@echo "  Load partition and compute Louvain modularity on a single node"
	@echo ""
	@echo "  Run test in a 3 node cluster"
	@echo "  ssh to driver"
	@echo "  make run-load-compute-driver [numNodes=1]"
	@echo ""
	@echo "  ssh to worker1"
	@echo "  make run-load-compute-worker1 [numNodes=1]"
	@echo ""
	@echo "  ssh to worker2"
	@echo "  make run-load-compute-worker2 [numNodes=1]"
	@echo ""
	@echo "-------- Options --------"
	@echo "graph: graph input .mtx file"
	@echo "alveoProject : Alveo project name without .par.proj extension"
	@echo "deviceNames  : Target device name. Supported devices: "
	@echo "               xilinx_u50_gen3x16_xdma_201920_3"
	@echo "               xilinx_u55c_gen3x16_xdma_base_2"
    @echo "numNodes     : Number of nodes (default=1)"
	@echo "numPars      : Number of partitions (default=1)"
	@echo "numDevices   : Number of devices (default=1)"	
	@echo "workers      : set IP adresses for workers"

