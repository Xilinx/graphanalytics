# partitions=9, devices=1, workers=0 (node=1), levels=100 (all)
test-n1d1:
	DEBUG=$(DEBUG) ./run_driver.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9  1  0 100

.PHONY: test-driver
test-driver:
# partitions=9, devices=3, workers=2 (nodes=3)
	cd .. && make DEBUG=$(DEBUG)
	DEBUG=$(DEBUG) ./run_driver.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9 3 2 100

.PHONY: test-worker1
test-worker1:
	cd .. && make DEBUG=$(DEBUG)
	DEBUG=$(DEBUG) ./run_worker.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9 100 1

.PHONY: test-worker2
test-worker2:
	cd .. && make DEBUG=$(DEBUG)
	DEBUG=$(DEBUG) ./run_worker.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9 100 2

.PHONY: test-driver
test-create-partitions:
	cd ../ && make DEBUG=$(DEBUG)
	rm -rf as-skitter-par9
	DEBUG=$(DEBUG) ./create_partitions.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9
