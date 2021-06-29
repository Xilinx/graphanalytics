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


graphName=social_$(USER)
ifdef sshKey
    SSH_KEY_OPT=-i $(sshKey)
endif	
compileMode=0
runMode=0
numDevices=1
numNodes=1
numPartitions=9
dataSource=$(CURDIR)/as-skitter/as-Skitter-wt-r100.mtx
alveoProject=$(CURDIR)/as-skitter/as-skitter-par9

all: run

PLUGIN_DEPS = ../../udf/xilinxComDetect.hpp \
              ../../udf/xilinxComDetectImpl.hpp \
			  ../../../../../louvainmod/include/xilinxlouvain.h \
			  ../../../../../louvainmod/src/ParLV.cpp

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
	cd ../../ && make DEBUG=$(DEBUG) stage && ./staging/examples/comdetect/run.sh -v \
	    -g $(graphName) -c $(compileMode) -r $(runMode) \
		-m $(numNodes) -d $(numDevices) \
	    -s $(dataSource) -a $(alveoProject) -n $(numPartitions)

.PHONY: run-cluster
run-cluster: .install-udf-done
	@echo "-------------------------------------------------------"
	@echo "Running run.sh from staging "
	@echo "-------------------------------------------------------"
	cd ../../ && make DEBUG=$(DEBUG) stage && ./staging/examples/comdetect/run.sh -v \
	    -g $(graphName) -c $(compileMode) -r $(runMode) \
		-m 3 -d 3 -n 9 -s $(dataSource) -a $(alveoProject) 

help:
	@echo "Valid make targets:"
	@echo "run-cluster: Run the example on the cluster of 3 nodes, 3 devices/node, 9 partitions"
	@echo ""
	@echo "Optional make parameters"
	@echo "alveoProject     : Alveo partition project basename"
	@echo "compileMode      : 0: recreate database and compile all (default); 1: only compile query gsql; 2: skip database creation and gsql compilation"
	@echo "numDevices       : number of FPGAs needed (default=1)"
	@echo "graphName        : graph name (default=social_<username>"
	@echo "sshKey           : SSH key for user tigergraph"
	@echo "numNodes         : Number of nodes in Tigergraph cluster"
	@echo "numPartitions    : Number of Alveo partitions"
	@echo "runMode          : 0: Run only on CPU (default); 1: Run only on Alveo; 2: Run on both CPU and Alveo"
	@echo "dataSource       : A .mtx file containing input graph. default=./as-Skitter/as-Skitter-wt-e110k.mtx"
	@echo " "
	@echo "Examples:"
	@echo "make alveoProject=/proj/gdba/tg-common/alveo-projects/as-skitter-partition"

clean:
	rm -f .install-plugin-done .install-udf-done .install-query-done




