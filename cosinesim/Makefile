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

#
# Define __PROFILING__ to turn on profiling message from compute units
#
ifndef XILINX_XRT
    XILINX_XRT = /opt/xilinx/xrt
    export XILINX_XRT
endif

ifndef XILINX_XRM
    XILINX_XRM=/opt/xilinx/xrm
    export XILINX_XRM
endif


.DELETE_ON_ERROR:
.PHONY: all

#all: javaApi cppBuild javaTest stage
all: stage

#
# Global Definitions
#

# Location of graphanalytics project
GRAPH_ANALYTICS_DIR = ..

# General output directory for intermediate build artifacts
BUILD_DIR = build

# Product version
PRODUCT_VER=$(strip $(shell cat VERSION))

#######################################################################################################################
#
# Java API
#

JAVA_LIB_DIR = lib
JAVA_BUILD_DIR = $(BUILD_DIR)
JAVA_SRC_DIR = java/src
JAVA_API_JAR = $(JAVA_LIB_DIR)/xilinxCosineSimApi-0.1.jar
JAVA_API_DOC_JAR = $(JAVA_LIB_DIR)/xilinxCosineSimApi-0.1-javadoc.jar
JAVA_JNI_HEADER = $(JAVA_BUILD_DIR)/api/c-headers/com_xilinx_xf_IntCosineSim.h

.PHONY: javaApi javaApiJar
	
javaApi: javaApiJar $(JAVA_API_DOC_JAR)
	
javaApiJar:
	cd java && ant jar
	
$(JAVA_API_DOC_JAR): $(JAVA_API_JAR)
	cd java && ant docs


#######################################################################################################################
#
# C++ API Library
#

CCC = g++
CXX = g++
CXXFLAGS = -std=c++11 -fPIC -Wall -Wno-unknown-pragmas -Wno-unused-label \
           -Wno-narrowing -D USE_HBM -fmessage-length=0 -D COSINESIM -D VERSION=\"$(PRODUCT_VER)\"

# Define the target directories.

ifdef DEBUG
CPP_BUILD_DIR = Debug
CXXFLAGS += -O0 -g
else
CPP_BUILD_DIR = Release
CXXFLAGS += -O3 -DNDEBUG
endif


## Target: libCosineSim.so
INCLUDES_libCosineSim = \
	-I$(XILINX_XRT)/include \
	-I$(XILINX_XRM)/include \
	-I$(GRAPH_ANALYTICS_DIR)/L2/include \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include \
	-I$(GRAPH_ANALYTICS_DIR)/ext \
	-I$(GRAPH_ANALYTICS_DIR)/ext/xcl2 \
	-Iinclude
#	-I${JAVA_HOME}/include \
#	-I${JAVA_HOME}/include/linux

LDLIBS_libCosineSim = \
	
LIB_SHORT_NAME = XilinxCosineSim
LIB_NAME = lib$(LIB_SHORT_NAME).so
LOADER_SHORT_NAME = XilinxCosineSim_loader
LOADER_NAME = lib$(LOADER_SHORT_NAME).a

#SRCS_L3 = $(wildcard $(GRAPH_ANALYTICS_DIR)/L3/src/*.cpp) $(GRAPH_ANALYTICS_DIR)/ext/xcl2/xcl2.cpp

SRCS_L3_NAMES = \
	op_similaritydense.cpp \
	xf_graph_L3.cpp \
	xf_graph_L3_handle.cpp

SRCS_L3 = $(addprefix $(GRAPH_ANALYTICS_DIR)/L3/src/,$(SRCS_L3_NAMES)) $(GRAPH_ANALYTICS_DIR)/ext/xcl2/xcl2.cpp
OBJS_L3 = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_L3:.cpp=.o)))

SRCS_loader = src/cosinesim_loader.cpp
OBJS_loader = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_loader:.cpp=.o)))
DEPS_loader = $(OBJS_loader:.o=.d)

