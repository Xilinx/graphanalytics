../../../../cosinesim/staging/lib/libXilinxCosineSim.so: ../../../../cosinesim/src/cosinesim.cpp
	cd ../../../../cosinesim && \
	make DEBUG=1 all 

../../install-plugin-done: ../../udf/xilinxRecomEngine.hpp ../../udf/xilinxRecomEngineImpl.hpp ../../../../cosinesim/staging/lib/libXilinxCosineSim.so
	cd ../../ && \
	./install.sh && \
	touch install-done

./install-query-done: query/base.gsql
	./install_query.sh && touch install-query-done
	
run-test: ../../install-plugin-done ./install-query-done
	gsql query/test.gsql



