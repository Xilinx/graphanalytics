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

SYNTHEA_DIR = examples/synthea

# Files to be direct-copied from plugin source tree to staging area

STAGE_COPY_FILES = \
    VERSION \
    \
    bin/set-plugin-vars.sh \
    bin/install-plugin-cluster-custom.sh \
    \
    udf/xilinxRecomEngine.hpp \
    udf/xilinxRecomEngineImpl.hpp \
    \
    $(wildcard $(SYNTHEA_DIR)/1000_patients/csv/*.csv) \
    $(SYNTHEA_DIR)/bin/common.sh \
    $(SYNTHEA_DIR)/bin/set-plugin-vars.sh \
    $(SYNTHEA_DIR)/jupyter-demo/TG_demo.ipynb \
    $(SYNTHEA_DIR)/python/TG_demo.py \
    $(SYNTHEA_DIR)/query/base.gsql \
    $(SYNTHEA_DIR)/query/client.gsql \
    $(SYNTHEA_DIR)/query/load_xgraph.gsql \
    $(SYNTHEA_DIR)/query/query.gsql \
    $(SYNTHEA_DIR)/query/schema_xgraph.gsql \
    $(SYNTHEA_DIR)/udf/codevector.hpp \
    $(SYNTHEA_DIR)/udf/syntheaDemo.hpp \
    $(SYNTHEA_DIR)/README.md \
    $(SYNTHEA_DIR)/init_graph.sh \
    $(SYNTHEA_DIR)/install_query.sh \
    $(SYNTHEA_DIR)/match.sh \
    $(SYNTHEA_DIR)/run.sh

# Example directories into which to copy example common files
STAGE_EXAMPLES_DIRS = $(SYNTHEA_DIR)

include ../common/Makefile-common.mk
