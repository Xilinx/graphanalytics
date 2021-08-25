Healthcare Conconcept Similarity with UMLS
===========================================

In this use case, we will try to find similar drugs/healthcare terms from the `Unified 
Medical Language System (UMLS) <https://www.nlm.nih.gov/research/umls/index.html>`_ 
dataset. UMLS is a collection of health and biomedical vocabularies from a wide 
variety of Healthcare data sources. One of the knowledge source in UMLS is a 
metathesaurus that is a collection of medical concepts called atoms and links them 
through useful relationships.

For the purpose of this demo, we use a small subset of the atoms file MRCONSO.RRF 
and use relationships between atoms from the relationships file MRREL.RRF. The 
atoms and their relations are modeled as a graph database using TigerGraph database 
and Cosine Similarity is used as the match similarity measure. 

Each atom is converted into an embedding representation by probabilistically capturing 
their relations. We use the Node2Vec algorithm to compute embeddings for each atom 
vertex in the graph.

This Example selects a random vertex in the graph from a query of UMLS database and 
returns top matching drugs, drug ingredients or related healthcare concepts.

In General, finding Cosine Similarity on large datasets take a large amount of time 
on CPU. With the Xilinx Cosine Similarity Acceleration, speedups > ~ 80x can be achieved.

This   can be downloaded from `Healthcare Conconcept Similarity with UMLS notebook on GitHub 
<https://github.com/Xilinx/graphanalytics/blob/master/plugin/tigergraph/recomengine/examples/drug_similarity/jupyter-demo/drug_similarity_TG_demo.ipynb>`_. 
