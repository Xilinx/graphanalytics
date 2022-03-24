#
# Copyright 2020-2021 Xilinx, Inc.
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

SHELL := /bin/bash

# Product name and version
STANDALONE_NAME=louvainmod
PRODUCT_VER=$(strip $(shell cat VERSION))

.DELETE_ON_ERROR:
.PHONY: all

#all: javaApi cppBuild javaTest stage
all: stage

#
# Global Definitions
#

## Location of graphanalytics project
GRAPH_ANALYTICS_DIR = ..

# General output directory for intermediate build artifacts
BUILD_DIR = build
# Product version
PRODUCT_VER=$(strip $(shell cat VERSION))

#######################################################################################################################
#
# C++ API Library
#

CCC = g++
CXX = g++
#CXX := /tools/batonroot/rodin/devkits/lnx64/gcc-6.2.0/bin/g++
#$(warning [WARNING]: CXX is $(CXX). Now there is an undetected bug by -O3 gcc-8.3.0, so just using 6.2.0!)
# -D PRINTINFO: print additional debug messages
CXXFLAGS = -std=c++11 -DPRAGMA -fopenmp -fmessage-length=0  -fPIC -Wall -Wno-unknown-pragmas -Wno-unused-label \
    -Wno-unused-variable -Wno-sign-compare -Wno-unused-result -Wno-unused-but-set-variable \
    -Wno-narrowing -D USE_HBM -fmessage-length=0 -D LOUVAINMOD 

# Define the target directories.
#DEBUG = 1
ifdef DEBUG
    CPP_BUILD_DIR = Debug
    #CXXFLAGS += -O0 -g -D_SINGLE_THREAD_
	#CXXFLAGS += -O0 -g
	CXXFLAGS += -O3 -Ofast
else
    CPP_BUILD_DIR = Release
    CXXFLAGS += -O3 -Ofast -DNDEBUG
endif

USE_U55C = 0
ifeq ($(USE_U55C),1)
    CXXFLAGS += -D USE_U55C
endif

## Target: libLouvain.so
INCLUDES_libLouvain = \
	-Iinclude \
	-I$(GRAPH_ANALYTICS_DIR)/louvainmod/grappolo/include \
	-I$(XILINX_XRT)/include \
	-I$(XILINX_XRM)/include \
	-I$(GRAPH_ANALYTICS_DIR)/L2/include \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include \
	-isystem $(GRAPH_ANALYTICS_DIR)/ext/HLS_arbitrary_Precision_Types/include \
	-I$(GRAPH_ANALYTICS_DIR)/ext/xcl2 \
	-I$(GRAPH_ANALYTICS_DIR)/ext \
	-I$(GRAPH_ANALYTICS_DIR)/plugin/tigergraph/comdetect/udf/QueryUdf \
#	-I${JAVA_HOME}/include \
#	-I${JAVA_HOME}/include/linux

LDLIBS_libLouvain = \
	
LIB_SHORT_NAME = XilinxLouvain
LIB_NAME = lib$(LIB_SHORT_NAME).so
LOADER_SHORT_NAME = XilinxLouvain_loader
LOADER_NAME = lib$(LOADER_SHORT_NAME).a

#SRCS_L3 = $(wildcard $(GRAPH_ANALYTICS_DIR)/L3/src/*.cpp) $(GRAPH_ANALYTICS_DIR)/ext/xcl2/xcl2.cpp

SRCS_L3_NAMES = \
	op_similaritydense.cpp \
	op_louvainmodularity.cpp \
	xf_graph_L3.cpp \
	xf_graph_L3_handle.cpp
# for louvain
SRCS_L3 = $(addprefix $(GRAPH_ANALYTICS_DIR)/L3/src/,$(SRCS_L3_NAMES)) $(GRAPH_ANALYTICS_DIR)/ext/xcl2/xcl2.cpp
OBJS_L3 = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_L3:.cpp=.o)))

# louvain dynamic loader
SRCS_loader = src/louvainmod_loader.cpp
OBJS_loader = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_loader:.cpp=.o)))
DEPS_loader = $(OBJS_loader:.o=.d)

