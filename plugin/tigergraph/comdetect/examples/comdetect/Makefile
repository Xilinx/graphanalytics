SHELL = bash

XGRAPH=social_$(USER)

../../../../../louvainmod/staging/lib/libXilinxLouvain.so: ../../../../../louvainmod/src/louvainmod_loader.cpp \
                                                           ../../../../../louvainmod/src/ParLV.cpp
	cd ../../../../../louvainmod && make DEBUG=1
	
.install-plugin-done: ../../udf/xilinxComDetect.hpp ../../udf/xilinxComDetectImpl.hpp ../../../../../louvainmod/staging/lib/libXilinxLouvain.so
	@echo "-------------------------------------------------------"
	@echo "Installing plugin files into TigerGraph software"
	@echo "-------------------------------------------------------"
	cd ../../ && make && ./staging/install.sh -v -f && \
	cd - && touch .install-plugin-done

.install-udf-done: .install-plugin-done udf/louvainDemo.hpp
	@echo "--------------------------------------------------------------"
	@echo "Installing application specific UDFs into TigerGraph software"
	@echo "--------------------------------------------------------------"
	cd ../../ && make && \
	./staging/examples/comdetect/bin/install-udf.sh -v -f && \
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

run: .install-udf-done
	@echo "-------------------------------------------------------"
	@echo "Running run.sh from staging "
	@echo "-------------------------------------------------------"
	cd ../../ && make && ./staging/examples/comdetect/run.sh -v -g $(XGRAPH) \
	    -s /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx 

clean:
	rm -f .install-plugin-done .install-udf-done .install-query-done




