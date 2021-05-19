SHELL = bash

../../../../../louvainmod/staging/lib/libXilinxLouvain.so:
	cd ../../../../../louvainmod && make DEBUG=1
	
./install-plugin-done: ../../udf/xilinxComDetect.hpp ../../udf/xilinxComDetectImpl.hpp ../../../../../louvainmod/staging/lib/libXilinxLouvain.so
	@echo "-------------------------------------------------------"
	@echo "make ./install-plugin-done"
	@echo "-------------------------------------------------------"
	cd ../../ && make && ./staging/install.sh -v -f && \
	cd - && touch install-plugin-done

./install-udf-done: ./install-plugin-done
	@echo "-------------------------------------------------------"
	@echo "make ./install-udf-done"
	@echo "-------------------------------------------------------"
	cd ../../ && make && \
	./staging/examples/comdetect/bin/install-udf.sh -v -f && \
	cd - && touch install-udf-done

./install-query-done: ./install-udf-done query/test.gsql 
	@echo "-------------------------------------------------------"
	@echo "make install-query-done"
	@echo "-------------------------------------------------------"
	@gsql query/test.gsql && touch install-query-done

run-test: ./install-query-done
	@echo "-------------------------------------------------------"
	@echo "make run-test"
	@echo "-------------------------------------------------------"
	gsql -g social_ywu "run query insert_dummy_nodes(1)"
	time gsql -g social_ywu "run query louvain_alveo(10, [\"Person\"], [\"Coworker\"], \
                                                 \"/home2/tigergraph/alveo_out.txt\", \
                                                 \"/proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx\", \
                                                 \"/proj/gdba/datasets/louvain-graphs/as-skitter-par9\", \
												 \"3\", \"9\", \"0\")"


clean:
	rm -f ./install-plugin-done ./install-udf-done ./install-query-done



