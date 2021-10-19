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
personSource=$(CURDIR)/data/people1k.csv
graphName=swift_$(USER)

compileMode=1
runMode=2

ifdef sshKey
    SSH_KEY_OPT=-i $(sshKey)
endif	

STANDALONE_NAME = fuzzymatch
EXAMPLE=swift_aml
PLUGIN_DEPS = ../../../../../$(STANDALONE_NAME)/src/fuzzy_match.cpp \
              ../../udf/xilinxFuzzyMatch.hpp \
			  ../../udf/xilinxFuzzyMatchImpl.hpp
			  
UDF_DEPS = udf/swiftAmlDemo.hpp udf/swiftAmlDemoImpl.hpp

all: run

.install-plugin-done: $(PLUGIN_DEPS)
	cd ../../../../../$(STANDALONE_NAME) && make DEBUG=$(DEBUG)
	@echo "-------------------------------------------------------"
	@echo "Installing plugin files into TigerGraph software"
	@echo "-------------------------------------------------------"
	cd ../../ && \
	    make DEBUG=$(DEBUG) stage && \
		./staging/install.sh -v -f $(SSH_KEY_OPT) -d $(deviceNames) && \
	cd - && touch .install-plugin-done

.install-udf-done: .install-plugin-done $(UDF_DEPS)
	@echo "--------------------------------------------------------------"
	@echo "Installing application specific UDFs into TigerGraph software"
	@echo "--------------------------------------------------------------"
	cd ../../ && make DEBUG=$(DEBUG) stage && \
	./staging/examples/$(EXAMPLE)/bin/install-udf.sh -v -f $(SSH_KEY_OPT) && \
	cd - && touch .install-udf-done

test: .install-udf-done
	@echo "-------------------------------------------------------"
	@echo "Running quick gsql test"
	@echo "-------------------------------------------------------"
	gsql -g $(graphName) test.gsql
	time gsql -g $(graphName) "run query test()"

install-plugin: .install-plugin-done

.PHONY: run
run: .install-udf-done
	@echo "-------------------------------------------------------"
	@echo "Running run.sh from staging "
	@echo "-------------------------------------------------------"
	cd ../../ && make DEBUG=$(DEBUG) stage && \
	    ./staging/examples/$(EXAMPLE)/bin/run.sh -v \
	    -g $(graphName) -s $(personSource) \
		-c $(compileMode) -r $(runMode) 

help:
	@echo "Makefile usage:"
	@echo "make install-plugin [deviceNames=deviceNames]"
	@echo "    Install the plugin for the selected target device"
	@echo ""
	@echo "make run [deviceNames=deviceNames]"
	@echo "    Run the example"
	@echo ""
	@echo ""
	@echo "Parameter descriptions:"
	@echo "deviceNames       : Target device name. Supported devices: "
	@echo "                    xilinx_u50_gen3x16_xdma_201920_3"
	@echo "personSource      : A csv file with person records. default=../data/people1k.csv"
	@echo "txSource          : A csv file with transactions records. default=../data/txdata.csv"
	@echo "graphName         : graph name (default=social_<username>"
	@echo "compileMode       : 0: skip database creation and gsql compilation"
	@echo "                    1: recreate database and compile all (default)"
	@echo "                    2: only compile query gsql"
	@echo "runMode           : 0: Skip both CPU and Alveo run"
	@echo "                    1: Run only on CPU"
	@echo "                    2: Run only on Alveo (default)"
	@echo "                    3: Run on both CPU and Alveo"
	@echo ""
	@echo "sshKey            : SSH key for user tigergraph"	
	@echo ""

clean:
	cd ../../../../../$(STANDALONE_NAME) && make clean
	rm -f .install-plugin-done .install-udf-done .install-query-done
