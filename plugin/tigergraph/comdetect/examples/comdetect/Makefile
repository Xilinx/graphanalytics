XGRAPH=social_$(USER)
ifdef SSH_KEY
    SSH_KEY_OPT=-i $(SSH_KEY)
endif	
compileMode=0
runMode=0
partitionMode=1
numDevices=1
numNodes=1
numPartitions=9
dataSource=/proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx
alveoProject=/proj/gdba/tg-common/alveo-projects/as-skitter-partition

all: run

../../../../../louvainmod/staging/lib/libXilinxLouvain.so: ../../../../../louvainmod/src/louvainmod_loader.cpp \
                                                           ../../../../../louvainmod/src/ParLV.cpp
	cd ../../../../../louvainmod && make DEBUG=$(DEBUG)
	
.install-plugin-done: ../../udf/xilinxComDetect.hpp ../../udf/xilinxComDetectImpl.hpp ../../../../../louvainmod/staging/lib/libXilinxLouvain.so
	@echo "-------------------------------------------------------"
	@echo "Installing plugin files into TigerGraph software"
	@echo "-------------------------------------------------------"
	cd ../../ && make && ./staging/install.sh -v -f $(SSH_KEY_OPT) && \
	cd - && touch .install-plugin-done

.install-udf-done: .install-plugin-done udf/louvainDemo.hpp
	@echo "--------------------------------------------------------------"
	@echo "Installing application specific UDFs into TigerGraph software"
	@echo "--------------------------------------------------------------"
	cd ../../ && make && \
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
	gsql -g $(XGRAPH) "run query insert_dummy_nodes(1)"
	time gsql -g $(XGRAPH) "run query test()"

install-plugin: .install-plugin-done

.PHONY: run
run: .install-udf-done
	@echo "-------------------------------------------------------"
	@echo "Running run.sh from staging "
	@echo "-------------------------------------------------------"
	cd ../../ && make && ./staging/examples/comdetect/run.sh -v $(SSH_KEY_OPT) \
	    -g $(XGRAPH) -c $(compileMode) -r $(runMode) -x $(partitionMode) \
		-m $(numNodes) -d $(numDevices) \
	    -s $(dataSource) -a $(alveoProject) -n $(numPartitions)

help:
	./run.sh -h

clean:
	rm -f .install-plugin-done .install-udf-done .install-query-done




