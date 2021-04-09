# Location of cosine similarity Alveo product
test -z "$XILINX_COSINESIM" && XILINX_COSINESIM=/opt/xilinx/apps/graphanalytics/cosinesim

# Location of XRT and XRM
test -z "$XILINX_XRT" && XILINX_XRT=/opt/xilinx/xrt
test -z "$XILINX_XRM" && XILINX_XRM=/opt/xilinx/xrm

# Location of the Python library
PYTHON_LIB_PATH=$PWD/../../staging/lib
test -d "$PYTHON_LIB_PATH" || PYTHON_LIB_PATH=$XILINX_COSINESIM/lib
export PYTHONPATH=$PYTHON_LIB_PATH:$PYTHONPATH

# Setup Xilinx Tools
. $(XILINX_XRT)/setup.sh
. $(XILINX_XRM)/setup.sh

# Run the demo
python3 pythondemo.py
