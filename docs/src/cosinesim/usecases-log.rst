Error Log Analysis
==================

This example uses TigerGraph database to represent log messages within their contexts 
and finds similar trouble Logs messages for a given query message. 

This example selects a random vertex in the graph from a query and returns the top 
matching Logs based on cosine similarity. Solutions from the matching logs can help 
resolve current issues.

Instead of finding similarity with the direct one-hot word representation, we used 
GloVe Word Embeddings, which maps words into more meaningful space.

In General, finding Cosine Similarity on large dataset will take a huge amount of time 
on CPU. With the Xilinx Cosine Similarity Acceleration, it can speed up the process 
by multiple orders.

This use case can be downloaded from `Error Log Analysis Jupyter notebook on GitHub 
<https://github.com/Xilinx/graphanalytics/blob/master/plugin/tigergraph/recomengine/examples/log_similarity/jupyter-demo/log_similarity_TG_demo.ipynb>`_. 
