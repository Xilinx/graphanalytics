

../../install-plugin-done: ../../udf/xilinxRecomEngine.hpp ../../udf/xilinxRecomEngineImpl.hpp
	cd ../../ && \
	./install-plugin-cluster.sh && \
	touch install-plugin-done

./install-query-done: query/base.gsql
	./install_query.sh && touch install-query-done
	
run-test: ../../install-plugin-done ./install-query-done
	gsql query/test.gsql



