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

.DELETE_ON_ERROR:
.PHONY: all	
all: stage
#all: stage cppBuild pythonApi  

SHELL := /bin/bash

ifndef XILINX_XRT
    XILINX_XRT = /opt/xilinx/xrt
    export XILINX_XRT
endif

ifndef XILINX_PLATFORMS
    XILINX_PLATFORMS = /opt/xilinx/platforms
    export XILINX_PLATFORMS
endif

#
# Global Definitions
#

## Location of graphanalytics project
GRAPH_ANALYTICS_DIR = ..

# General output directory for intermediate build artifacts
BUILD_DIR = build
# Product version and name
PRODUCT_VER=$(strip $(shell cat VERSION))
STANDALONE_NAME=mis

#######################################################################################################################
#
# C++ API Library
#

CCC = g++
CXX = g++
# -D PRINTINFO: print additional debug messages
CXXFLAGS = -std=c++14 -DPRAGMA -Ofast -fopenmp -fmessage-length=0  -fPIC -Wall -Wno-unknown-pragmas -Wno-unused-label \
    -Wno-unused-variable -Wno-sign-compare -Wno-unused-result -Wno-unused-but-set-variable \
    -Wno-narrowing -D USE_HBM -fmessage-length=0 

# Define the target directories.
#DEBUG = 1
ifdef DEBUG
CPP_BUILD_DIR = Debug
CXXFLAGS += -O0 -g
else
CPP_BUILD_DIR = Release
CXXFLAGS += -O3 -DNDEBUG
#CXXFLAGS += -O3 -g
endif

USE_F1 = 0
ifeq ($(USE_F1),1)
    CXXFLAGS += -D USE_F1
endif

## Target: libMis.so
INCLUDES_libMis = \
	-Iinclude \
	-I$(GRAPH_ANALYTICS_DIR)/mis/include \
	-I$(XILINX_XRT)/include \
	-I$(GRAPH_ANALYTICS_DIR)/L2/include \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include \
	-isystem $(GRAPH_ANALYTICS_DIR)/ext/HLS_arbitrary_Precision_Types/include \
	-I$(GRAPH_ANALYTICS_DIR)/ext/xcl2 \
	-I$(GRAPH_ANALYTICS_DIR)/ext
#	-I$(GRAPH_ANALYTICS_DIR)/plugin/tigergraph/comdetect/udf/QueryUdf 
#	-I${JAVA_HOME}/include \
#	-I${JAVA_HOME}/include/linux

LDLIBS_libMis = \
	
LIB_SHORT_NAME = XilinxMis
LIB_NAME = lib$(LIB_SHORT_NAME).so
LOADER_SHORT_NAME = XilinxMis_loader
LOADER_NAME = lib$(LOADER_SHORT_NAME).a

SRCS_L3 = $(GRAPH_ANALYTICS_DIR)/ext/xcl2/xcl2.cpp
OBJS_L3 = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_L3:.cpp=.o)))
# louvain dynamic load 

SRCS_loader = src/mis_loader.cpp
OBJS_loader = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_loader:.cpp=.o)))
DEPS_loader = $(OBJS_loader:.o=.d)

