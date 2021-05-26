# node=1 device=1
test-n1d1:
	DEBUG=$(DEBUG) ./run_driver.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9  1   0

.PHONY: test-driver
test-driver:
	cd .. && make DEBUG=1
	DEBUG=$(DEBUG) ./run_driver.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9 3 2

.PHONY: test-worker1
test-worker1:
	cd .. && make DEBUG=1
	DEBUG=$(DEBUG) ./run_worker.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9 1

.PHONY: test-worker2
test-worker2:
	cd .. && make DEBUG=1
	DEBUG=$(DEBUG) ./run_worker.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9 2

.PHONY: test-driver
test-create-partitions:
	cd ../ && make DEBUG=$(DEBUG)
	rm -rf as-skitter-par9
	DEBUG=$(DEBUG) ./create_partitions.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9