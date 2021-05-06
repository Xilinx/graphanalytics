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

EXAMPLE_DIR = examples/louvainmod

# Files to be direct-copied from plugin source tree to staging area

STAGE_COPY_FILES = \
    VERSION \
    \
    bin/set-plugin-vars.sh \
    \
    udf/xilinxComDetect.hpp \
    udf/xilinxComDetectImpl.hpp \
    \
    $(wildcard $(EXAMPLE_DIR)/as-skitter/*) \
    $(EXAMPLE_DIR)/bin/common.sh \
    $(EXAMPLE_DIR)/bin/louvain-py-requirements.txt \
    $(EXAMPLE_DIR)/bin/louvain.py \
    $(EXAMPLE_DIR)/bin/set-plugin-vars.sh \
    $(EXAMPLE_DIR)/udf/louvainDemo.hpp \
    $(EXAMPLE_DIR)/udf/louvainDemoImpl.hpp \
    $(EXAMPLE_DIR)/load.gsql \
    $(EXAMPLE_DIR)/louvain_alveo.gsql \
    $(EXAMPLE_DIR)/louvain_cpu.gsql \
    $(EXAMPLE_DIR)/louvain_distributed_cpu.gsql \
    $(EXAMPLE_DIR)/schema.gsql \
    $(EXAMPLE_DIR)/run.sh

# Example directories into which to copy example common files
STAGE_EXAMPLES_DIRS = $(EXAMPLE_DIR)


include ../common/Makefile-common.mk

