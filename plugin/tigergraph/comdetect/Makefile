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

# Product version
PRODUCT_VER=$(strip $(shell cat VERSION))

EXAMPLE_DIR = examples/comdetect

# Files to be direct-copied from plugin source tree to staging area

STAGE_COPY_FILES = \
    VERSION \
    \
    bin/set-plugin-vars.sh \
    \
    udf/xilinxComDetect.hpp \
    udf/xilinxComDetectImpl.hpp \
    \
    $(wildcard $(EXAMPLE_DIR)/data/*) \
	$(EXAMPLE_DIR)/README.md \
    $(EXAMPLE_DIR)/bin/common.sh \
    $(EXAMPLE_DIR)/bin/set-plugin-vars.sh \
    $(EXAMPLE_DIR)/udf/louvainDemo.hpp \
    $(EXAMPLE_DIR)/udf/louvainDemoImpl.hpp \
	$(EXAMPLE_DIR)/query/base.gsql \
    $(EXAMPLE_DIR)/query/load.gsql \
    $(EXAMPLE_DIR)/query/louvain_alveo.gsql \
    $(EXAMPLE_DIR)/query/louvain_distributed_q_cpu.gsql \
    $(EXAMPLE_DIR)/query/schema.gsql \
    \
    $(EXAMPLE_DIR)/bin/run.sh \
    $(EXAMPLE_DIR)/bin/compute.sh \
    $(EXAMPLE_DIR)/bin/partition.sh

# Example directories into which to copy example common files
STAGE_EXAMPLES_DIRS = $(EXAMPLE_DIR)

include ../common/Makefile-common.mk

update-version:
	sed -i 's,/graphanalytics/louvainmod/.*,/graphanalytics/louvainmod/$(PRODUCT_VER),g' bin/set-plugin-vars.sh

stage: update-version stage-common

clean: clean-common

cleanall: cleanall-common

help: help-common


