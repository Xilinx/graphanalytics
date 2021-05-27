# partitions=9, devices=1, workers=0 (node=1), levels=100 (all)
run-n1d1:
	cd .. && make DEBUG=$(DEBUG)
	DEBUG=$(DEBUG) ./run_driver.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9  1  0 100

.PHONY: run-driver
run-driver:
# partitions=9, devices=3, workers=2 (nodes=3)
	cd .. && make DEBUG=$(DEBUG)
	DEBUG=$(DEBUG) ./run_driver.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9 3 2 100

.PHONY: run-worker1
run-worker1:
	cd .. && make DEBUG=$(DEBUG)
	DEBUG=$(DEBUG) ./run_worker.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9 100 1

.PHONY: run-worker2
run-worker2:
	cd .. && make DEBUG=$(DEBUG)
	DEBUG=$(DEBUG) ./run_worker.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9 100 2

.PHONY: run-driver
create-partitions:
	cd ../ && make DEBUG=$(DEBUG)
	rm -rf as-skitter-par9
	DEBUG=$(DEBUG) ./create_partitions.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9

help:
	@echo "Make targets:"
	@echo "create-partitions: Create partitions"
	@echo "run-n1d1         : Run test with 1 node and 1 device/node"
	@echo ""
	@echo "Run on cluster with 3 nodes"
	@echo "On node 0: make run-driver"
	@echo "On node 1: make run-worker1"
	@echo "On node 2: make run-worker2"

	

