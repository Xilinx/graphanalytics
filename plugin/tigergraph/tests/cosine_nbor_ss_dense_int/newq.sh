#!/bin/bash

echo "Loading cache..."
time gsql -g xgraph "set query_timeout=240000000 run query client_load_cache()"
echo "Running SW cosine sim..."
time gsql -g xgraph "set query_timeout=240000000 run query client_cosinesim_sw(10, \"37b6517f-844d-4201-99f1-f3e8911e2d2c\")"