SRCS_top = $(filter-out $(SRCS_loader),$(wildcard src/*.cpp)) $(wildcard grappolo/src/*.cpp)
OBJS_top = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_top:.cpp=.o)))

SRCS_all = $(SRCS_L3) $(SRCS_top) $(SRCS_loader)
OBJS_all = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_all:.cpp=.o)))
DEPS_all = $(OBJS_all:.o=.d)

OBJS_libLouvain = $(OBJS_L3) $(OBJS_top) 

CPP_DOXY_TARGET_DIR = $(BUILD_DIR)/cppdocs
CPP_DOXY_TRIGGER_FILE = $(CPP_DOXY_TARGET_DIR)/html/index.html

.PHONY: cppBuild cppBuild2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
cppBuild: $(CPP_BUILD_DIR)
	@make cppBuild2

cppBuild2: $(CPP_BUILD_DIR)/$(LIB_NAME) $(CPP_BUILD_DIR)/$(LOADER_NAME)

# Create the target directory (if needed)
$(CPP_BUILD_DIR):
	mkdir -p $(CPP_BUILD_DIR)

# .so

LIB_DEPS = -L$(XILINX_XRT)/lib -lOpenCL -lpthread -lrt -Wno-unused-label -Wno-narrowing -DVERBOSE -L$(XILINX_XRM)/lib -lxrm

ifeq ($(MEMALLOC),tcmalloc)
    LIB_DEPS += -ltcmalloc
endif

# SHAREDLIB_FLAGS_libLouvain = -shared
SHAREDLIB_FLAGS_libLouvain = -fPIC -rdynamic -shared -w -Wl,--export-dynamic -Wunused-variable

$(CPP_BUILD_DIR)/$(LIB_NAME): $(OBJS_libLouvain) $(DEPLIBS_libLouvain)
	$(LINK.cc) -o $@ $(OBJS_libLouvain) $(SHAREDLIB_FLAGS_libLouvain) $(LDLIBS_libLouvain) $(LIB_DEPS)

# loader .a

$(CPP_BUILD_DIR)/$(LOADER_NAME): $(OBJS_loader) $(DEPLIBS_loader)
	ar ru $@ $(OBJS_loader)
	ranlib $@

# Macro to create a .o rule and a .d rule for each .cpp

define BUILD_CPP_RULE

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o): $(1)
	$(COMPILE.cc) $(INCLUDES_libLouvain) -o $$@ $$<

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).d): $(1)
	@set -e; \
	rm -f $$@; \
	$(COMPILE.cc) -MM -MT $(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o) $(INCLUDES_libLouvain) -MF $$@.$$$$$$$$ $$<; \
        sed 's,\($(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o)\)[ :]*,\1 $$@ : ,g' < $$@.$$$$$$$$ > $$@; \
	rm -f $$@.$$$$$$$$

endef

# Expand the macro for each source file

$(foreach src,$(SRCS_all),$(eval $(call BUILD_CPP_RULE,$(src))))

# Include generated header dependency rules, but only if building the JNI .so
# (This restriction prevents problems with the dependency on the Java API for the JNI header.)

ifeq ($(MAKECMDGOALS),cppBuild2)
-include $(DEPS_all)
endif

#======================================================================================================================

# Doxygen output for proofreading documentation (as opposed to the process to publish on xilinx.github.io)

.PHONY: doxy-cpp

doxy-cpp: $(CPP_DOXY_TRIGGER_FILE)

CPP_DOXY_CONFIG_FILE = docs/cpp/DoxyFile

$(CPP_DOXY_TRIGGER_FILE): include/xilinxlouvain.hpp $(CPP_DOXY_CONFIG_FILE)
	@set -e; \
	if ! [ -x "$$(command -v doxygen)" ]; then \
	    echo "ERROR: doxygen appears not to be installed.  Please install at least version 1.8.5"; \
	    false; \
	fi; \
	mkdir -p $(CPP_DOXY_TARGET_DIR); \
	sed 's,LOUVAINMOD_VERSION,$(PRODUCT_VER),g' < $(CPP_DOXY_CONFIG_FILE) > $(CPP_DOXY_TARGET_DIR)/doxyfile.tmp; \
	doxygen $(CPP_DOXY_TARGET_DIR)/doxyfile.tmp; \
	rm -f $(CPP_DOXY_TARGET_DIR)/doxyfile.tmp


#######################################################################################################################
#
# python API wrapper
#

PYTHON = python3
PYTHON_DIR = /usr/include/$(PYTHON)
PYTHONENV_NAME = pyvenv
PYTHONENV = ./$(PYTHONENV_NAME)/bin/$(PYTHON)
PYTHON_API_DIR = wrappers/python
PYTHON_LIB_NAME := xilLouvainmod.so #$(shell python-config --extension-suffix)

SRC_FILE_NAMES_python = pythonapi.cpp

INCLUDES_python = \
    -I$(PYTHON_DIR) \
    -Iinclude

SRCS_python = $(addprefix $(PYTHON_API_DIR)/,$(SRC_FILE_NAMES_python))
OBJS_python = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_python:.cpp=.o)))

LIB_PATH = $(CPP_BUILD_DIR)

LDFLAGS_python = -L$(LIB_PATH) -l$(LIB_SHORT_NAME) -lzmq

# Add pybind11 includes and set library name after setting virtual env
ifeq ($(MAKECMDGOALS),pythonApi2)
INCLUDES_python += $(shell $(PYTHONENV) -m pybind11 --includes)
endif

SHAREDLIB_FLAGS_python = -shared

.PHONY: pythonApi pythonApi2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
pythonApi: cppBuild $(PYTHONENV_NAME)
	@make pythonApi2

$(PYTHONENV_NAME):
	$(PYTHON) -m venv $(PYTHONENV_NAME);\
	$(PYTHONENV) -m pip install pybind11
#	$(PYTHONENV) -m pip install python-config

pythonApi2: $(CPP_BUILD_DIR) $(CPP_BUILD_DIR)/$(PYTHON_LIB_NAME)

$(CPP_BUILD_DIR)/$(PYTHON_LIB_NAME): $(OBJS_python)
	$(LINK.cc) $^ -o $@ $(SHAREDLIB_FLAGS_python) $(LDFLAGS_python)

# Macro to create a .o rule and a .d rule for each .cpp

define BUILD_CPP_RULE

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o): $(1)
	$(COMPILE.cc) $(INCLUDES_python) -o $$@ $$<

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).d): $(1)
	@set -e; \
	rm -f $$@; \
	$(COMPILE.cc) -MM -MT $(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o) $(INCLUDES_python) -MF $$@.$$$$$$$$ $$<; \
        sed 's,\($(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o)\)[ :]*,\1 $$@ : ,g' < $$@.$$$$$$$$ > $$@; \
	rm -f $$@.$$$$$$$$

endef

# Expand the macro for each source file

$(foreach src,$(SRCS_python),$(eval $(call BUILD_CPP_RULE,$(src))))

#######################################################################################################################
#
# C++ Tests
#

TEST_DIR = tests

INCLUDES_test = \
	-Iinclude \
	-Itests \
	-Itests/findcommunities \
	-I$(XILINX_XRT)/include \
	-I$(XILINX_XRM)/include \
	-I$(GRAPH_ANALYTICS_DIR)/L2/include \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include \
	-I$(GRAPH_ANALYTICS_DIR)/ext \
	-I$(GRAPH_ANALYTICS_DIR)/ext/xcl2 \
	-isystem $(GRAPH_ANALYTICS_DIR)/ext/HLS_arbitrary_Precision_Types/include 

LDLIBS_test = \
	

FIND_COMMUNITIES_DIR = findcommunities
FIND_COMMUNITIES_SRC_FILE_NAMES = \
    community.cpp \
    graph.cpp \
    graph_binary.cpp \

FIND_COMMUNITIES_OBJS = $(addprefix $(CPP_BUILD_DIR)/,$(FIND_COMMUNITIES_SRC_FILE_NAMES:.cpp=.o))

SRC_FILE_NAMES_test = \
    cppdemo.cpp \
    pardump.cpp \
    islandsMain.cpp \
    louvain_test.cpp \
    $(addprefix $(FIND_COMMUNITIES_DIR)/,$(FIND_COMMUNITIES_SRC_FILE_NAMES))

SRCS_test = $(addprefix $(TEST_DIR)/,$(SRC_FILE_NAMES_test))
OBJS_test = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_test:.cpp=.o)))
DEPS_test = $(OBJS_test:.o=.d)

# List of all test executables to build
EXEC_FILE_NAMES_test = \
    cppdemo \
    pardump \
    islands \
    louvain_test

EXECS_test = $(addprefix $(CPP_BUILD_DIR)/,$(EXEC_FILE_NAMES_test))

.PHONY: cppTest cppTest2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
cppTest: cppBuild
	@make cppTest2

cppTest2: $(EXECS_test)

# Test executables

LDFLAGS_test = -fPIC -w -L $(CPP_BUILD_DIR) -l$(LIB_SHORT_NAME) -lzmq -fopenmp -m64

$(CPP_BUILD_DIR)/cppdemo: $(CPP_BUILD_DIR)/cppdemo.o $(CPP_BUILD_DIR)/$(LIB_NAME)
	$(LINK.cc) -o $@ $< $(LDFLAGS_test) $(LIB_DEPS)

CPPDEMO_OUT_DIR = $(CPP_BUILD_DIR)/cppdemo-out
graph = ./examples/data/as-Skitter-wt-r100.mtx
graphFilename = $(notdir $(graph))
graphBasename = $(basename $(graphFilename))
alveoProject = $(CPPDEMO_OUT_DIR)/$(graphBasename)
numNodes = 1
numPars = 9
numDevices = 1
communityOutputOpt =
ifneq ($(communityOutput),)
    communityOutputOpt = -o $(communityOutput)
endif

deviceNames=xilinx_u50_gen3x16_xdma_201920_3
ifeq ($(deviceNames),xilinx_u50_gen3x16_xdma_201920_3)
    XCLBIN_RUN = $(CURDIR)/staging/xclbin/louvainmod_pruning_xilinx_u50_gen3x16_xdma_201920_3.xclbin
	KERNEL_MODE = -kernel_mode 2
endif
ifeq ($(deviceNames),xilinx_u55c_gen3x16_xdma_base_2)
    XCLBIN_RUN = $(CURDIR)/staging/xclbin/louvainmod_2cu_xilinx_u55c_gen3x16_xdma_2_202110_1.xclbin
	KERNEL_MODE = -kernel_mode 4
endif

CMDLINE_CREATE_PARTITIONS = $(graph) $(KERNEL_MODE) -num_pars $(numPars) -create_alveo_partitions \
                            -name $(alveoProject)

WORKERS=192.168.1.21 192.168.1.31
CMDLINE_LOAD_COMPUTE = -x $(XCLBIN_RUN) $(KERNEL_MODE) -num_devices $(numDevices) -devices $(deviceNames) \
	-num_level 100 -num_iter 100 -load_alveo_partitions $(alveoProject).par.proj -setwkr 0 -driverAlone $(communityOutputOpt)

CMDLINE_LOAD_COMPUTE_DRIVER = -x $(XCLBIN_RUN) $(KERNEL_MODE) -num_nodes 3 -num_devices $(numDevices) \
	-num_level 100 -num_iter 100 -load_alveo_partitions $(alveoProject).par.proj -setwkr 2 $(WORKERS) -driverAlone $(communityOutputOpt)

CMDLINE_LOAD_COMPUTE_WORKER1 = -x $(XCLBIN_RUN) $(KERNEL_MODE) -num_nodes 3 -num_devices $(numDevices) \
	-num_level 100 -num_iter 100 -load_alveo_partitions $(alveoProject).par.proj -workerAlone 1

CMDLINE_LOAD_COMPUTE_WORKER2 = -x $(XCLBIN_RUN) $(KERNEL_MODE) -num_nodes 3 -num_devices $(numDevices) \
    -num_level 100 -num_iter 100 -load_alveo_partitions $(alveoProject).par.proj -workerAlone 2

.PHONY: run run-create-partitions run-load-compute run-load-compute-driver run-load-compute-worker1 run-load-compute-worker2

$(alveoProject).par.proj: $(graph)
	@echo "------------------------------------------------------------------------"
	@echo "-                       Create Partitions                              -"
	@echo "------------------------------------------------------------------------"
	rm -rf $(CPPDEMO_OUT_DIR)
	mkdir -p $(CPPDEMO_OUT_DIR)
	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$(LD_LIBRARY_PATH); $(CPP_BUILD_DIR)/cppdemo $(CMDLINE_CREATE_PARTITIONS)

run: run-load-compute

run-create-partitions: stage cppTest
	@echo "------------------------------------------------------------------------"
	@echo "-                       Create Partitions                              -"
	@echo "------------------------------------------------------------------------"
	rm -rf $(CPPDEMO_OUT_DIR)
	mkdir -p $(CPPDEMO_OUT_DIR)
	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$(LD_LIBRARY_PATH); $(CPP_BUILD_DIR)/cppdemo $(CMDLINE_CREATE_PARTITIONS)

run-load-compute: stage cppTest $(alveoProject).par.proj
	@echo "------------------------------------------------------------------------"
	@echo "-                    Load Partitions and Compute                       -"
	@echo "------------------------------------------------------------------------"
	set -e; \
	. $(XILINX_XRT)/setup.sh; \
	. $(XILINX_XRM)/setup.sh; \
	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$$LD_LIBRARY_PATH $(CPP_BUILD_DIR)/cppdemo $(CMDLINE_LOAD_COMPUTE)

run-load-compute-driver: stage cppTest $(alveoProject).par.proj
	set -e; \
	. $(XILINX_XRT)/setup.sh; \
	. $(XILINX_XRM)/setup.sh; \
	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$$LD_LIBRARY_PATH $(CPP_BUILD_DIR)/cppdemo $(CMDLINE_LOAD_COMPUTE_DRIVER)

run-load-compute-worker1: stage cppTest 
	set -e; \
	. $(XILINX_XRT)/setup.sh; \
	. $(XILINX_XRM)/setup.sh; \
	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$$LD_LIBRARY_PATH; $(CPP_BUILD_DIR)/cppdemo $(CMDLINE_LOAD_COMPUTE_WORKER1)

run-load-compute-worker2: stage cppTest
	set -e; \
	. $(XILINX_XRT)/setup.sh; \
	. $(XILINX_XRM)/setup.sh; \
	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$$LD_LIBRARY_PATH; $(CPP_BUILD_DIR)/cppdemo $(CMDLINE_LOAD_COMPUTE_WORKER2)	

debug-cppdemo: cppTest
	rm -rf $(CPPDEMO_OUT_DIR)
	mkdir -p $(CPPDEMO_OUT_DIR)
	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$(LD_LIBRARY_PATH); totalview $(CPP_BUILD_DIR)/cppdemo -a $(CMDLINE_CREATE_PARTITIONS)

$(CPP_BUILD_DIR)/pardump: $(CPP_BUILD_DIR)/pardump.o $(CPP_BUILD_DIR)/$(LIB_NAME)
	$(LINK.cc) -o $@ $< $(LDFLAGS_test) $(LIB_DEPS)

$(CPP_BUILD_DIR)/islands: $(CPP_BUILD_DIR)/islandsMain.o $(CPP_BUILD_DIR)/$(LIB_NAME)
	$(LINK.cc) -o $@ $< $(LDFLAGS_test) $(LIB_DEPS)

$(CPP_BUILD_DIR)/louvain_test: $(CPP_BUILD_DIR)/louvain_test.o $(FIND_COMMUNITIES_OBJS) $(CPP_BUILD_DIR)/$(LIB_NAME)
	$(LINK.cc) -o $@ $^ $(LDFLAGS_test) $(LIB_DEPS)

# Macro to create a .o rule and a .d rule for each .cpp

define BUILD_CPP_RULE

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o): $(1)
	$(COMPILE.cc) $(INCLUDES_test) -o $$@ $$<

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).d): $(1)
	@set -e; \
	rm -f $$@; \
	$(COMPILE.cc) -MM -MT $(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o) $(INCLUDES_test) -MF $$@.$$$$$$$$ $$<; \
        sed 's,\($(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o)\)[ :]*,\1 $$@ : ,g' < $$@.$$$$$$$$ > $$@; \
	rm -f $$@.$$$$$$$$

endef

# Expand the macro for each source file

$(foreach src,$(SRCS_test),$(eval $(call BUILD_CPP_RULE,$(src))))

# Include generated header dependency rules, but only if building the JNI .so
# (This restriction prevents problems with the dependency on the Java API for the JNI header.)

ifeq ($(MAKECMDGOALS),cppTest2)
-include $(DEPS_test)
endif


#######################################################################################################################
#
# XCLBIN
#
# U50 XCLBIN 
L2_TOP_DIR_U50 = $(GRAPH_ANALYTICS_DIR)/L2/louvainmod_pruning_kernel
L2_XDEVICE_U50 = xilinx_u50_gen3x16_xdma_201920_3
L2_BUILD_DIR_U50 = $(L2_TOP_DIR_U50)/build_dir.hw.$(L2_XDEVICE_U50)
XCLBIN_NAME_U50 = kernel_louvain_pruning_alias.xclbin
XCLBIN_FILE_U50 = $(L2_BUILD_DIR_U50)/$(XCLBIN_NAME_U50)
XCLBIN_INSTALLED_NAME_U50 = louvainmod_pruning_$(L2_XDEVICE_U50).xclbin
XCLBIN_INSTALLED_U50 =/opt/xilinx/apps/graphanalytics/louvainmod/1.0/xclbin/$(XCLBIN_INSTALLED_NAME_U50)
# U55 XCLBIN 
L2_TOP_DIR_U55 = $(GRAPH_ANALYTICS_DIR)/L2/louvainmod_2cu_u55c_kernel
L2_XDEVICE_U55 = xilinx_u55c_gen3x16_xdma_2_202110_1
L2_BUILD_DIR_U55 = $(L2_TOP_DIR_U55)/build_dir.hw.$(L2_XDEVICE_U55)
XCLBIN_NAME_U55 = kernel_louvain_2cu_u55c_alias.xclbin
XCLBIN_FILE_U55 = $(L2_BUILD_DIR_U55)/$(XCLBIN_NAME_U55)
XCLBIN_INSTALLED_NAME_U55 = louvainmod_2cu_$(L2_XDEVICE_U55).xclbin
XCLBIN_INSTALLED_U55 =/opt/xilinx/apps/graphanalytics/louvainmod/1.0/xclbin/$(XCLBIN_INSTALLED_NAME_U55)


ifeq ($(USE_U55C),1)
	XCLBIN_BUILD_DIR=$(L2_TOP_DIR_U55)
else
	XCLBIN_BUILD_DIR=$(L2_TOP_DIR_U50)
endif

.PHONY: xclbin
xclbin:
	cd $(XCLBIN_BUILD_DIR); \
	source $(XILINX_XRT)/setup.sh; \
	export PLATFORM_REPO_PATHS=$(XILINX_PLATFORMS); \
	make build TARGET=hw
	
$(XCLBIN_FILE_U50):
	@if [ -r $(XCLBIN_INSTALLED_U50) ]; then \
	    echo "INFO: The XCLBIN file $(XCLBIN_FILE_U50) is missing.  Copying from installed area."; \
	    cp -r $(XCLBIN_INSTALLED_U50) $@; \
	else \
	    echo "ERROR: The XCLBIN file $(XCLBIN_FILE_U50) is missing."; \
	    echo "Because building an XCLBIN file can be an all-day proposition, you have two choices for completing this build:"; \
	    echo "1. Copy a pre-built XCLBIN to the target location above.  (Recommended for developers)"; \
	    echo "2. Do 'make xclbin' and go for a very long coffee break.  (Recommended for build automation)"; \
	    false; \
	fi

	
$(XCLBIN_FILE_U55):
	@if [ -r $(XCLBIN_INSTALLED_U55) ]; then \
	    echo "INFO: The XCLBIN file $(XCLBIN_FILE_U50) is missing.  Copying from installed area."; \
	    cp -r $(XCLBIN_INSTALLED_U55) $@; \
	else \
	    echo "ERROR: The XCLBIN file $(XCLBIN_FILE_U55) is missing."; \
	    echo "Because building an XCLBIN file can be an all-day proposition, you have two choices for completing this build:"; \
	    echo "1. Copy a pre-built XCLBIN to the target location above.  (Recommended for developers)"; \
	    echo "2. Do 'make xclbin' and go for a very long coffee break.  (Recommended for build automation)"; \
	    false; \
	fi

#######################################################################################################################
#
# Staging
#

STAGE_DIR = staging

# Files to be direct-copied from source tree to staging area

STAGE_COPY_FILES = \
    VERSION \
    include/xilinxlouvain.h \
	include/app_common.h \
    src/louvainmod_loader.cpp

define STAGE_COPY_RULE
$$(STAGE_DIR)/$(1): $(1)
	cp -f $(1) $$(STAGE_DIR)/$(1)
endef
$(foreach f,$(STAGE_COPY_FILES),$(eval $(call STAGE_COPY_RULE,$(f))))

# Binary (C++) files

STAGE_LIB_FILES = \
    $(LIB_NAME) \
    $(LOADER_NAME)

ifneq ($(wildcard $(CPP_BUILD_DIR)/$(PYTHON_LIB_NAME)),)
    STAGE_LIB_FILES += $(PYTHON_LIB_NAME)
endif

define STAGE_LIB_RULE
$$(STAGE_DIR)/lib/$(1): $$(CPP_BUILD_DIR)/$(1)
	cp -f $$^ $$@
endef
$(foreach f,$(STAGE_LIB_FILES),$(eval $(call STAGE_LIB_RULE,$(f))))

# Example files - need to be zipped up

#STAGE_EXAMPLE_ZIP = examples.zip
STAGE_EXAMPLE_FILE_NAMES = \
	cpp/cppdemo.cpp cpp/Makefile cpp/README.md \
	data/as-Skitter-wt-r100.mtx

STAGE_EXAMPLE_FILES = $(addprefix examples/,$(STAGE_EXAMPLE_FILE_NAMES))
STAGE_STAGED_EXAMPLE_FILES = $(addprefix $(STAGE_DIR)/,$(STAGE_EXAMPLE_FILES))

.PHONY: stage stage2 fill-example-stage stage-examples

#$(STAGE_DIR)/$(STAGE_EXAMPLE_ZIP): $(STAGE_EXAMPLE_FILES)
#	mkdir -p $(sort $(dir $(STAGE_STAGED_EXAMPLE_FILES)))
#	@make fill-example-stage
#	@cd $(STAGE_DIR) ; \
#	zip -r $(STAGE_EXAMPLE_ZIP) examples
#	rm -rf $(STAGE_DIR)/examples

stage-examples: $(STAGE_EXAMPLE_FILES)
	mkdir -p $(sort $(dir $(STAGE_STAGED_EXAMPLE_FILES)))
	@make fill-example-stage

fill-example-stage: $(STAGE_STAGED_EXAMPLE_FILES)
	cp VERSION $(STAGE_DIR)/examples/
	
$(STAGE_STAGED_EXAMPLE_FILES):
	cp -f $(patsubst $(STAGE_DIR)/%,%,$@) $@
	

# XCLBIN file
STAGE_XCLBIN_NAME_U50 = $(XCLBIN_INSTALLED_NAME_U50)
STAGE_XCLBIN_FILE_U50 = $(STAGE_DIR)/xclbin/$(STAGE_XCLBIN_NAME_U50)
STAGE_XCLBIN_NAME_U55 = $(XCLBIN_INSTALLED_NAME_U55)
STAGE_XCLBIN_FILE_U55 = $(STAGE_DIR)/xclbin/$(STAGE_XCLBIN_NAME_U55)

$(STAGE_XCLBIN_FILE_U50): $(XCLBIN_FILE_U50)
	cp -f $^ $@

$(STAGE_XCLBIN_FILE_U55): $(XCLBIN_FILE_U55)
	cp -f $^ $@

STAGE_XILINX_APPS_COMMON = $(STAGE_DIR)/include/xilinx_apps_common.hpp
XILINX_APPS_COMMON = $(GRAPH_ANALYTICS_DIR)/ext/xilinx_apps_common.hpp
$(STAGE_XILINX_APPS_COMMON): $(XILINX_APPS_COMMON)
	cp -f $^ $@

# Top-level packaging rule

STAGE_SUBDIR_NAMES = include lib src xclbin
STAGE_SUBDIRS = $(addprefix $(STAGE_DIR)/,$(STAGE_SUBDIR_NAMES))
STAGE_ALL_FILES = \
    $(addprefix $(STAGE_DIR)/,$(STAGE_COPY_FILES)) \
    $(addprefix $(STAGE_DIR)/lib/,$(STAGE_LIB_FILES)) \
    $(STAGE_XCLBIN_FILE_U50) \
	$(STAGE_XCLBIN_FILE_U55) \
    $(STAGE_DIR)/$(STAGE_EXAMPLE_ZIP) \
	$(STAGE_XILINX_APPS_COMMON) \

# make staging areaing in two steps so intermediate outputs can be included in the 
# staging output as well.
stage: 
	make stage2

stage2: cppTest $(STAGE_SUBDIRS) stage-examples $(STAGE_ALL_FILES)

$(STAGE_SUBDIRS):
	mkdir -p $@

# Common targets for all products (packaging)
include ../common/Makefile-common.mk

#######################################################################################################################
#
# Java Tests
#
.PHONY: javaTest

javaTest:
	cd test/java && ant

#######################################################################################################################
#
# Clean
#

#### Clean target deletes all generated files ####
.PHONY: clean clean-xclbin

clean:
	rm -rf Debug Release $(JAVA_LIB_DIR) $(BUILD_DIR) $(STAGE_DIR)
	@echo "Note: because the XCLBIN file takes so long to generate, it is not removed with this 'clean' target."
	@echo "To clean the XCLBIN, use 'make clean-xclbin'"
	
clean-xclbin:
	rm -f $(XCLBIN_FILE_U50)

clean-par:
	rm -f $(CPPDEMO_OUT_DIR)/$(graphBasename)*

cleanpy:
	rm -rf $(PYTHONENV_NAME)

cleanall: clean cleanpy
	rm -f ./package/*.deb ./package/*.rpm
	

help:
	@echo "Makefile usages:"
	@echo "  make dist"
	@echo "  Generate product installation package (RPM or DEB) for the current OS and architecture"
	@echo ""
	@echo "  make doxy-cpp"
	@echo "  Generate CPP documentation using doxygen. The default html output directory is ./build/cppdocs"
	@echo ""
	@echo "  make run-create-partitions [graph=/path/to/graph.mtx] [numPars=1]"
	@echo "  Create partitions on a single node"
	@echo ""
	@echo "  make run [graph=/path/to/graph.mtx] [numPars=1] [numDevices=1] [deviceNames=xilinx_u50_gen3x16_xdma_201920_3]"
	@echo "  Load partition and compute Louvain modularity on a single node"
	@echo ""
	@echo "  Run test in a 3 node cluster"
	@echo "  ssh to driver"
	@echo "  make run-load-compute-driver [graph=/path/to/graph.mtx] [numNodes=1] [numDevices=1]"
	@echo ""
	@echo "  ssh to worker1"
	@echo "  make run-load-compute-worker1 [graph=/path/to/graph.mtx] [numNodes=1] [numDevices=1]"
	@echo ""
	@echo "  ssh to worker2"
	@echo "  make run-load-compute-worker2 [graph=/path/to/graph.mtx] [numNodes=1] [numDevices=1]"
	@echo ""
	@echo "  debug-cppdemo"
	@echo "  Debug cppdemo"
	@echo ""
	@echo "-------- Options --------"
	@echo "DEBUG          : 1: build application in debug mode. 0: build application in release mode."
	@echo "graph          : graph input .mtx file"
	@echo "alveoProject   : Alveo project name without .par.proj extension"
	@echo "numNodes       : Number of nodes (default=1)"
	@echo "deviceNames    : Target device name. Supported devices: "
	@echo "                 xilinx_u50_gen3x16_xdma_201920_3"
	@echo "                 xilinx_u55c_gen3x16_xdma_base_2"
	@echo "numPars        : Number of partitions (default=1)"
	@echo "numDevices     : Number of devices (default=1)"
	@echo "communityOutput: Output file for detected communities"
