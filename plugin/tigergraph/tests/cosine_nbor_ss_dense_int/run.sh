#!/bin/bash
time gsql schema_xgraph.gsql
time gsql load_xgraph.gsql
time gsql base.gsql
time gsql query.gsql
echo "Run query cosinesim_ss_tg"
time gsql -g xgraph "set query_timeout=240000000 run query cosinesim_ss_tg(\"$PWD/tg.txt\")"
echo "Run query loadgraph_cosinesim_ss_fpga"
time gsql -g xgraph "set query_timeout=240000000 run query loadgraph_cosinesim_ss_fpga()"
echo "Run query cosinesim_ss_fpga"
time gsql -g xgraph "set query_timeout=240000000 run query cosinesim_ss_fpga(\"$PWD/fpga1.txt\", \"$PWD/fpga2.txt\")"
echo "Run query close_fpga"
time gsql -g xgraph "set query_timeout=240000000 run query close_fpga()"
