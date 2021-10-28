# Instructions
## Install UDFs
The UDFs only need to be installed once.
```
./bin/install-udf.sh
```

## Run Louvain Modularity on CPU and FPGA
### Print help messages
```
./bin/run.sh -h
```

### Example command lines
```
Run Louvain Modularity on CPU only: set run_mode to 0
./bin/run.sh -r 0

Run Louvain Modularity on CPU only with your own graph .mtx file
./bin/run.sh -r 0 -s /path/to/dataset.mtx

```


