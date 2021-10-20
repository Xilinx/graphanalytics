# Source XRT environment
`source /opt/xilinx/xrt/setup.sh`

# Build the test
`make test`

# Run the test 
`make run`

## Run executable directly outside make after running 'make test'
To run the executable directly, set **LD_LIBRRAY_PATH** to include ../../lib directory.

Example: 

`export LD_LIBRARY_PATH=../../lib:$LD_LIBRARY_PATH`

Then specify the right xclbin for the FPGA device being used as the -xclbin argument, and execute as below:

### On Alveo U50
`./test -xclbin ../../xclbin/fuzzy_xilinx_u50_gen3x16_xdma_201920_3.xclbin -d ../data/ -c 0`

### On Alveo U200
`./test -xclbin ../../xclbin/fuzzy_xilinx_u200_xdma_201830_2.xclbin -d ../data/ -c 0`

### On Alveo U250
`unzip -d ../../xclbin ../../xclbin/fuzzy_xilinx_u250_xdma_201830_2.zip`
`./test -xclbin ../../xclbin/fuzzy_xilinx_u250_xdma_201830_2.xclbin -d ../data/ -c 0`

### On AWS F1 
`./test -xclbin ../../xclbin/fuzzy_xilinx_aws-vu9p-f1_shell-v04261818_201920_2.awsxclbin -d ../data/ -c 0`

#### To run this benchmark on multi-thread CPU without FPGA, run:
Run the above command with '-c 1' instead of of '-c 0'

#### To run on FPGA followed by run on multi-thread CPU and compare the results, run:
Run the above command with '-c 2' instead of of '-c 0'
