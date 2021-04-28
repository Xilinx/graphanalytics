Synthea Demo Query Reference
============================

This section describes the queries supplied with the Synthea demo in greater detail.

Introduction
------------

Operating on the patient graph are a set of GSQL operations and queries for creating graph,
generating the patient vectors, and invoking the
cosine similarity operations of the Recommendation Engine.  The query source files can be found in the ``query``
subdirectory under the demo directory.

The queries are organized by file into levels of operation.  Starting at the lowest level, the files are:

* ``schema_xgraph.gsql``: Defines the graph schema -- the types of vertices and edges

* ``load_xgraph.gsql``: Loads the Synthea patient data from CSV files

* ``base.gsql``: Infrastructure queries for generating patient vectors and invoking cosine similarity operations

* ``client.gsql``: Queries built on top of ``base.gsql`` queries that an application can invoke to control the
  graph and cosine similarity operations

* ``query.gsql``: Queries built on top of ``base.gsql`` to be invoked from a terminal window as a simple demo

The following subsection describes the last three files in greater detail.

Base.gsql Queries
-----------------

The queries of ``base.gsql`` generally return raw data as GSQL data objects using GSQL ``RETURN`` statements.
The queries are intended to be called from higher-level queries that determine the appropriate style of output
for their intended uses.  The main queries in ``base.gsql`` include:

* ``patient_vector``: Given a patient vertex, this query returns a patient vector.  The query pulls data from
  the patient vertex itself and walks the edges of the patient vertex to pull data from the patient's medical
  history nodes.  To form feature maps (vectors of features) for group attributes, the query calls
  ``udf_get_similarity_vec``, a UDF defined as part of this Synthea demo.

* ``cosinesim_embed_vectors``: This query traverses all patient vertices, calls ``patient_vector`` to form
  the patient vector for each patient, and embeds the vector into the patient vertex.  Cosine similarity operations
  use these embeddings, so this query must be called first.  In a typical database application,
  embeddings would be created in bulk at data loading time, and individually as the database is updated.

* ``cosinesim_match_sw``: This query performs a cosine similarity search using only the CPU, for comparison
  purposes with the hardware-accelerated version.  To save on redundant calculations, it leverages a partial
  computation (the cosine similarity "normal" for each vector) embedded into the patient vertex at the time

* ``cosinesim_set_num_devices``: This query sets the number of Alveo U50 cards to use.  If the number of cards
  is less than the total number of cards installed in the server, cards are chosen in an unspecified manner.

* ``load_graph_cosinesim_ss_fpga_core``: This query transfers all patient vectors to the Alveo card(s).

* ``cosinesim_ss_fpga_core``: Given a patient vertex and a number of results to return, this query invokes the
  cosine similarity match operation on the Alveo card(s), returning the best-matching patients and the
  cosine similarity score for that patient.

Client.gsql Queries
-------------------

The queries in ``client.gsql`` differ from their ``base.gsql`` counterparts in that the ``client.gsql`` queries
return their data in JSON format (using the GSQL ``PRINT`` statement), to facilitate processing of the data
from an application calling the query through TigerGraph's REST interface.
The main queries in ``client.gsql`` include:

* ``client_cosinesim_embed_vectors``: Creates the embeddings by calling ``cosinesim_embed_vectors`` in ``base.gsql``.
  The returned data includes the execution time in milliseconds.

* ``client_cosinesim_match_sw``: Performs a cosine similarity match using only the CPU by calling
  ``cosinesim_match_sw`` in ``base.gsql``.  The output is a JSON array whose elements are a dictionary of
  matching patient ID and consine similarity score.

* ``client_cosinesim_set_num_devices``: Sets the number of Alveo U50 cards to use by calling
  ``cosinesim_set_num_devices`` in ``base.gsql``.  The query returns the number of devices set.

* ``client_cosinesim_get_alveo_status``: Returns status information about the Recommendation Engine, such as
  whether the patient vectors have been loaded into the Alveo card and the number of cards to use.

* ``client_cosinesim_load_alveo``: Transfers all patient vectors to the Alveo card(s) using
  ``load_graph_cosinesim_ss_fpga_core`` in ``base.gsql``.

* ``client_cosinesim_match_alveo``: Invokes the cosine similarity match operation on the Alveo card(s), given
  the target patient to match and the desired number of matches to return.
  The output is a JSON array whose elements are a dictionary of matching patient ID and consine similarity score.
  This query calls ``cosinesim_ss_fpga_core`` in ``base.gsql`` to perform the work.

Query.gsql Queries
------------------

The queries in ``query.gsql`` are simple demonstrations of post-processing the raw data of the ``base.gsql``
queries.  These queries are not intended to be called from an application, as they produce human-readable
output in files.  The ``match.sh`` demo script uses these queries.

* ``cosinesim_ss_tg``: A CPU-based implementation of cosine similarity based on ``cosinesim_match_sw``
  in ``base.gsql``

* ``load_graph_cosinesim_ss_fpga``: Transfers all patient vectors to the Alveo card(s) using
  ``load_graph_cosinesim_ss_fpga_core`` in ``base.gsql``.

* ``cosinesim_ss_fpga``: Runs a cosine similarity match operation in the Alveo cards(s) using
  ``cosinesim_ss_fpga_core`` in ``base.gsql``.
