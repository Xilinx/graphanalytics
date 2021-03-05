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
############################### Setting up Project Variables ##############################

ifndef XILINX_XRT
  XILINX_XRT = /opt/xilinx/xrt
  export XILINX_XRT
endif

XILINX_XRM=/opt/xilinx/xrm


.PHONY: all javaApi javaApiJar javaTest cppBuild cppBuild2

#all: javaApi cppBuild javaTest
all: cppBuild


#
# Java API
#

JAVA_LIB_DIR = lib
JAVA_BUILD_DIR = build
JAVA_SRC_DIR = java/src
JAVA_API_JAR = $(JAVA_LIB_DIR)/xilinxCosineSimApi-0.1.jar
JAVA_API_DOC_JAR = $(JAVA_LIB_DIR)/xilinxCosineSimApi-0.1-javadoc.jar
JAVA_JNI_HEADER = $(JAVA_BUILD_DIR)/api/c-headers/com_xilinx_xf_IntCosineSim.h

javaApi: javaApiJar $(JAVA_API_DOC_JAR)
	
javaApiJar:
	cd java && ant jar
	
$(JAVA_API_DOC_JAR): $(JAVA_API_JAR)
	cd java && ant docs

#
# C++ JNI Library (.so) to be dynamically loaded from the Java API
#

## Location of graphanalytics project
GRAPH_ANALYTICS_DIR = ..

CCC = g++
CXX = g++
CXXFLAGS = -std=c++11 -fPIC -Wall -Wno-unknown-pragmas -Wno-unused-label -D USE_HBM -fmessage-length=0

# Define the target directories.

ifdef DEBUG
CPP_BUILD_DIR = Debug
CXXFLAGS += -O0 -g
else
CPP_BUILD_DIR = Release
CXXFLAGS += -O3
endif


## Target: libCosineSim.so
INCLUDES_libCosineSim = \
	-I$(XRM_DIR)/include \
	-I$(XILINX_XRT)/include \
	-I$(XILINX_XRM)/include \
	-I$(GRAPH_ANALYTICS_DIR)/L2/include \
	-I$(GRAPH_ANALYTICS_DIR)/L3/include \
	-I$(GRAPH_ANALYTICS_DIR)/ext/xcl2 \
	-Iinclude
#	-I${JAVA_HOME}/include \
#	-I${JAVA_HOME}/include/linux

LDLIBS_libCosineSim = \
	
LIB_NAME = libXilinxCosineSim.so
LOADER_NAME = libXilinxCosineSim_loader.a

SRCS_L3 = $(wildcard $(GRAPH_ANALYTICS_DIR)/L3/src/*.cpp)
OBJS_L3 = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_L3:.cpp=.o)))

SRCS_loader = src/cosinesim_loader.cpp
OBJS_loader = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_loader:.cpp=.o)))
DEPS_loader = $(OBJS_loader:.o=.d)

SRCS_top = $(filter-out $(SRCS_loader),$(wildcard src/*.cpp))
OBJS_top = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_top:.cpp=.o)))

SRCS_all = $(SRCS_L3) $(SRCS_top) $(SRCS_loader)

OBJS_libCosineSim = $(OBJS_L3) $(OBJS_top) 
DEPS_libCosineSim = $(OBJS_libCosineSim:.o=.d)

# Make in a child process so that we can isolate the run that includes automatic header dependencies
cppBuild: $(CPP_BUILD_DIR)
	@make cppBuild2

cppBuild2: $(CPP_BUILD_DIR)/$(LIB_NAME) $(CPP_BUILD_DIR)/$(LOADER_NAME)


# Create the target directory (if needed)
$(CPP_BUILD_DIR):
	mkdir -p $(CPP_BUILD_DIR)

# .so


LDFLAGS += -L$(XILINX_XRT)/lib -lOpenCL -lpthread -lrt -Wno-unused-label -Wno-narrowing -DVERBOSE -L$(XILINX_XRM)/lib -lxrm
# SHAREDLIB_FLAGS_libCosineSim = -shared
SHAREDLIB_FLAGS_libCosineSim = -fPIC -rdynamic -shared $(LDFLAGS) -w -Wl,--export-dynamic

$(CPP_BUILD_DIR)/$(LIB_NAME): $(OBJS_libCosineSim) $(DEPLIBS_libCosineSim)
	$(LINK.cc) -o $@ $(OBJS_libCosineSim) $(SHAREDLIB_FLAGS_libCosineSim) $(LDLIBS_libCosineSim)

# loader .a

$(CPP_BUILD_DIR)/$(LOADER_NAME): $(OBJS_loader) $(DEPLIBS_loader)
	ar ru $@ $(OBJS_loader)
	ranlib $@

# Macro to create a .o rule and a .d rule for each .cpp

define BUILD_CPP_RULE
OBJ_FILE := $(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o)
DEP_FILE := $$(OBJ_FILE:.o=.d)

$$(OBJ_FILE): $(1)
	$(COMPILE.cc) $(INCLUDES_libCosineSim) -o $$@ $$<

$$(DEP_FILE): $(1)
	@set -e; \
	rm -f $$@; \
	$(COMPILE.cc) -MM -MT $$(OBJ_FILE) $(INCLUDES_libCosineSim) -MF $$@.$$$$$$$$ $$<; \
        sed 's,\($$*\)\.o[ :]*,\1.o $$@ : ,g' < $$@.$$$$$$$$ > $$@; \
	rm -f $$@.$$$$$$$$

endef

# Expand the macro for each source file

$(foreach src,$(SRCS_all),$(eval $(call BUILD_CPP_RULE,$(src))))

# Include generated header dependency rules, but only if building the JNI .so
# (This restriction prevents problems with the dependency on the Java API for the JNI header.)

ifeq ($(MAKECMDGOALS),cppBuild2)
-include $(DEPS_libCosineSim)
endif

#
# Java Demos
#

javaTest:
	cd test/java && ant

#
# Clean
#

#### Clean target deletes all generated files ####
clean:
	rm -rf Debug Release $(JAVA_LIB_DIR) $(JAVA_BUILD_DIR)

