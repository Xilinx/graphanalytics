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

XILINX_XRT = /opt/xilinx/xrt
XILINX_XRM=/opt/xilinx/xrm

DEVICE ?= xilinx_u50_gen3x16_xdma_201920_3
XDEVICE = $(basename $(notdir $(DEVICE)))

MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))
PROJ_ROOT ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%/L3/tests/Louvain/Makefile}')
INPUT_DATA_DIR := $(abspath $(PROJ_ROOT))/L3/tests/Louvain/
CXXFLAGS += -I $(PROJ_ROOT)/L3/include
CXXFLAGS += -I $(PROJ_ROOT)/L3/include/include_hls
CXXFLAGS += -I $(PROJ_ROOT)/../ext/xcl2
CXXFLAGS += -I $(PROJ_ROOT)/L3/tests/Louvain/host
CXXFLAGS += -I $(XILINX_XRM)/include
LIB_PATH := $(PROJ_ROOT)/L3/lib
CXXFLAGS += -O3
#CXXFLAGS += -g
export LD_LIBRARY_PATH=$(LIB_PATH):$(XILINX_XRT)/lib

ifneq (,$(shell echo $(XDEVICE) | awk '/u50/'))
XCLBIN_FILE := $(PROJ_ROOT)/L3/tests/Louvain/louvain_xilinx_u50_gen3x16_xdma_201920_3.xclbin
else ifneq (,$(shell echo $(XDEVICE) | awk '/u200/'))
XCLBIN_FILE := $(PROJ_ROOT)/L3/tests/Louvain/louvain_xilinx_u200_xdma_201830_2.xclbin
else ifneq (,$(shell echo $(XDEVICE) | awk '/u250/'))
XCLBIN_FILE := $(PROJ_ROOT)/L3/tests/Louvain/louvain_xilinx_u250_xdma_201830_2.xclbin
else ifneq (,$(shell echo $(XDEVICE) | awk '/aws-vu9p-f1/'))
XCLBIN_FILE := $(PROJ_ROOT)/L3/tests/Louvain/louvain_xilinx_aws-vu9p-f1_shell-v04261818_201920_1.xclbin
else
$(warning [WARNING]: This project has not been tested for $(DEVICE). It may or may not work.)
endif


EXE_FILE := host.exe

$(EXE_FILE): test_louvainModularity.cpp  $(PROJ_ROOT)/../ext/xcl2/xcl2.cpp
	/usr/bin/g++ -std=c++11 $^ -o $@ \
		-I$(XILINX_XRT)/include $(CXXFLAGS)\
		-L$(LIB_PATH) -lgraphL3 -lzmq\
		-L$(XILINX_XRT)/lib -lOpenCL -lpthread -m64 -fopenmp\
		-L$(XILINX_XRM)/lib -lxrm
#	g++ -std=c++11 $^ -o $@ \
#		-I$(XILINX_XRT)/include $(CXXFLAGS)\
#		-L$(LIB_PATH) -lxilinxlouvain \
#		-L$(XILINX_XRT)/lib -lOpenCL -lpthread -m64
#g++ -std=c++11 $^ -o $@ -DMULTITHREAD\

.PHONY: run clean

run: $(EXE_FILE)
	$(abspath $^) -x $(XCLBIN_FILE) -f 3 -c -o -m 10000 $(INPUT_DATA_DIR)/data/as-Skitter-wt.mtx
#$(abspath $^) -x $(XCLBIN_FILE) -f 3 -c -o -m 10000 $(INPUT_DATA_DIR)/data/europe_osm-wt.mtx

clean:
	rm -f $(EXE_FILE)
