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

# Product version/name
PRODUCT_VER=$(strip $(shell cat VERSION))
STANDALONE_NAME = fuzzymatch

SWIFTAML_DIR = examples/swiftaml
ENTRESOLUTION_DIR = examples/entity-resolution

# Files to be direct-copied from plugin source tree to staging area

STAGE_COPY_FILES = \
    VERSION \
    \
    bin/set-plugin-vars.sh \
    \
    udf/xilinxFuzzyMatch.hpp \
    udf/xilinxFuzzyMatchImpl.hpp \
    \
    $(wildcard $(SWIFTAML_DIR)/data/*) \
	$(SWIFTAML_DIR)/README.md \
    $(SWIFTAML_DIR)/bin/common.sh \
    $(SWIFTAML_DIR)/bin/set-plugin-vars.sh \
    $(SWIFTAML_DIR)/bin/run.sh \
	$(SWIFTAML_DIR)/udf/swiftAmlDemo.hpp \
    $(SWIFTAML_DIR)/udf/swiftAmlDemoImpl.hpp \
	$(SWIFTAML_DIR)/query/base.gsql \
    $(SWIFTAML_DIR)/query/load.gsql \
    $(SWIFTAML_DIR)/query/fuzzymatch.gsql \
    $(SWIFTAML_DIR)/query/schema.gsql \
    \
    $(wildcard $(ENTRESOLUTION_DIR)/data/*) \
	$(ENTRESOLUTION_DIR)/README.md \
    $(ENTRESOLUTION_DIR)/bin/common.sh \
    $(ENTRESOLUTION_DIR)/bin/set-plugin-vars.sh \
    $(ENTRESOLUTION_DIR)/udf/entityResolution.hpp \
    $(ENTRESOLUTION_DIR)/udf/entityResolutionImpl.hpp \
    $(ENTRESOLUTION_DIR)/jupyter-demo/TG_demo.ipynb \
    $(ENTRESOLUTION_DIR)/query/base.gsql \
    $(ENTRESOLUTION_DIR)/query/load.gsql \
    $(ENTRESOLUTION_DIR)/query/entity_resolution.gsql \
    $(ENTRESOLUTION_DIR)/query/schema.gsql \
    $(ENTRESOLUTION_DIR)/bin/run.sh \


# Example directories into which to copy example common files
STAGE_EXAMPLES_DIRS = $(SWIFTAML_DIR) $(ENTRESOLUTION_DIR)

include ../common/Makefile-common.mk

update-version:
	sed -i 's,/graphanalytics/fuzzymatch/.*,/graphanalytics/fuzzymatch/$(PRODUCT_VER),g' bin/set-plugin-vars.sh

stage: update-version stage-common

clean: clean-common

cleanall: cleanall-common

help: help-common


