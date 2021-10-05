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

PLUGIN_DEPS = ../../udf/xilinxRecomEngine.hpp \
			  ../../udf/xilinxRecomEngineImpl.hpp \
			  ../../bin/set-plugin-vars.sh \
			  ../../../common/bin/install-plugin-node.sh

help-common:
	@echo "Makefile usage:"
	@echo "make install-plugin [deviceNames=deviceNames] [sshKey=ssh-key-file]"
	@echo "    Supported deviceNames:"
	@echo "    xilinx_u50_gen3x16_xdma_201920_3"
	@echo "    xilinx_u55c_gen3x16_xdma_base_2"

	@echo "Parameter descriptions:"
	@echo ""
	@echo "sshKey            : SSH key for user tigergraph"	


.install-plugin-done: $(PLUGIN_DEPS)
	cd ../../../../../cosinesim && make DEBUG=$(DEBUG)
	@echo "-------------------------------------------------------"
	@echo "Installing plugin files into TigerGraph software"
	@echo "-------------------------------------------------------"
	cd ../../ && \
	    make DEBUG=$(DEBUG) stage && \
	    ./staging/install.sh -v -f $(SSH_KEY_OPT) -d $(deviceNames) && \
	cd - && touch .install-plugin-done

.install-udf-done: .install-plugin-done
	@echo "--------------------------------------------------------------"
	@echo "Installing application specific UDFs into TigerGraph software"
	@echo "--------------------------------------------------------------"
	cd ../../ && make DEBUG=$(DEBUG) stage && \
	./staging/examples/synthea/bin/install-udf.sh -v -f $(SSH_KEY_OPT) && \
	cd - && touch .install-udf-done
	
install-plugin: .install-plugin-done

clean:
	cd ../../../../../cosinesim && make clean
	cd ../../ && make clean
	rm -f .install-plugin-done .install-udf-done .install-query-done