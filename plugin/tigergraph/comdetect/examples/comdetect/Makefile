./install-plugin-done: 
	cd ../../ && \
	./install.sh && \
	touch install-done

./install-query-done: query/base.gsql
	./install_query.sh && touch install-query-done
	
run-test: ../../install-plugin-done ./install-query-done
	gsql query/test.gsql



