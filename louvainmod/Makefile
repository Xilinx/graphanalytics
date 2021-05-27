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

ifndef XILINX_XRT
    XILINX_XRT = /opt/xilinx/xrt
    export XILINX_XRT
endif

ifndef XILINX_XRM
    XILINX_XRM=/opt/xilinx/xrm
    export XILINX_XRM
endif

ifndef XILINX_PLATFORMS
    XILINX_PLATFORMS = /opt/xilinx/platforms
    export XILINX_PLATFORMS
endif

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



#######################################################################################################################
#
# C++ API Library
#

CCC = g++
CXX = g++
CXXFLAGS = -std=c++11 -DPRAGMA -Ofast -fopenmp -fmessage-length=0  -fPIC -Wall -Wno-unknown-pragmas -Wno-unused-label -Wno-unused-variable -Wno-sign-compare -Wno-unused-result  \
           -Wno-narrowing -D USE_HBM -fmessage-length=0 -D USE_HBM -D LOUVAINMOD

# Define the target directories.

ifdef DEBUG
CPP_BUILD_DIR = Debug
CXXFLAGS += -O0 -g
else
CPP_BUILD_DIR = Release
CXXFLAGS += -O3 -DNDEBUG
endif


## Target: libLouvain.so
INCLUDES_libLouvain = \
	-Iinclude \
	-I$(GRAPH_ANALYTICS_DIR)/louvainmod/grappolo/include \
	-I$(XILINX_XRT)/include \
	-I$(XILINX_XRM)/include \
	-I$(GRAPH_ANALYTICS_DIR)/L2/include \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include/include_hls \
	-I$(GRAPH_ANALYTICS_DIR)/ext/xcl2 \
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
	sed 's,LOUVAINMOD_VERSION,$(strip $(shell cat VERSION)),g' < $(CPP_DOXY_CONFIG_FILE) > $(CPP_DOXY_TARGET_DIR)/doxyfile.tmp; \
	doxygen $(CPP_DOXY_TARGET_DIR)/doxyfile.tmp; \
	rm -f $(CPP_DOXY_TARGET_DIR)/doxyfile.tmp




#######################################################################################################################
#
# C++ Tests
#

TEST_DIR = tests

INCLUDES_test = \
	-Iinclude \
	-I$(XILINX_XRT)/include \
	-I$(XILINX_XRM)/include \
	-I$(GRAPH_ANALYTICS_DIR)/L2/include \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include \
	-I$(GRAPH_ANALYTICS_DIR)/ext/xcl2 \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include/include_hls \

LDLIBS_test = \
	

SRC_FILE_NAMES_test = \
    louvainmod_test.cpp

SRCS_test = $(addprefix $(TEST_DIR)/,$(SRC_FILE_NAMES_test))
OBJS_test = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_test:.cpp=.o)))
DEPS_test = $(OBJS_test:.o=.d)

# List of all test executables to build
EXEC_FILE_NAMES_test = \
    louvainmod_test

EXECS_test = $(addprefix $(CPP_BUILD_DIR)/,$(EXEC_FILE_NAMES_test))

.PHONY: cppTest cppTest2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
cppTest: cppBuild
	@make cppTest2

cppTest2: $(EXECS_test)

# Test executables

LDFLAGS_test = -fPIC -w -L $(CPP_BUILD_DIR) -l$(LIB_SHORT_NAME) -lzmq -fopenmp -m64

$(CPP_BUILD_DIR)/louvainmod_test: $(CPP_BUILD_DIR)/louvainmod_test.o $(CPP_BUILD_DIR)/$(LIB_NAME)
	$(LINK.cc) -o $@ $< $(LDFLAGS_test) $(LIB_DEPS)

#$(CPP_BUILD_DIR)/cosinesim_dyn_test: $(CPP_BUILD_DIR)/cosinesim_test.o $(CPP_BUILD_DIR)/$(LOADER_NAME) $(CPP_BUILD_DIR)/$(LIB_NAME)
#	$(LINK.cc) -o $@ $< -fPIC -w -L $(CPP_BUILD_DIR) -l$(LOADER_SHORT_NAME) -ldl $(LIB_DEPS)

run-test: cppTest
	LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$(LD_LIBRARAY_PATH) $(CPP_BUILD_DIR)/louvainModularity_test $(NUMDEVICES)

#run-dyn-test: cppTest
#	LD_PRELOAD=$(PWD)/$(CPP_BUILD_DIR)/$(LIB_NAME) LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$(LD_LIBRARAY_PATH) \
#	    $(CPP_BUILD_DIR)/cosinesim_dyn_test $(NUMDEVICES)

#LDFLAGS_fakeudf = -fPIC -w -L /scratch/tigergraph/tigergraph/app/3.1.0/dev/gdk/gsql/src/QueryUdf -lgraphL3wrapper -ldl
#LDFLAGS_fakeudf = -fPIC -w -L $(CPP_BUILD_DIR) -l$(LIB_SHORT_NAME)