SRCS_top = $(filter-out $(SRCS_loader),$(wildcard src/*.cpp))
#SRCS_top = $(wildcard src/*.cpp) 
OBJS_top = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_top:.cpp=.o)))

SRCS_all = $(SRCS_L3) $(SRCS_top) $(SRCS_loader)
#SRCS_all = $(SRCS_L3) $(SRCS_top) 
OBJS_all = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_all:.cpp=.o)))
DEPS_all = $(OBJS_all:.o=.d)

OBJS_libMis = $(OBJS_L3) $(OBJS_top) 


.PHONY: cppBuild cppBuild2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
cppBuild: $(CPP_BUILD_DIR)
	@make cppBuild2

#cppBuild2: $(CPP_BUILD_DIR)/$(LIB_NAME) 
cppBuild2: $(CPP_BUILD_DIR)/$(LIB_NAME) $(CPP_BUILD_DIR)/$(LOADER_NAME)

# Create the target directory (if needed)
$(CPP_BUILD_DIR):
	mkdir -p $(CPP_BUILD_DIR)

# .so

LIB_DEPS = -L$(XILINX_XRT)/lib -lOpenCL -lxrt_coreutil  -lpthread -lrt -Wno-unused-label -Wno-narrowing -DVERBOSE -fopenmp

ifeq ($(MEMALLOC),tcmalloc)
    LIB_DEPS += -ltcmalloc
endif

# SHAREDLIB_FLAGS_libMis = -shared
SHAREDLIB_FLAGS_libMis = -fPIC -rdynamic -shared -w -Wl,--export-dynamic -Wunused-variable

$(CPP_BUILD_DIR)/$(LIB_NAME): $(OBJS_libMis) $(DEPLIBS_libMis)
	$(LINK.cc) -o $@ $(OBJS_libMis) $(SHAREDLIB_FLAGS_libMis) $(LDLIBS_libMis) $(LIB_DEPS)

# loader .a

$(CPP_BUILD_DIR)/$(LOADER_NAME): $(OBJS_loader) $(DEPLIBS_loader)
	ar ru $@ $(OBJS_loader)
	ranlib $@

# Macro to create a .o rule and a .d rule for each .cpp

define BUILD_CPP_RULE

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o): $(1)
	$(COMPILE.cc) $(INCLUDES_libMis) -o $$@ $$<

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).d): $(1)
	@set -e; \
	rm -f $$@; \
	$(COMPILE.cc) -MM -MT $(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o) $(INCLUDES_libMis) -MF $$@.$$$$$$$$ $$<; \
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

#######################################################################################################################
#
# XCLBIN
#
INC_ALL_XCLBINS=1

# U50 XCLBIN 
L2_TOP_DIR_U50 = $(GRAPH_ANALYTICS_DIR)/L2/mis_kernel
L2_XDEVICE_U50 = xilinx_u50_gen3x16_xdma_201920_3
L2_BUILD_DIR_U50 = $(L2_TOP_DIR_U50)/build_dir.hw.$(L2_XDEVICE_U50)
XCLBIN_NAME_U50 = findMIS.xclbin
XCLBIN_FILE_U50 = $(L2_BUILD_DIR_U50)/$(XCLBIN_NAME_U50)
XCLBIN_INSTALLED_NAME_U50 = mis_xilinx_u50_gen3x16_xdma_201920_3.xclbin
XCLBIN_INSTALLED_U50 =/opt/xilinx/apps/graphanalytics/mis/$(PRODUCT_VER)/xclbin/$(XCLBIN_INSTALLED_NAME_U50)

# U55 XCLBIN 
L2_TOP_DIR_U55 = $(GRAPH_ANALYTICS_DIR)/L2/mis_kernel
L2_XDEVICE_U55 = xilinx_u55c_gen3x16_xdma_2_202110_1
L2_BUILD_DIR_U55 = $(L2_TOP_DIR_U55)/build_dir.hw.$(L2_XDEVICE_U55)
XCLBIN_NAME_U55 = findMIS.xclbin
XCLBIN_FILE_U55 = $(L2_BUILD_DIR_U55)/$(XCLBIN_NAME_U55)
XCLBIN_INSTALLED_NAME_U55 = mis_$(L2_XDEVICE_U55).xclbin
XCLBIN_INSTALLED_U55 =/opt/xilinx/apps/graphanalytics/mis/$(PRODUCT_VER)/xclbin/$(XCLBIN_INSTALLED_NAME_U55)

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
	    echo "INFO: The XCLBIN file $(XCLBIN_FILE_U50) is missing."; \
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
# C++ Tests
#

TEST_DIR = tests

INCLUDES_test = \
    -Iinclude \
	-I$(XILINX_XRT)/include \
	-I$(GRAPH_ANALYTICS_DIR)/L2/include \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include \
	-I$(GRAPH_ANALYTICS_DIR)/ext \
	-I$(GRAPH_ANALYTICS_DIR)/ext/xcl2 \
	-isystem $(GRAPH_ANALYTICS_DIR)/ext/HLS_arbitrary_Precision_Types/include 


SRC_FILE_NAMES_test = \
    mis_test.cpp 

SRCS_test = $(addprefix $(TEST_DIR)/,$(SRC_FILE_NAMES_test))
OBJS_test = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_test:.cpp=.o)))
DEPS_test = $(OBJS_test:.o=.d)

# List of all test executables to build
EXEC_FILE_NAMES_test = \
    mis_test \

EXECS_test = $(addprefix $(CPP_BUILD_DIR)/,$(EXEC_FILE_NAMES_test))

.PHONY: cppTest cppTest2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
cppTest: cppBuild
	@make cppTest2

cppTest2: $(EXECS_test)

# Test executables
LDFLAGS_test = -fPIC -w -L $(CPP_BUILD_DIR) -l$(LIB_SHORT_NAME)

$(CPP_BUILD_DIR)/mis_test: $(SRCS_test) $(CPP_BUILD_DIR)/$(LIB_NAME)
	$(LINK.cc) -o $@ $< $(LDFLAGS_test) $(LIB_DEPS) $(INCLUDES_test)

###############################################################################
# Test targets and parameters

deviceNames=xilinx_u50_gen3x16_xdma_201920_3
ifeq ($(deviceNames),xilinx_u50_gen3x16_xdma_201920_3)
    XCLBIN_RUN = $(CURDIR)/staging/xclbin/$(XCLBIN_INSTALLED_NAME_U50)
else ifeq ($(deviceNames),xilinx_u55c_gen3x16_xdma_base_2)
    XCLBIN_RUN = $(CURDIR)/staging/xclbin/$(XCLBIN_INSTALLED_NAME_U55)		
endif


run: cppTest stage
	set -e; \
	. $(XILINX_XRT)/setup.sh && \
	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$$LD_LIBRARY_PATH $(CPP_BUILD_DIR)/mis_test \
	    -xclbin $(XCLBIN_RUN) -d ./examples/data  --devices $(deviceNames) 

run-staging-cpp: cppTest stage
	cd staging/examples/cpp && make run

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

# Expand the macro for each test source file

$(foreach src,$(SRCS_test),$(eval $(call BUILD_CPP_RULE,$(src))))

# Include generated header dependency rules, but only if building the JNI .so
# (This restriction prevents problems with the dependency on the Java API for the JNI header.)

ifeq ($(MAKECMDGOALS),cppTest2)
-include $(DEPS_test)
endif


#######################################################################################################################
#
# Staging
#

STAGE_DIR = staging

# Files to be direct-copied from source tree to staging area

STAGE_COPY_FILES = \
    VERSION \
    include/xilinxmis.hpp \
    src/mis_loader.cpp

define STAGE_COPY_RULE
$(STAGE_DIR)/$(1): $(1)
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
	cpp/misdemo.cpp cpp/Makefile \
	data/rowPtr.bin data/colIdx.bin data/infos.txt

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
    $(STAGE_DIR)/$(STAGE_EXAMPLE_ZIP) \
	$(STAGE_XILINX_APPS_COMMON) \

# Conditionally add XCLBIN files
ifeq ($(INC_ALL_XCLBINS),1) 
	STAGE_ALL_FILES += $(STAGE_XCLBIN_FILE_U50) $(STAGE_XCLBIN_FILE_U55)
else ifeq ($(deviceNames),xilinx_u50_gen3x16_xdma_201920_3)
    STAGE_ALL_FILES += $(STAGE_XCLBIN_FILE_U50)
else ifeq ($(deviceNames),xilinx_u55c_gen3x16_xdma_2_202110_1)
    STAGE_ALL_FILES += $(STAGE_XCLBIN_FILE_U55)
endif



# make staging areaing in two steps so intermediate outputs can be included in the 
# staging output as well.
stage: cppBuild
	make stage2 INC_ALL_XCLBINS=$(INC_ALL_XCLBINS) deviceNames=$(deviceNames)

stage2: $(STAGE_SUBDIRS) stage-examples $(STAGE_ALL_FILES)

$(STAGE_SUBDIRS):
	mkdir -p $@

# Common targets for all products
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
.PHONY: clean clean-xclbin clean-dist

run: 

clean:
	rm -rf Debug Release $(JAVA_LIB_DIR) $(BUILD_DIR) $(STAGE_DIR)
	@echo "Note: because the XCLBIN file takes so long to generate, it is not removed with this 'clean' target."
	@echo "To clean the XCLBIN, use 'make clean-xclbin'"
	
clean-xclbin:
	rm -f $(XCLBIN_FILE_U50)

clean-dist:
	@cd package; \
	make clean


cleanall: clean cleanpy
	rm -f ./package/*.deb ./package/*.rpm
	

help:
	@echo "Makefile usages:"
	@echo "  make run [deviceNames=xilinx_u50_gen3x16_xdma_201920_3]"
	@echo "    Run standalone test with statically linked executable"
	@echo ""
	@echo "  Run options:"
	@echo "  deviceNames  : Specify Alveo devices to use for computation"
	@echo "                 Supported devices:"
	@echo "                 xilinx_u50_gen3x16_xdma_201920_3 (default)"
	@echo "                 xilinx_u55c_gen3x16_xdma_base_2"
	@echo "  totalEntities: total number of entities to match against(default=10000000)"
	@echo ""
	@echo "  make all"
	@echo "  Compile MIS lib and generate staging area"
	@echo "" 
	@echo "  make dist"
	@echo "  Generate product installation package (RPM or DEB) for the current OS and architecture"
	@echo "" 
	@echo "  make clean-dist"
	@echo "  Clean build area for distribution packages"
	@echo "" 
	@echo "  make run-staging-cpp"
	@echo "  Run the CPP example from the staging area"
	@echo ""
	@echo "  Common options"
	@echo "  INC_ALL_XCLBINS: 1: Include all supported XCLBINs for staging (default). 0: Only incude target XCLBIN"