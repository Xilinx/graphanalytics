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
graphName=travelplan_$(USER)
tp2tr=$(abspath $(CURDIR)/data/travelplan2trucks100.csv)
tp2wo=$(abspath $(CURDIR)/data/travelplan2workorders100.csv)
compileMode=1
runMode=2
INC_ALL_XCLBINS=1

ifdef sshKey
    SSH_KEY_OPT=-i $(sshKey)
endif	

STANDALONE_NAME = mis
EXAMPLE=travelplan
#PLUGIN_DEPS = ../../../../../$(STANDALONE_NAME)/src/fuzzymatch.cpp \
#              ../../../../../$(STANDALONE_NAME)/include/fuzzymatch.hpp \
#              ../../udf/xilinxFuzzyMatch.hpp \
#			  ../../udf/xilinxFuzzyMatchImpl.hpp
PLUGIN_DEPS = ../../udf/xilinxMis.hpp \
			  ../../udf/xilinxMisImpl.hpp
PLUGIN_TARGET = .install-plugin-done
TG_DATA_ROOT = $(shell cat ~tigergraph/.tg.cfg | jq .System.DataRoot | tr -d \")
EXAMPLE_UDF_DEPS = udf/travelPlanDemo.hpp udf/travelPlanDemoImpl.hpp
#EXAMPLE_UDF_TARGET = $(TG_DATA_ROOT)/gsql/udf/travelplanDemo.hpp
EXAMPLE_UDF_TARGET = .install-udf-done

all: run

$(PLUGIN_TARGET): $(PLUGIN_DEPS)
	cd ../../../../../$(STANDALONE_NAME) && make DEBUG=$(DEBUG)
	@echo "-------------------------------------------------------"
	@echo "Installing plugin files into TigerGraph $(TG_DATA_ROOT)"
	@echo "-------------------------------------------------------"
	cd ../../ && \
	    make DEBUG=$(DEBUG) stage INC_ALL_XCLBINS=$(INC_ALL_XCLBINS) deviceNames=$(deviceNames) && \
		./staging/install.sh -v -f $(SSH_KEY_OPT) -d $(deviceNames) 
	touch $(PLUGIN_TARGET)

$(EXAMPLE_UDF_TARGET): $(PLUGIN_TARGET) $(EXAMPLE_UDF_DEPS)
	@echo "--------------------------------------------------------------"
	@echo "Installing application specific UDFs into TigerGraph software"
	@echo "--------------------------------------------------------------"
	cd ../../ && make DEBUG=$(DEBUG) stage && \
	./staging/examples/$(EXAMPLE)/bin/install-udf.sh -v -f $(SSH_KEY_OPT)
	touch $(EXAMPLE_UDF_TARGET)

test: $(EXAMPLE_UDF_TARGET)
	@echo "-------------------------------------------------------"
	@echo "Running quick gsql test"
	@echo "-------------------------------------------------------"
	gsql -g $(graphName) test.gsql
	time gsql -g $(graphName) "run query test()"

install-plugin: $(PLUGIN_TARGET)

.PHONY: run
run: $(EXAMPLE_UDF_TARGET)
	@echo "-------------------------------------------------------"
	@echo "Running run.sh from staging "
	@echo "-------------------------------------------------------"
	cd ../../ && make DEBUG=$(DEBUG) stage && \
	    ./staging/examples/$(EXAMPLE)/bin/run.sh -v \
	    -g $(graphName) -t $(tp2tr) -w $(tp2wo) \
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
	@echo "deviceNames : Target device name. Supported devices: "
	@echo "              xilinx_u50_gen3x16_xdma_201920_3 (default)"
	@echo "graphName   : graph name (default=travelplan_<username>"
	@echo "tp2tr       : CSV file with travel plan to truck mapping"
	@echo "tp2wo       : CSV file with travel plan to work order mapping"
	@echo "compileMode : 0: skip database creation and gsql compilation"
	@echo "              1: recreate database and compile all (default)"
	@echo "              2: only compile query gsql"
	@echo "runMode     : 0: Skip both CPU and Alveo run"
	@echo "              1: Run only on CPU"
	@echo "              2: Run only on Alveo (default)"
	@echo "              3: Run on both CPU and Alveo"
	@echo ""
	@echo "sshKey      : SSH key for user tigergraph"	
	@echo "INC_ALL_XCLBINS : 1: Include all supported XCLBINs for staging"
	@echo "                  0: Include target XCLBIN only"
	@echo ""

clean:
	cd ../../../../../$(STANDALONE_NAME) && make clean
	cd ../../ && make clean
	rm -f $(PLUGIN_TARGET) $(EXAMPLE_UDF_TARGET) .install-query-done