#$(CPP_BUILD_DIR)/fakeudf: $(CPP_BUILD_DIR)/fakeudf.o
#	$(LINK.cc) -o $@ $^ $(LDFLAGS_fakeudf) $(LIB_DEPS)

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


L2_TOP_DIR = $(GRAPH_ANALYTICS_DIR)/L2/louvainmod_pruning_kernel
L2_XDEVICE = xilinx_u50_gen3x16_xdma_201920_3
L2_BUILD_DIR = $(L2_TOP_DIR)/build_dir.hw.$(L2_XDEVICE)
XCLBIN_NAME = kernel_louvain_pruning.xclbin
XCLBIN_FILE = $(L2_BUILD_DIR)/$(XCLBIN_NAME)
XCLBIN_INSTALLED_NAME = louvainmod_pruning_$(L2_XDEVICE).xclbin
XCLBIN_INSTALLED =/opt/xilinx/apps/graphanalytics/louvainmod/1.0/xclbin/$(XCLBIN_INSTALLED_NAME)

.PHONY: xclbin

xclbin:
	cd $(L2_TOP_DIR); \
	source $(XILINX_XRT)/setup.sh; \
	export PLATFORM_REPO_PATHS=$(XILINX_PLATFORMS); \
	make build TARGET=hw
	
$(XCLBIN_FILE):
	@if [ -r $(XCLBIN_INSTALLED) ]; then \
	    echo "INFO: The XCLBIN file $(XCLBIN_FILE) is missing.  Copying from installed area."; \
	    cp -r $(XCLBIN_INSTALLED) $@; \
	else \
	    echo "ERROR: The XCLBIN file $(XCLBIN_FILE) is missing."; \
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
#STAGE_EXAMPLE_FILE_NAMES = \
	cpp/cppdemo.cpp cpp/Makefile cpp/README.md \
    python/pythondemo.py python/run.sh \
    python/README.md python/jupyter/jupyter_demo.ipynb

#STAGE_EXAMPLE_FILES = $(addprefix examples/,$(STAGE_EXAMPLE_FILE_NAMES))
#STAGE_STAGED_EXAMPLE_FILES = $(addprefix $(STAGE_DIR)/,$(STAGE_EXAMPLE_FILES))

#.PHONY: stage fill-example-stage
.PHONY: stage

#$(STAGE_DIR)/$(STAGE_EXAMPLE_ZIP): $(STAGE_EXAMPLE_FILES)
#	mkdir -p $(sort $(dir $(STAGE_STAGED_EXAMPLE_FILES)))
#	@make fill-example-stage
#	@cd $(STAGE_DIR) ; \
#	zip -r $(STAGE_EXAMPLE_ZIP) examples
#	rm -rf $(STAGE_DIR)/examples

#fill-example-stage: $(STAGE_STAGED_EXAMPLE_FILES)
	
#$(STAGE_STAGED_EXAMPLE_FILES):
#	cp -f $(patsubst $(STAGE_DIR)/%,%,$@) $@
	

# XCLBIN file

STAGE_XCLBIN_NAME = $(XCLBIN_INSTALLED_NAME)
STAGE_XCLBIN_FILE = $(STAGE_DIR)/xclbin/$(STAGE_XCLBIN_NAME)

$(STAGE_XCLBIN_FILE): $(XCLBIN_FILE)
	cp -f $^ $@

# Top-level packaging rule

STAGE_SUBDIR_NAMES = include lib src xclbin
STAGE_SUBDIRS = $(addprefix $(STAGE_DIR)/,$(STAGE_SUBDIR_NAMES))
STAGE_ALL_FILES = \
    $(addprefix $(STAGE_DIR)/,$(STAGE_COPY_FILES)) \
    $(addprefix $(STAGE_DIR)/lib/,$(STAGE_LIB_FILES)) \
    $(STAGE_XCLBIN_FILE) \
    $(STAGE_DIR)/$(STAGE_EXAMPLE_ZIP) \

stage: cppTest $(STAGE_SUBDIRS) $(STAGE_ALL_FILES)

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

clean:
	rm -rf Debug Release $(JAVA_LIB_DIR) $(BUILD_DIR) $(STAGE_DIR)
	@echo "Note: because the XCLBIN file takes so long to generate, it is not removed with this 'clean' target."
	@echo "To clean the XCLBIN, use 'make clean-xclbin'"
	
clean-xclbin:
	rm -f $(XCLBIN_FILE)

help:
	@echo "Valid make targets:"
	@echo "dist     : Generate product installation package (RPM or DEB) for the current OS and architecture"
	@echo "doxy-cpp : Generate CPP documentation using doxygen. The default html output directory is ./build/cppdocs"
	
