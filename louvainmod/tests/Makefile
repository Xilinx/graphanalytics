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

# partitions=9, devices=1, workers=0 (node=1), levels=100 (all)
graph=../../plugin/tigergraph/comdetect/examples/comdetect/as-skitter/as-Skitter-wt-r100.mtx
alveoProject=../../plugin/tigergraph/comdetect/examples/comdetect/as-skitter/as-skitter-par9
run-n1d1:
	cd .. && make DEBUG=$(DEBUG)
	DEBUG=$(DEBUG) ./run_driver.sh  $(graph) $(alveoProject) 9  1  0 100

.PHONY: run-driver
run-driver:
# partitions=9, devices=3, workers=2 (nodes=3)
	cd .. && make DEBUG=$(DEBUG)
	DEBUG=$(DEBUG) ./run_driver.sh $(graph) $(alveoProject) 9 3 2 100

.PHONY: run-worker1
run-worker1:
	cd .. && make DEBUG=$(DEBUG)
	DEBUG=$(DEBUG) ./run_worker.sh $(graph) $(alveoProject) 9 100 1

.PHONY: run-worker2
run-worker2:
	cd .. && make DEBUG=$(DEBUG)
	DEBUG=$(DEBUG) ./run_worker.sh $(graph) $(alveoProject) 9 100 2

.PHONY: create-partitions
create-partitions:
	cd ../ && make DEBUG=$(DEBUG)
	rm -rf $(alveoProject)
	DEBUG=$(DEBUG) ./create_partitions.sh $(graph) $(alveoProject) 9

clean:
	cd ../ && make clean

help:
	@echo "Make targets:"
	@echo "create-partitions: Create partitions"
	@echo "run-n1d1         : Run test with 1 node and 1 device/node"
	@echo ""
	@echo "Run on cluster with 3 nodes"
	@echo "On node 0: make run-driver"
	@echo "On node 1: make run-worker1"
	@echo "On node 2: make run-worker2"

	