SRCS_top = $(filter-out $(SRCS_loader),$(wildcard src/*.cpp))
OBJS_top = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_top:.cpp=.o)))

SRCS_all = $(SRCS_L3) $(SRCS_top) $(SRCS_loader)
OBJS_all = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_all:.cpp=.o)))
DEPS_all = $(OBJS_all:.o=.d)

OBJS_libCosineSim = $(OBJS_L3) $(OBJS_top) 

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

# SHAREDLIB_FLAGS_libCosineSim = -shared
SHAREDLIB_FLAGS_libCosineSim = -fPIC -rdynamic -shared -w -Wl,--export-dynamic

$(CPP_BUILD_DIR)/$(LIB_NAME): $(OBJS_libCosineSim) $(DEPLIBS_libCosineSim)
	$(LINK.cc) -o $@ $(OBJS_libCosineSim) $(SHAREDLIB_FLAGS_libCosineSim) $(LDLIBS_libCosineSim) $(LIB_DEPS)

# loader .a

$(CPP_BUILD_DIR)/$(LOADER_NAME): $(OBJS_loader) $(DEPLIBS_loader)
	ar ru $@ $(OBJS_loader)
	ranlib $@

# Macro to create a .o rule and a .d rule for each .cpp

define BUILD_CPP_RULE

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o): $(1)
	$(COMPILE.cc) $(INCLUDES_libCosineSim) -o $$@ $$<

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).d): $(1)
	@set -e; \
	rm -f $$@; \
	$(COMPILE.cc) -MM -MT $(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o) $(INCLUDES_libCosineSim) -MF $$@.$$$$$$$$ $$<; \
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

$(CPP_DOXY_TRIGGER_FILE): include/cosinesim.hpp $(CPP_DOXY_CONFIG_FILE)
	@set -e; \
	if ! [ -x "$$(command -v doxygen)" ]; then \
	    echo "ERROR: doxygen appears not to be installed.  Please install at least version 1.8.5"; \
	    false; \
	fi; \
	mkdir -p $(CPP_DOXY_TARGET_DIR); \
	sed 's,COSINESIM_VERSION,$(strip $(shell cat VERSION)),g' < $(CPP_DOXY_CONFIG_FILE) > $(CPP_DOXY_TARGET_DIR)/doxyfile.tmp; \
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
PYTHON_LIB_NAME := xilCosineSim.so #$(shell python-config --extension-suffix)

SRC_FILE_NAMES_python = pythonapi.cpp

INCLUDES_python = \
    -I$(PYTHON_DIR) \
	-I$(GRAPH_ANALYTICS_DIR)/ext \
    -Iinclude

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
	$(PYTHONENV) -m pip install pybind11
#	$(PYTHONENV) -m pip install python-config

pythonApi2: $(CPP_BUILD_DIR) $(CPP_BUILD_DIR)/$(PYTHON_LIB_NAME) examples/python/run.sh

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
	-I$(XILINX_XRT)/include \
	-I$(XILINX_XRM)/include \
	-I$(GRAPH_ANALYTICS_DIR)/L2/include \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include \
	-I$(GRAPH_ANALYTICS_DIR)/ext \
	-I$(GRAPH_ANALYTICS_DIR)/ext/xcl2 \
	-Iinclude

LDLIBS_test = \
	

SRC_FILE_NAMES_test = \
    cosinesim_test.cpp 

SRCS_test = $(addprefix $(TEST_DIR)/,$(SRC_FILE_NAMES_test))
OBJS_test = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_test:.cpp=.o)))
DEPS_test = $(OBJS_test:.o=.d)

# List of all test executables to build
EXEC_FILE_NAMES_test = \
    cosinesim_test \
    cosinesim_dyn_test 

EXECS_test = $(addprefix $(CPP_BUILD_DIR)/,$(EXEC_FILE_NAMES_test))

.PHONY: cppTest cppTest2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
cppTest: cppBuild
	@make cppTest2

cppTest2: $(EXECS_test)

# Test executables

LDFLAGS_test = -fPIC -w -L $(CPP_BUILD_DIR) -l$(LIB_SHORT_NAME)

$(CPP_BUILD_DIR)/cosinesim_test: $(CPP_BUILD_DIR)/cosinesim_test.o $(CPP_BUILD_DIR)/$(LIB_NAME)
	$(LINK.cc) -o $@ $< $(LDFLAGS_test) $(LIB_DEPS)

$(CPP_BUILD_DIR)/cosinesim_dyn_test: $(CPP_BUILD_DIR)/cosinesim_test.o $(CPP_BUILD_DIR)/$(LOADER_NAME) $(CPP_BUILD_DIR)/$(LIB_NAME)
	$(LINK.cc) -o $@ $< -fPIC -w -L $(CPP_BUILD_DIR) -l$(LOADER_SHORT_NAME) -ldl $(LIB_DEPS)

numDevices=1
iterations=1
numVectors=5000
deviceNames=xilinx_u50_gen3x16_xdma_201920_3
run-test: cppTest
	set -e; \
	. $(XILINX_XRT)/setup.sh && \
	. $(XILINX_XRM)/setup.sh && \
	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$$LD_LIBRARY_PATH $(CPP_BUILD_DIR)/cosinesim_test -d $(numDevices) -i $(iterations) -t $(deviceNames)

run-dyn-test: cppTest
	LD_PRELOAD=$(PWD)/$(CPP_BUILD_DIR)/$(LIB_NAME) LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$(LD_LIBRARY_PATH) \
	    $(CPP_BUILD_DIR)/cosinesim_dyn_test $(numDevices)

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


L2_TOP_DIR = $(GRAPH_ANALYTICS_DIR)/L2/dense_similarity_multi_kernel
L2_XDEVICE_U50 = xilinx_u50_gen3x16_xdma_201920_3
L2_BUILD_DIR_U50 = $(L2_TOP_DIR)/build_dir.hw.$(L2_XDEVICE_U50)
XCLBIN_NAME = denseSimilarityKernelWithAlias.xclbin
XCLBIN_FILE_U50 = $(L2_BUILD_DIR_U50)/$(XCLBIN_NAME)
XCLBIN_NAME_U50 = cosinesim_32bit_$(L2_XDEVICE_U50).xclbin
XCLBIN_INSTALLED_U50 = /opt/xilinx/apps/graphanalytics/cosinesim/xclbin/$(XCLBIN_NAME_U50)

L2_XDEVICE_U55C = xilinx_u55c_gen3x16_xdma_2_202110_1
L2_BUILD_DIR_U55C = $(L2_TOP_DIR_4PU)/build_dir.hw.$(L2_XDEVICE_U55C)
L2_TOP_DIR_4PU = $(GRAPH_ANALYTICS_DIR)/L2/dense_similarity_multi_kernel_4pu
XCLBIN_NAME_4PU = denseSimilarityKernel4PUWithAlias.xclbin
XCLBIN_NAME_U55C = cosinesim_32bit_4pu_xilinx_u55c_gen3x16_xdma_base_2.xclbin
XCLBIN_FILE_U55C = $(L2_BUILD_DIR_U55C)/$(XCLBIN_NAME_4PU)
XCLBIN_INSTALLED_U55C = /opt/xilinx/apps/graphanalytics/cosinesim/xclbin/$(XCLBIN_NAME_U55C)

DEVICE ?= $(L2_XDEVICE)
ifeq ($(findstring u55, $(DEVICE)), u55)
	XCLBIN_BUILD_DIR=$(L2_TOP_DIR_4PU)
else
	XCLBIN_BUILD_DIR=$(L2_TOP_DIR)
endif

.PHONY: xclbin
xclbin:
	@if [ "$(PLATFORM_REPO_PATHS)" == "" ]; then \
		echo "PLATFORM_REPO_PATHS environment varaible must be set to the path to Xilinx platform files" ; \
	else \
		cd $(XCLBIN_BUILD_DIR); \
		make build TARGET=hw DEVICE=$(DEVICE); \
	fi
	
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

$(XCLBIN_FILE_U55C):
	    @if [ -r $(XCLBIN_INSTALLED_U55C) ]; then \
            echo "INFO: The XCLBIN file $(XCLBIN_FILE_U55C) is missing.  Copying from installed area."; \
            cp -r $(XCLBIN_INSTALLED_U55C) $@; \
        else \
            echo "ERROR: The XCLBIN file $(XCLBIN_FILE_U55C) is missing."; \
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
    include/cosinesim.hpp \
    src/cosinesim_loader.cpp

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
    python/pythondemo.py python/run.sh \
    python/README.md python/jupyter/jupyter_demo.ipynb python/jupyter/wikipedia_demo.ipynb

STAGE_EXAMPLE_FILES = $(addprefix examples/,$(STAGE_EXAMPLE_FILE_NAMES))
STAGE_STAGED_EXAMPLE_FILES = $(addprefix $(STAGE_DIR)/,$(STAGE_EXAMPLE_FILES))

.PHONY: stage fill-example-stage stage-examples

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
	
$(STAGE_STAGED_EXAMPLE_FILES):
	cp -f $(patsubst $(STAGE_DIR)/%,%,$@) $@
	

# XCLBIN file

STAGE_XCLBIN_NAME_U50 = $(XCLBIN_NAME_U50)
STAGE_XCLBIN_FILE_U50 = $(STAGE_DIR)/xclbin/$(STAGE_XCLBIN_NAME_U50)

$(STAGE_XCLBIN_FILE_U50): $(XCLBIN_FILE_U50)
	cp -f $^ $@


STAGE_XCLBIN_NAME_U55C = $(XCLBIN_NAME_U55C)
STAGE_XCLBIN_FILE_U55C = $(STAGE_DIR)/xclbin/$(STAGE_XCLBIN_NAME_U55C)

$(STAGE_XCLBIN_FILE_U55C): $(XCLBIN_FILE_U55C)
	cp -f $^ $@

# Top-level packaging rule

STAGE_SUBDIR_NAMES = include lib src xclbin
STAGE_SUBDIRS = $(addprefix $(STAGE_DIR)/,$(STAGE_SUBDIR_NAMES))
STAGE_ALL_FILES = \
    $(addprefix $(STAGE_DIR)/,$(STAGE_COPY_FILES)) \
    $(addprefix $(STAGE_DIR)/lib/,$(STAGE_LIB_FILES)) \
    $(STAGE_XCLBIN_FILE_U50) \
    $(STAGE_XCLBIN_FILE_U55C) 
#    $(STAGE_DIR)/$(STAGE_EXAMPLE_ZIP) \

.PHONY: stage stage2
stage: pythonApi
	make stage2

stage2: cppTest $(STAGE_SUBDIRS) stage-examples $(STAGE_ALL_FILES)

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

dist: update-version stage
	@cd package; \
	if [ "$(DIST_TARGET)" == "" ]; then \
	    echo "Packaging is supported for only Ubuntu and CentOS."; \
	else \
	    echo "Packaging $(DIST_TARGET) for $(OSDIST)"; \
	    make ; \
	fi

examples/python/run.sh: VERSION
	# update PRODUCT_VER in python run.sh
	sed -i 's/PRODUCT_VER=.*/PRODUCT_VER=$(PRODUCT_VER)/g' examples/python/run.sh
	sed -i 's,/cosinesim/1.2/xclbin,/cosinesim/$(PRODUCT_VER)/xclbin,g' src/cosinesim.cpp

update-version : examples/python/run.sh

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
.PHONY: clean clean-xclbin clean-pyenv

clean:
	rm -rf Debug Release $(JAVA_LIB_DIR) $(BUILD_DIR) $(STAGE_DIR)
	@echo "Note: because the XCLBIN file takes so long to generate, it is not removed with this 'clean' target."
	@echo "To clean the XCLBIN, use 'make clean-xclbin'"
	cd package; make clean
	
clean-xclbin:
	rm -f $(XCLBIN_FILE_U50) $(XCLBIN_FILE_U55C)

clean-pyenv:
	rm -rf $(PYTHONENV_NAME)

.PHONY: help
help:
	@echo "Makefile Usage:"
	@echo "  make"
	@echo "      Build staging files including the library for PythonAPI"
	@echo ""
	@echo "  make dist"
	@echo "      Generate cosinesim product package (RPM or DEB) for the current OS and architecture"
	@echo ""
	@echo "  make doxy-cpp "
	@echo "      Generate CPP documentation using doxygen. The default html output directory is ./build/cppdocs"
	@echo ""
	@echo "  make run-test [numDevices=1] [iterations=1] [numVectors=5000] [deviceNames=xilinx_u50_gen3x16_xdma_201920_3]"
	@echo "      Run standalone test with statically linked executable. numDevices, numVectors, and iterations are optional."
	@echo "      Supported deviceNames: "
	@echo "      xilinx_u50_gen3x16_xdma_201920_3"
	@echo "      xilinx_u55c_gen3x16_xdma_base_2"
	@echo ""
	@echo "  make xclbin DEVICE=<FPGA platform>"
	@echo "      Build xclbin file targetting specified device. Vitis tool and PLATFORM_REPO_PATHS must be set to make xclbin."
	@echo "      Supported devices:"
	@echo "      xilinx_u50_gen3x16_xdma_201920_3 (Vitis 2020.2)"
	@echo "      xilinx_u55c_gen3x16_xdma_2_202110_1 (Vitis 2021.2)"
	@echo ""
