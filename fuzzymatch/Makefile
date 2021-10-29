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

#all: javaApi cppBuild javaTest stage
all: stage
#all: stage cppBuild pythonApi  
help:
	@echo "Makefile usages:"
	@echo "  make run [runMode=0] [deviceNames=xilinx_u50_gen3x16_xdma_201920_3] [totalEntities=10000000]"
	@echo "    Run standalone test with statically linked executable"
	@echo ""
	@echo "  Run options:"
	@echo "  deviceNames  : Specify Alveo devices to use for computation"
	@echo "                 Supported devices:"
	@echo "                 xilinx_u50_gen3x16_xdma_201920_3 (default)"
	@echo "                 xilinx_aws-vu9p-f1_shell-v04261818_201920_2"
	@echo "  numEntities  : Number of entities to match (default=100)"
	@echo "  runMode      : 0: Run on Alveo Only (default)"
	@echo "                 1: Run on CPU Only"
	@echo "                 2: Run on both Alveo and CPU"
	@echo "  totalEntities: total number of entities to match against(default=10000000)"
	@echo ""
	@echo "  make all"
	@echo "  Compile fuzzyMatch lib and generate staging area"
	@echo "" 
	@echo "  make dist"
	@echo "  Generate product installation package (RPM or DEB) for the current OS and architecture"
	@echo "" 
	@echo "  make run-staging-cpp"
	@echo "  Run the CPP example from the staging area"
	@echo ""
	@echo "  Common options"
	@echo "  INC_ALL_XCLBINS: 1: Include all supported XCLBINs for staging (default). 0: Only incude target XCLBIN"

SHELL := /bin/bash

INC_ALL_XCLBINS=1

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
# Product version
PRODUCT_VER=$(strip $(shell cat VERSION))

#######################################################################################################################
#
# C++ API Library
#

CCC = g++
CXX = g++
# -D PRINTINFO: print additional debug messages
CXXFLAGS = -std=c++11 -DPRAGMA -D_GLIBCXX_USE_CXX11_ABI=0 -Ofast -fopenmp -fmessage-length=0  -fPIC -Wall -Wno-unknown-pragmas -Wno-unused-label \
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

## Target: libFuzzyMatch.so
INCLUDES_libFuzzyMatch = \
	-Iinclude \
	-I$(GRAPH_ANALYTICS_DIR)/fuzzysearch/include \
	-I$(XILINX_XRT)/include \
	-I$(GRAPH_ANALYTICS_DIR)/L2/include \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include \
	-isystem $(GRAPH_ANALYTICS_DIR)/ext/HLS_arbitrary_Precision_Types/include \
	-I$(GRAPH_ANALYTICS_DIR)/ext/xcl2 \
	-I$(GRAPH_ANALYTICS_DIR)/ext
#	-I$(GRAPH_ANALYTICS_DIR)/plugin/tigergraph/comdetect/udf/QueryUdf 
#	-I${JAVA_HOME}/include \
#	-I${JAVA_HOME}/include/linux

LDLIBS_libFuzzyMatch = \
	
LIB_SHORT_NAME = XilinxFuzzyMatch
LIB_NAME = lib$(LIB_SHORT_NAME).so
LOADER_SHORT_NAME = XilinxFuzzyMatch_loader
LOADER_NAME = lib$(LOADER_SHORT_NAME).a

SRCS_L3 = $(GRAPH_ANALYTICS_DIR)/ext/xcl2/xcl2.cpp
OBJS_L3 = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_L3:.cpp=.o)))
# louvain dynamic load 

SRCS_loader = src/fuzzymatch_loader.cpp
OBJS_loader = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_loader:.cpp=.o)))
DEPS_loader = $(OBJS_loader:.o=.d)

