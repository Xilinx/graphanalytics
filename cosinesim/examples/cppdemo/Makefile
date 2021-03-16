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

MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))
XF_PROJ_ROOT ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%/examples/cppdemo/Makefile}')

LIB_PATH := $(XF_PROJ_ROOT)/staging/lib

#$(info PROJ_ROOT="$(XF_PROJ_ROOT)")

CXXFLAGS = -O0 -g -std=c++11  -fPIC -pthread -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-label -Wno-unused-variable -Wno-unused-parameter -Wno-missing-field-initializers -Wno-deprecated-declarations

all: cosinesim_test
cosinesim_test: cosinesim_test.cpp
	g++ $(CXXFLAGS) $^ -o $@ \
		-I$(XILINX_XRT)/include -I$(XILINX_XRM)/include -I$(XF_PROJ_ROOT)/include \
		-L$(LIB_PATH) -lXilinxCosineSim -lxrm\
		-L$(XILINX_XRT)/lib -L$(XILINX_XRM)/lib -lOpenCL -lpthread -m64

.PHONY: run clean


clean:
	rm -f cosinesim_test *txt *log
