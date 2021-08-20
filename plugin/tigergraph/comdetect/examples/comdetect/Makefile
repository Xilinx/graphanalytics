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

dataSource=$(CURDIR)/data/as-Skitter-wt-r100.mtx
graphName=social_$(USER)

numNodes=1
numPartitionsNode=9

compileMode=0
partitionMode=1
runMode=1
partitionDumpPath=""

ifdef sshKey
    SSH_KEY_OPT=-i $(sshKey)
endif	

PLUGIN_DEPS = ../../udf/xilinxComDetect.hpp \
              ../../udf/xilinxComDetectImpl.hpp \
			  ../../../../../louvainmod/include/xilinxlouvain.h \
			  ../../../../../louvainmod/src/ParLV.cpp \
			  ../../../../../louvainmod/src/xilinxlouvain.cpp

all: run

.install-plugin-done: $(PLUGIN_DEPS)
	cd ../../../../../louvainmod && make DEBUG=$(DEBUG)
	@echo "-------------------------------------------------------"
	@echo "Installing plugin files into TigerGraph software"
	@echo "-------------------------------------------------------"
	cd ../../ && make DEBUG=$(DEBUG) && ./staging/install.sh -v -f $(SSH_KEY_OPT) && \
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
	cd ../../ && make DEBUG=$(DEBUG) stage && ./staging/examples/comdetect/bin/run.sh -v \
	    -g $(graphName) -s $(dataSource) \
		-m $(numNodes) -n $(numPartitionsNode) \
		-c $(compileMode) -x $(partitionMode) -r $(runMode) -a $(partitionDumpPath)

.PHONY: run-cluster
run-cluster: .install-udf-done
	@echo "-------------------------------------------------------"
	@echo "Running run.sh from staging "
	@echo "-------------------------------------------------------"
	cd ../../ && make DEBUG=$(DEBUG) stage && ./staging/examples/comdetect/bin/run.sh -v \
	    -g $(graphName) -c $(compileMode) -r $(runMode) \
		-m 3 -d 3 -n 9 -s $(dataSource) -a $(partitionDumpPath)

.PHONY: partition
partition: .install-udf-done
	@echo "-------------------------------------------------------"
	@echo "Running partition.sh from staging "
	@echo "-------------------------------------------------------"
	cd ../../ && make DEBUG=$(DEBUG) stage && ./staging/examples/comdetect/partition.sh -v

help:
	@echo "Valid make targets:"
	@echo "run-cluster: Run the example on the cluster of 3 nodes, 3 devices/node, 9 partitions"
	@echo "partition: Run partition.sh to partition the graph in TigerGraph memory"
	@echo ""
	@echo "Optional make parameters"
	@echo "dataSource       : A .mtx file containing input graph. default=./as-Skitter/as-Skitter-wt-r100.mtx"
	@echo "graphName        : graph name (default=social_<username>"
	@echo "numNodes         : Number of nodes in Tigergraph cluster"
	@echo "numPartitionsNode: Number of Alveo partitions per node"
	@echo "partitionDumpPath: partition files path / partition file name prefix"
	@echo ""
	@echo "compileMode      : 0: recreate database and compile all (default)"
	@echo "                   1: only compile query gsql"
	@echo "                   2: skip database creation and gsql compilation"
	@echo "partitionMode    : 0: Use existing partitions from disks; 1: Generate partitions from TigerGraph memory"
	@echo "runMode          : 0: Run only on CPU"
	@echo "                   1: Run only on Alveo (default)"
	@echo "                   2: Run on both CPU and Alveo"
	@echo "                   3: Skip both CPU and Alveo run (i.e. only run partition)"
	@echo ""
	@echo "sshKey           : SSH key for user tigergraph"	
	@echo ""
	@echo "Examples:"
	@echo "make dataSource=/proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx"

clean:
	cd ../../../../../louvainmod && make clean
	rm -f .install-plugin-done .install-udf-done .install-query-done
