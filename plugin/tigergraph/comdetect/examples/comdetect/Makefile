# Copyright 2020 Xilinx, Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

deviceNames=xilinx_u50_gen3x16_xdma_201920_3
dataSource=$(CURDIR)/data/as-Skitter-wt-r100.mtx
graphName=social_$(USER)
alveoProject=

numPartitionsNode=9

compileMode=1
partitionMode=3
runMode=2
genOutputFile=0

ifdef sshKey
    SSH_KEY_OPT=-i $(sshKey)
endif	

PLUGIN_DEPS = ../../udf/xilinxComDetect.hpp \
              ../../udf/xilinxComDetectImpl.hpp \
			  ../../../../../louvainmod/include/xilinxlouvain.h \
			  ../../../../../louvainmod/src/ParLV.cpp \
			  ../../../../../louvainmod/src/xilinxlouvain.cpp

TG_DATA_ROOT = $(shell cat ~tigergraph/.tg.cfg | jq .System.DataRoot | tr -d \")
#PLUGIN_TARGET = $(TG_DATA_ROOT)/gsql/udf/xilinx-plugin-config.json
PLUGIN_TARGET = .install-plugin-done

all: run

$(PLUGIN_TARGET): $(PLUGIN_DEPS)
	cd ../../../../../louvainmod && make DEBUG=$(DEBUG)
	@echo "-------------------------------------------------------"
	@echo "Installing plugin files into TigerGraph $(TG_DATA_ROOT)"
	@echo "-------------------------------------------------------"
	cd ../../ && \
	    make DEBUG=$(DEBUG) stage && \
		./staging/install.sh -v -f $(SSH_KEY_OPT) -d $(deviceNames) && \
	cd - && touch .install-plugin-done

.install-udf-done: .install-plugin-done udf/louvainDemo.hpp
	@echo "--------------------------------------------------------------"
	@echo "Installing application specific UDFs into TigerGraph software"
	@echo "--------------------------------------------------------------"
	cd ../../ && make DEBUG=$(DEBUG) stage && \
	./staging/examples/comdetect/bin/install-udf.sh -v -f $(SSH_KEY_OPT) && \
	cd - && touch .install-udf-done

.install-query-done: .install-udf-done query/test.gsql 
	@echo "-------------------------------------------------------"
	@echo "make install-query-done"
	@echo "-------------------------------------------------------"
	@gsql query/test.gsql && touch .install-query-done

run-test: .install-query-done
	@echo "-------------------------------------------------------"
	@echo "make run-test"
	@echo "-------------------------------------------------------"
	gsql -g $(graphName) "run query insert_dummy_nodes(1)"
	time gsql -g $(graphName) "run query test()"

install-plugin: .install-plugin-done

.PHONY: run
run: .install-udf-done
	@echo "-------------------------------------------------------"
	@echo "Running run.sh from staging "
	@echo "-------------------------------------------------------"
ifeq ($(alveoProject),)
	$(error "ERROR: alveoProject must be set to a pathname accessiable by all nodes")
endif	
	cd ../../ && make DEBUG=$(DEBUG) stage && \
	    ./staging/examples/comdetect/bin/run.sh -v \
	    -g $(graphName) -s $(dataSource) -a $(alveoProject) \
		-n $(numPartitionsNode) -c $(compileMode) -x $(partitionMode) -r $(runMode) -o $(genOutputFile)

.PHONY: partition
partition: .install-udf-done
	@echo "-------------------------------------------------------"
	@echo "Running partition.sh from staging "
	@echo "-------------------------------------------------------"
	cd ../../ && make DEBUG=$(DEBUG) stage && ./staging/examples/comdetect/partition.sh -v

.PHONY: reset
reset:
	@echo "Resetting all Alveo cards in all machines in cluster"
	../../../utils/resetc.sh

help:
	@echo "Makefile usage:"
	@echo "make install-plugin [deviceNames=deviceNames]"
	@echo "    Install the plugin for the selected target device"
	@echo ""
	@echo "make run alveoProject=alveo-project-name [deviceNames=deviceNames]"
	@echo "    Run the example"
	@echo ""
	@echo "make partition"
	@echo "    Partition the graph in TigerGraph memory"
	@echo ""
	@echo "make reset"
	@echo "    Resets all the Alveo cards in the xsj-dxgradb cluster:"
	@echo ""
	@echo "Parameter descriptions:"
	@echo "alveoProject      : Required. Partition files path / partition file name prefix"
	@echo "deviceNames       : Target device name. Supported devices: "
	@echo "                    xilinx_u50_gen3x16_xdma_201920_3 (default)"
	@echo "                    xilinx_u55c_gen3x16_xdma_base_2"
	@echo "dataSource        : A .mtx file containing input graph. default=./data/as-Skitter-wt-r100.mtx"
	@echo "graphName         : graph name (default=social_<username>"
	@echo "numPartitionsNode : Number of Alveo partitions per node"
	@echo ""
	@echo "compileMode       : 0: skip database creation and gsql compilation"
	@echo "                    1: recreate database and compile all (default)"
	@echo "                    2: only compile query gsql"
	@echo "partitionMode     : 0: Use existing partitions from disks; "
	@echo "                    1: Generate partitions from TigerGraph memory"
	@echo "                    2: Generate partitions from .mtx file"
	@echo "                    3: Generate partitions from TigerGraph using whole-graph technique (default)"
	@echo "                    4: Generate partitions from TG's vertex IDs and low-bandwidth partitioner"
	@echo "runMode           : 0: Skip both CPU and Alveo run (i.e. only run partition)"
	@echo "                    1: Run only on CPU"
	@echo "                    2: Run only on Alveo (default)"
	@echo "                    3: Run on both CPU and Alveo"
	@echo ""
	@echo "sshKey            : SSH key for user tigergraph"	
	@echo "genOutputFile     : 0: not generate output file (default)"
	@echo "                  : 1: generate output file"
	@echo ""
	@echo "Examples:"
	@echo "make dataSource=/proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx"

clean:
	cd ../../../../../louvainmod && make clean
	rm -f .install-plugin-done .install-udf-done .install-query-done
