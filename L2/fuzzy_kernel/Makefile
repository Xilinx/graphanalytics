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

ifndef XILINX_XRT
$(error XILINX_XRT ENV variable is not set, please set ENV variable correctly and rerun)
endif

DEVICE ?= xilinx_u50_gen3x16_xdma_201920_3
XDEVICE = $(basename $(notdir $(DEVICE)))

MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))
XF_PROJ_ROOT ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%makefile}')
XF_LIB_DIR := $(abspath $(XF_PROJ_ROOT))

ifneq (,$(shell echo $(XDEVICE) | awk '/u50/'))
XCLBIN_FILE := $(XF_PROJ_ROOT)/../shipment/xclbin/fuzzy_xilinx_u50_gen3x16_xdma_201920_3.xclbin
else ifneq (,$(shell echo $(XDEVICE) | awk '/u200/'))
XCLBIN_FILE := $(XF_PROJ_ROOT)/../shipment/xclbin/fuzzy_xilinx_u200_xdma_201830_2.xclbin
else ifneq (,$(shell echo $(XDEVICE) | awk '/u250_xdma/'))
XCLBIN_FILE := $(XF_PROJ_ROOT)/../shipment/xclbin/fuzzy_xilinx_u250_xdma_201830_2.xclbin
else ifneq (,$(shell echo $(XDEVICE) | awk '/u250_gen3x16/'))
XCLBIN_FILE := $(XF_PROJ_ROOT)/../shipment/xclbin/fuzzy_xilinx_u250_gen3x16_xdma_2_1_202010_1.xclbin
else ifneq (,$(shell echo $(XDEVICE) | awk '/aws-vu9p-f1/'))
XCLBIN_FILE := $(XF_PROJ_ROOT)/../shipment/xclbin/fuzzy_xilinx_aws-vu9p-f1_shell-v04261818_201920_1.xclbin
else
$(warning [WARNING]: This project has not been tested for $(DEVICE). It may or may not work.)
endif

LIB_SRCS += $(XF_LIB_DIR)/src/swift_mt103_check.cpp
LIB_SRCS += $(XF_LIB_DIR)/tests/fpga/host/xcl2.cpp
LIB_HDRS += $(XF_LIB_DIR)/src/swift_mt103_check.hpp
LIB_HDRS += $(XF_LIB_DIR)/tests/fpga/host/xcl2.hpp
PYTHONLIB_SRCS := ${LIB_SRCS}
PYTHONLIB_SRCS += $(XF_LIB_DIR)/src/pybind11_module.cpp


uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))
inc_paths = $(foreach _,$1,$(patsubst %xf_database/,%,$(dir $(abspath $_))))
inc_opts = $(patsubst %,-I%,$(call uniq,$(call inc_paths,$1)))

LIB_SO := $(XF_PROJ_ROOT)/../shipment/lib/libxilinxaml.so
PYTHONLIB_SO := $(XF_PROJ_ROOT)/../shipment/lib/xf_aml_python.so
PYTHON_INC_FLAGS = $(shell python3-config --includes)

CXX := g++
ifneq ($(shell expr $(shell g++ -dumpversion) \>= 5), 1)
$(error [ERROR]: g++ version too old. Please use 5.0 or above)
endif
$(LIB_SO): $(LIB_SRCS) $(LIB_HDRS)
	$(CXX) -std=c++11 -o $@ -fPIC -shared $(LIB_SRCS) \
		$(call inc_opts, $(LIB_HDRS)) -pthread -lOpenCL \
		-I$(XILINX_XRT)/include -L$(XILINX_XRT)/lib

$(PYTHONLIB_SO): ${PYTHONLIB_SRCS} $(LIB_HDRS)
	g++ -std=c++11 -o $@ -fPIC -shared $(PYTHONLIB_SRCS) $(LIB_HDRS)\
		-pthread -lOpenCL \
		-I$(XF_LIB_DIR)/tests/fpga/host \
		-I$(XILINX_XRT)/include -L$(XILINX_XRT)/lib $(PYTHON_INC_FLAGS)


.PHONY: lib xclbin

lib: $(LIB_SO)

$(XCLBIN_FILE):
	make -C tests/fpga xclbin TARGET=hw DEVICE=$(DEVICE)

xclbin: | $(XCLBIN_FILE)

plib:$(PYTHONLIB_SO)

.PHONY: cleanlib cleanxclbin

cleanlib:
	rm $(LIB_SO) $(PYTHONLIB_SO) -f

cleanxclbin:
	rm $(XCLBIN_FILE)
