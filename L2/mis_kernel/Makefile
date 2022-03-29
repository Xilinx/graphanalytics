#
# Copyright 2019-2021 Xilinx, Inc.
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
# makefile-generator v1.0.4
#

# ####################################### Help Section #####################################
.PHONY: help

help::
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make all TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform>"
	$(ECHO) "       Command to generate the design for specified Target and Shell."
	$(ECHO) ""
	$(ECHO) "  make clean "
	$(ECHO) "      Command to remove the generated non-hardware files."
	$(ECHO) ""
	$(ECHO) "  make cleanall"
	$(ECHO) "      Command to remove all the generated files."
	$(ECHO) ""
	$(ECHO) "  make build TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform>"
	$(ECHO) "      Command to build xclbin application."
	$(ECHO) ""

# ##################### Setting up default value of TARGET ##########################
TARGET ?= sw_emu

# ################### Setting up default value of DEVICE ##############################
DEVICE ?= xilinx_u50_gen3x16_xdma_201920_3

# #################### Checking if DEVICE in blacklist #############################

# #################### Checking if DEVICE in whitelist ############################
ifneq ($(findstring u250, $(DEVICE)), u250)
ifneq ($(findstring u200, $(DEVICE)), u200)
ifneq ($(findstring u50, $(DEVICE)), u50)
ifneq ($(findstring u55c, $(DEVICE)), u55c)
$(error [ERROR]: This project is not supported for $(DEVICE).)
endif
endif
endif
endif

