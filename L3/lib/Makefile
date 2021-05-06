#
# Copyright 2020 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
############################### Setting up Project Variables ##############################
MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
XF_PROJ_ROOT ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%L3/lib/*}')
XFLIB_DIR = $(XF_PROJ_ROOT)

HOST_ARCH := x86
RMDIR = rm -rf

ifndef XILINX_XRT
  XILINX_XRT = /opt/xilinx/xrt
  export XILINX_XRT
endif

############################## Setting up Host Variables ##############################
#CXXFLAGS += -I$(XFLIB_DIR)/L2/include
CXXFLAGS += -I$(XFLIB_DIR)/ext/xcl2
CXXFLAGS += -I$(XFLIB_DIR)/L3/include

# for louvain
CXXFLAGS += -I$(XFLIB_DIR)/L3/include/include_hls

# for Xilinx Alveo Interface (XAI) to allow third party software to make a call
CXXFLAGS += -I$(XFLIB_DIR)/plugin/tigergraph/louvainmod/udf/QueryUdf

XILINX_XRM=/opt/xilinx/xrm

CXXFLAGS += -I$(XILINX_XRT)/include -I$(XILINX_VIVADO)/include -I$(XILINX_XRM)/include -std=c++11 -Wall -Wno-unknown-pragmas -Wno-unused-label
LDFLAGS += -L$(XILINX_XRT)/lib -lOpenCL -lpthread -lrt -Wno-unused-label -Wno-narrowing -L$(XILINX_XRM)/lib -lxrm
CXXFLAGS += -fmessage-length=0 
CXXFLAGS +=-I$(CUR_DIR)/src/ -DPRINTINFO_2
#CXXFLAGS += -DPRINTINFO -DVERBOSE 

ifneq ($(HOST_ARCH), x86)
	LDFLAGS += --sysroot=$(SYSROOT)
endif

CXXFLAGS += -D USE_HBM -D LOUVAINMOD
CXXFLAGS += -g
#CXXFLAGS += -O3

############################## Setting Rules for so (Building Dynamic Library) ##############################
libGRAPHL3 = $(XFLIB_DIR)/L3/lib/libgraphL3.so
libgraphL3 : $(libGRAPHL3)
CPP_PATH += $(XFLIB_DIR)/L3/src/common.cpp
CPP_PATH += $(XFLIB_DIR)/L3/src/op_similaritydense.cpp
CPP_PATH += $(XFLIB_DIR)/L3/src/xf_graph_L3_handle.cpp
CPP_PATH += $(XFLIB_DIR)/L3/src/xf_graph_L3.cpp
CPP_PATH += $(XFLIB_DIR)/ext/xcl2/xcl2.cpp

# for louvain
CPP_PATH += $(XFLIB_DIR)/L3/src/op_louvainmodularity.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/driverForGraphClustering.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/RngStream.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/utilityFunctions.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/parseInputFiles.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/buildNextPhase.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/coloringDistanceOne.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/utilityClusteringFunctions.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/parallelLouvainMethod.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/parallelLouvainWithColoring.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/louvainMultiPhaseRun.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/parseInputParameters.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/vertexFollowing.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/partitionLouvain.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/ParLV.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/ctrlLV.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/louvainPhase.cpp
CPP_PATH += $(XFLIB_DIR)/L3/tests/Louvain/host/xai.cpp

#$(libGRAPHL3): ${CPP_PATH}
#	/usr/bin/g++ -fPIC $(CXXFLAGS) -rdynamic -shared -o $@ $^ $(LDFLAGS) -w -Wl,--export-dynamic

# for louvain
test:
	echo $(XFLIB_DIR) $(XF_PROJ_ROOT) $(MK_PATH) $(MAKEFILE_LIST)

$(libGRAPHL3): ${CPP_PATH}
	/usr/bin/g++ -DPRAGMA -Ofast -fopenmp -fPIC $(CXXFLAGS) -rdynamic -shared -o $@ $^ $(LDFLAGS) -w -Wl,--export-dynamic

############################## Cleaning Rules ##############################
clean:
	-$(RMDIR) ${XFLIB_DIR}/L3/lib/*.so
