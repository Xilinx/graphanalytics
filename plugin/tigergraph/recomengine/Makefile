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

# Example directories
SYNTHEA_DIR = examples/synthea
LOGSIM_DIR = examples/log_similarity
DRUGSIM_DIR = examples/drug_similarity

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
    $(SYNTHEA_DIR)/run.sh \
    \
    $(wildcard $(LOGSIM_DIR)/data/csv/*.csv) \
    $(wildcard $(LOGSIM_DIR)/data/json/*.log) \
    $(LOGSIM_DIR)/bin/common.sh \
    $(LOGSIM_DIR)/bin/set-plugin-vars.sh \
    $(LOGSIM_DIR)/bin/get_glove.sh \
    $(LOGSIM_DIR)/jupyter-demo/log_similarity_TG_demo.ipynb \
    $(LOGSIM_DIR)/query/base.gsql \
    $(LOGSIM_DIR)/query/client.gsql \
    $(LOGSIM_DIR)/query/load_xgraph.gsql \
    $(LOGSIM_DIR)/query/query.gsql \
    $(LOGSIM_DIR)/query/schema_xgraph.gsql \
    $(LOGSIM_DIR)/udf/codevector.hpp \
    $(LOGSIM_DIR)/udf/logSimilarityDemo.hpp \
    $(LOGSIM_DIR)/README.md \
    $(LOGSIM_DIR)/init_graph.sh \
    $(LOGSIM_DIR)/install_query.sh \
    $(LOGSIM_DIR)/match.sh \
    $(LOGSIM_DIR)/run.sh \
    \
    $(wildcard $(DRUGSIM_DIR)/data/RRF/*.RRF) \
    $(wildcard $(DRUGSIM_DIR)/data/csv/*.csv) \
    $(DRUGSIM_DIR)/bin/common.sh \
    $(DRUGSIM_DIR)/bin/set-plugin-vars.sh \
    $(DRUGSIM_DIR)/jupyter-demo/drug_similarity_TG_demo.ipynb \
    $(DRUGSIM_DIR)/query/base.gsql \
    $(DRUGSIM_DIR)/query/client.gsql \
    $(DRUGSIM_DIR)/query/load_xgraph.gsql \
    $(DRUGSIM_DIR)/query/query.gsql \
    $(DRUGSIM_DIR)/query/schema_xgraph.gsql \
    $(DRUGSIM_DIR)/udf/codevector.hpp \
    $(DRUGSIM_DIR)/udf/word2vec.h \
    $(DRUGSIM_DIR)/udf/drugSimilarityDemo.hpp \
    $(DRUGSIM_DIR)/README.md \
    $(DRUGSIM_DIR)/init_graph.sh \
    $(DRUGSIM_DIR)/install_query.sh \
    $(DRUGSIM_DIR)/match.sh \
    $(DRUGSIM_DIR)/run.sh

# Example directories into which to copy example common files
STAGE_EXAMPLES_DIRS = $(SYNTHEA_DIR) $(LOGSIM_DIR) $(DRUGSIM_DIR)

include ../common/Makefile-common.mk

help: help-common
	@echo ""
	@echo "Install plugin files with SSH key file"
	@echo "make sshKey=~/.ssh/tigergraph_rsa"