# ######################## Setting up Project Variables #################################
MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
XF_PROJ_ROOT ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%/mis_kernel/*}')
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))
XFLIB_DIR = $(XF_PROJ_ROOT)

# ######################### Include environment variables in utils.mk ####################
include ./utils.mk
XDEVICE := $(call device2xsa, $(DEVICE))
TEMP_DIR := _x_temp.$(TARGET).$(XDEVICE)
TEMP_REPORT_DIR := $(CUR_DIR)/reports/_x.$(TARGET).$(XDEVICE)
BUILD_DIR := build_dir.$(TARGET).$(XDEVICE)
BUILD_REPORT_DIR := $(CUR_DIR)/reports/_build.$(TARGET).$(XDEVICE)
XCLBIN_DIR := $(CUR_DIR)/$(BUILD_DIR)
export XCL_BINDIR = $(XCLBIN_DIR)

# ##################### Kernel compiler global settings ##########################
VPP_FLAGS += -t $(TARGET) --platform $(XPLATFORM) --save-temps --optimize 2
VPP_FLAGS += --hls.jobs 16
VPP_FLAGS += --hls.clock 333000000:misKernel
VPP_FLAGS += -I$(XFLIB_DIR)/mis_kernel/include
ifeq ($(DEBUG), yes)
VPP_FLAGS += -g
endif

VPP_LDFLAGS += --vivado.synth.jobs 16 --vivado.impl.jobs 16

# Kernel linker flags
VPP_LDFLAGS_misKernel_temp := --config $(CUR_DIR)/opts.cfg 

VPP_FLAGS += -DMIS_entries=1
VPP_FLAGS += -DMIS_numChannels=16
VPP_FLAGS += -DMIS_maxRows=1024000 

ifneq (,$(shell echo $(XPLATFORM) | awk '/u200/'))
VPP_LDFLAGS_misKernel_temp += --config $(CUR_DIR)/conn_u200c.cfg
VPP_LDFLAGS_misKernel_temp += --kernel_frequency 300
SRCS=$(XFLIB_DIR)/mis_kernel/src/misKernel_ddr.cpp
endif

ifneq (,$(shell echo $(XPLATFORM) | awk '/u250/'))
VPP_LDFLAGS_misKernel_temp += --config $(CUR_DIR)/conn_u250.cfg
VPP_LDFLAGS_misKernel_temp += --kernel_frequency 300
SRCS=$(XFLIB_DIR)/mis_kernel/src/misKernel_ddr.cpp
endif

ifneq (,$(shell echo $(XPLATFORM) | awk '/u55c/'))
VPP_LDFLAGS_misKernel_temp += --config $(CUR_DIR)/conn_u55c.cfg
VPP_LDFLAGS_misKernel_temp += --kernel_frequency 300
SRCS=$(XFLIB_DIR)/mis_kernel/src/misKernel.cpp
endif

ifneq (,$(shell echo $(XPLATFORM) | awk '/u50/'))
VPP_LDFLAGS_misKernel_temp += --config $(CUR_DIR)/conn_u50.cfg
VPP_LDFLAGS_misKernel_temp += --kernel_frequency 300
SRCS=$(XFLIB_DIR)/mis_kernel/src/misKernel.cpp
endif

VPP_LDFLAGS_misKernel += $(VPP_LDFLAGS_misKernel_temp)

# ############################ Declaring Binary Containers ##########################

BINARY_CONTAINERS += $(BUILD_DIR)/misKernel.xclbin
BINARY_CONTAINER_misKernel_OBJS += $(TEMP_DIR)/misKernel.xo

# ######################### Setting Targets of Makefile ################################

.PHONY: all clean cleanall docs 
all: check_vpp check_platform $(BINARY_CONTAINERS) 

.PHONY: xclbin
xclbin: check_vpp $(BINARY_CONTAINERS)

.PHONY: build
build: xclbin

# ################ Setting Rules for Binary Containers (Building Kernels) ################
$(TEMP_DIR)/misKernel.xo: $(SRCS)
	$(ECHO) "Compiling Kernel: misKernel"
	mkdir -p $(TEMP_DIR)
	$(VPP) -c $(misKernel_VPP_FLAGS) $(VPP_FLAGS) -k misKernel -I'$(<D)' --temp_dir $(TEMP_DIR) --report_dir $(TEMP_REPORT_DIR) -o'$@' '$<'

ifneq (,$(shell echo $(XPLATFORM) | awk '/u50/'))

$(BUILD_DIR)/misKernel.xclbin: $(BINARY_CONTAINER_misKernel_OBJS)
	mkdir -p $(BUILD_DIR)
	$(VPP) -l $(VPP_FLAGS) --temp_dir $(TEMP_DIR) --report_dir $(BUILD_REPORT_DIR)/misKernel $(VPP_LDFLAGS) $(VPP_LDFLAGS_misKernel) -o '$@' $(+)
else 
$(BUILD_DIR)/misKernel.xclbin: $(BINARY_CONTAINER_misKernel_OBJS)
	mkdir -p $(BUILD_DIR)
	$(VPP) -l $(VPP_FLAGS) --temp_dir $(TEMP_DIR) --report_dir $(BUILD_REPORT_DIR)/misKernel $(VPP_LDFLAGS) $(VPP_LDFLAGS_misKernel) -o '$@' $(+)
endif


# ################################# Cleaning Rules ##################################
cleanh:
	-$(RMDIR) vitis_* TempConfig system_estimate.xtxt *.rpt .run/
	-$(RMDIR) src/*.ll _xocc_* .Xil dltmp* xmltmp* *.log *.jou *.wcfg *.wdb sample_link.ini sample_compile.ini obj* bin* *.csv *.jpg *.jpeg *.png

cleank:
	-$(RMDIR) $(BUILD_DIR)/*.xclbin _vimage *xclbin.run_summary qemu-memory-_* emulation/ _vimage/ pl* start_simulation.sh *.xclbin
	-$(RMDIR) _x_temp.*/_x.* _x_temp.*/.Xil _x_temp.*/profile_summary.* xo_* _x*
	-$(RMDIR) _x_temp.*/dltmp* _x_temp.*/kernel_info.dat _x_temp.*/*.log 
	-$(RMDIR) _x_temp.* 

cleanall: cleanh cleank
	-$(RMDIR) $(BUILD_DIR)  build_dir.*  *.html $(TEMP_DIR) $(CUR_DIR)/reports *.csv *.run_summary $(CUR_DIR)/*.raw package_* run_script.sh .ipcache *.str
	-$(RMDIR) $(XFLIB_DIR)/common/data/*.xe2xd* $(XFLIB_DIR)/common/data/*.orig*

	-$(RMDIR) $(AIE_CONTAINERS) $(CUR_DIR)/Work $(CUR_DIR)/*.xpe $(CUR_DIR)/hw.o $(CUR_DIR)/*.xsa $(CUR_DIR)/xnwOut aiesimulator_output .AIE_SIM_CMD_LINE_OPTIONS

clean: cleanh