SRCS_top = $(filter-out $(SRCS_loader),$(wildcard src/*.cpp))
#SRCS_top = $(wildcard src/*.cpp) 
OBJS_top = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_top:.cpp=.o)))

SRCS_all = $(SRCS_L3) $(SRCS_top) $(SRCS_loader)
#SRCS_all = $(SRCS_L3) $(SRCS_top) 
OBJS_all = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_all:.cpp=.o)))
DEPS_all = $(OBJS_all:.o=.d)

OBJS_libFuzzyMatch = $(OBJS_L3) $(OBJS_top) 


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

LIB_DEPS = -L$(XILINX_XRT)/lib -lOpenCL -lpthread -lrt -Wno-unused-label -Wno-narrowing -DVERBOSE

ifeq ($(MEMALLOC),tcmalloc)
    LIB_DEPS += -ltcmalloc
endif

# SHAREDLIB_FLAGS_libFuzzyMatch = -shared
SHAREDLIB_FLAGS_libFuzzyMatch = -fPIC -rdynamic -shared -w -Wl,--export-dynamic -Wunused-variable

$(CPP_BUILD_DIR)/$(LIB_NAME): $(OBJS_libFuzzyMatch) $(DEPLIBS_libFuzzyMatch)
	$(LINK.cc) -o $@ $(OBJS_libFuzzyMatch) $(SHAREDLIB_FLAGS_libFuzzyMatch) $(LDLIBS_libFuzzyMatch) $(LIB_DEPS)

# loader .a

$(CPP_BUILD_DIR)/$(LOADER_NAME): $(OBJS_loader) $(DEPLIBS_loader)
	ar ru $@ $(OBJS_loader)
	ranlib $@

# Macro to create a .o rule and a .d rule for each .cpp

define BUILD_CPP_RULE

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o): $(1)
	$(COMPILE.cc) $(INCLUDES_libFuzzyMatch) -o $$@ $$<

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).d): $(1)
	@set -e; \
	rm -f $$@; \
	$(COMPILE.cc) -MM -MT $(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o) $(INCLUDES_libFuzzyMatch) -MF $$@.$$$$$$$$ $$<; \
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
# U50 XCLBIN 
L2_TOP_DIR_U50 = $(GRAPH_ANALYTICS_DIR)/L2/fuzzy_kernel
L2_XDEVICE_U50 = xilinx_u50_gen3x16_xdma_201920_3
L2_BUILD_DIR_U50 = $(L2_TOP_DIR_U50)/build_dir.hw.$(L2_XDEVICE_U50)
XCLBIN_NAME_U50 = fuzzy_xilinx_u50_gen3x16_xdma_201920_3.xclbin 
XCLBIN_FILE_U50 = $(L2_BUILD_DIR_U50)/$(XCLBIN_NAME_U50)
XCLBIN_INSTALLED_NAME_U50 = $(XCLBIN_NAME_U50)
XCLBIN_INSTALLED_U50 =/opt/xilinx/apps/graphanalytics/fuzzymatch/1.0/xclbin/$(XCLBIN_INSTALLED_NAME_U50)

XCLBIN_BUILD_DIR=$(L2_TOP_DIR_U50)


# F1 XCLBIN 
L2_TOP_DIR_F1 = $(GRAPH_ANALYTICS_DIR)/L2/fuzzy_kernel
L2_XDEVICE_F1 = xilinx_aws-vu9p-f1_shell-v04261818_201920_2
L2_BUILD_DIR_F1 = $(L2_TOP_DIR_F1)/build_dir.hw.$(L2_XDEVICE_F1)
XCLBIN_NAME_F1 = fuzzy_xilinx_aws-vu9p-f1_shell-v04261818_201920_2.awsxclbin
XCLBIN_FILE_F1 = $(L2_BUILD_DIR_F1)/$(XCLBIN_NAME_F1)
XCLBIN_INSTALLED_NAME_F1 = $(XCLBIN_NAME_F1)
XCLBIN_INSTALLED_F1 =/opt/xilinx/apps/graphanalytics/fuzzymatch/1.0/xclbin/$(XCLBIN_INSTALLED_NAME_F1)

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

$(XCLBIN_FILE_F1):
	@if [ -r $(XCLBIN_INSTALLED_F1) ]; then \
	    echo "INFO: The XCLBIN file $(XCLBIN_FILE_F1) is missing.  Copying from installed area."; \
	    cp -r $(XCLBIN_INSTALLED_F1) $@; \
	else \
	    echo "ERROR: The XCLBIN file $(XCLBIN_FILE_F1) is missing."; \
	    echo "Because building an XCLBIN file can be an all-day proposition, you have two choices for completing this build:"; \
	    echo "1. Copy a pre-built XCLBIN to the target location above.  (Recommended for developers)"; \
	    echo "2. Do 'make xclbin' and go for a very long coffee break.  (Recommended for build automation)"; \
	    false; \
	fi
#######################################################################################################################
#
# python API wrapper
#

PYTHON = python3
PYTHON_DIR = /usr/include/$(PYTHON)
PYTHONENV_NAME = pyvenv
PYTHONENV = ./$(PYTHONENV_NAME)/bin/$(PYTHON)
PYTHON_API_DIR = wrappers/python
PYTHON_LIB_NAME := xilFuzzyMatchPython.so #$(shell python-config --extension-suffix)

SRC_FILE_NAMES_python = pythonapi.cpp
PYTHON_INC_FLAGS = $(shell python3-config --includes)
INCLUDES_python = \
    -I$(PYTHON_DIR) \
    -Iinclude \
	-I$(GRAPH_ANALYTICS_DIR)/ext \
	-I$(PYTHON_INC_FLAGS)

SRCS_python = $(addprefix $(PYTHON_API_DIR)/,$(SRC_FILE_NAMES_python))
OBJS_python = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_python:.cpp=.o)))

LIB_PATH = $(CPP_BUILD_DIR)

LDFLAGS_python = -L$(LIB_PATH) -l$(LIB_SHORT_NAME)

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
	$(PYTHONENV) -m pip install wheel; \
	$(PYTHONENV) -m pip install pybind11; \
	$(PYTHONENV) -m pip install python-config

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
	$(COMPILE.cc) -MM -MT $(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o) $(INCLUDES_python)   -MF $$@.$$$$$$$$ $$<; \
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
	-I$(XILINX_XRT)/include \
	-I$(GRAPH_ANALYTICS_DIR)/L2/include \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include \
	-I$(GRAPH_ANALYTICS_DIR)/ext \
	-I$(GRAPH_ANALYTICS_DIR)/ext/xcl2 \
	-isystem $(GRAPH_ANALYTICS_DIR)/ext/HLS_arbitrary_Precision_Types/include 


SRC_FILE_NAMES_test = \
    fuzzymatch_test.cpp 

SRCS_test = $(addprefix $(TEST_DIR)/,$(SRC_FILE_NAMES_test))
OBJS_test = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_test:.cpp=.o)))
DEPS_test = $(OBJS_test:.o=.d)

# List of all test executables to build
EXEC_FILE_NAMES_test = \
    fuzzymatch_test \

EXECS_test = $(addprefix $(CPP_BUILD_DIR)/,$(EXEC_FILE_NAMES_test))

.PHONY: cppTest cppTest2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
cppTest: cppBuild
	@make cppTest2

cppTest2: $(EXECS_test)

# Test executables
LDFLAGS_test = -fPIC -w -L $(CPP_BUILD_DIR) -l$(LIB_SHORT_NAME)

$(CPP_BUILD_DIR)/fuzzymatch_test: $(CPP_BUILD_DIR)/fuzzymatch_test.o $(CPP_BUILD_DIR)/$(LIB_NAME)
	$(LINK.cc) -o $@ $< $(LDFLAGS_test) $(LIB_DEPS)

###############################################################################
# Test targets and parameters

deviceNames=xilinx_u50_gen3x16_xdma_201920_3
ifeq ($(deviceNames),xilinx_u50_gen3x16_xdma_201920_3)
    XCLBIN_RUN = $(CURDIR)/staging/xclbin/$(XCLBIN_INSTALLED_NAME_U50)
else ifeq ($(deviceNames),xilinx_aws-vu9p-f1_shell-v04261818_201920_2)
    XCLBIN_RUN = $(CURDIR)/staging/xclbin/$(XCLBIN_INSTALLED_NAME_F1)	
endif

runMode=0
totalEntities=10000000
numEntities=100
run: cppTest stage
	set -e; \
	. $(XILINX_XRT)/setup.sh && \
	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$$LD_LIBRARY_PATH $(CPP_BUILD_DIR)/fuzzymatch_test \
	    -xclbin $(XCLBIN_RUN) -d ./examples/data -c $(runMode) --devices $(deviceNames) \
		--total_entities $(totalEntities) --num_entities $(numEntities)

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
    include/fuzzymatch.hpp \
    src/fuzzymatch_loader.cpp \
	src/fuzzymatch_utils.cpp \
	src/fuzzymatch.cpp

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
	cpp/fuzzymatch_test.cpp cpp/Makefile cpp/README.md \
	data/people.csv data/README.md data/txdata.csv \
	python/pythondemo.py python/run.sh

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
STAGE_XCLBIN_NAME_F1 = $(XCLBIN_INSTALLED_NAME_F1)
STAGE_XCLBIN_FILE_F1 = $(STAGE_DIR)/xclbin/$(STAGE_XCLBIN_NAME_F1)

$(STAGE_XCLBIN_FILE_U50): $(XCLBIN_FILE_U50)
	cp -f $^ $@

$(STAGE_XCLBIN_FILE_F1): $(XCLBIN_FILE_F1)
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
	STAGE_ALL_FILES += $(STAGE_XCLBIN_FILE_U50) $(STAGE_XCLBIN_FILE_F1)
else ifeq ($(deviceNames),xilinx_u50_gen3x16_xdma_201920_3)
    STAGE_ALL_FILES += $(STAGE_XCLBIN_FILE_U50)
else ifeq ($(deviceNames),xilinx_aws-vu9p-f1_shell-v04261818_201920_2)
    STAGE_ALL_FILES += $(STAGE_XCLBIN_FILE_F1)
endif



# make staging areaing in two steps so intermediate outputs can be included in the 
# staging output as well.
stage: pythonApi
	make stage2 INC_ALL_XCLBINS=$(INC_ALL_XCLBINS) deviceNames=$(deviceNames)

stage2: $(STAGE_SUBDIRS) stage-examples $(STAGE_ALL_FILES)

$(STAGE_SUBDIRS):
	mkdir -p $@


#######################################################################################################################
#
# Packaging
#

OSDIST = $(shell lsb_release -si)
DIST_TARGET =
ifeq ($(OSDIST),Ubuntu)
    DIST_TARGET = DEB
else ifeq ($(OSDIST),CentOS)
    DIST_TARGET = RPM
endif

.PHONY: dist

dist: stage
	@cd package; \
	if [ "$(DIST_TARGET)" == "" ]; then \
	    echo "Packaging is supported for only Ubuntu and CentOS."; \
	else \
	    echo "Packaging $(DIST_TARGET) for $(OSDIST)"; \
	    make ; \
	fi

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

run: 

clean:
	rm -rf Debug Release $(JAVA_LIB_DIR) $(BUILD_DIR) $(STAGE_DIR)
	@echo "Note: because the XCLBIN file takes so long to generate, it is not removed with this 'clean' target."
	@echo "To clean the XCLBIN, use 'make clean-xclbin'"
	
clean-xclbin:
	rm -f $(XCLBIN_FILE_U50)


cleanpy:
	rm -rf $(PYTHONENV_NAME)

cleanall: clean cleanpy
	rm -f ./package/*.deb ./package/*.rpm
	




