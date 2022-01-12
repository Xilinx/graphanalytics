hopNum=1;
i=4096;
bypass=0;
duplicate=1;

testpath=/proj/xsjhdstaff5/yunleiz/nobkup/gitgragh/u50page/nhop/L2/tests/pocnHop
cd /proj/xsjhdstaff5/siyangw/nobkup/git_hub_test/nhop10/L2/tests/nHop
#for((i=64; i<=4096; i=i*2))
#do

    $testpath/build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/host.exe -xclbin build_dir.hw.xilinx_u50_gen3x16_xdma_201920_3/nHop_kernel.xclbin --offset data/data-csr-offset.mtx --index data/data-csr-indicesweights.mtx --pair data/data-pair.mtx --golden data/data-golden-small.mtx --batch $i --hop $hopNum --bypass $bypass --duplicate $duplicate
    cd -
    cp opencl_summary.csv $testpath/testResult/data-batch$i-hop$hopNum.csv

#done


grep "nHop_kernel,1," $testpath/testResult -r | grep ":nHop_kernel,1," | tee $testpath/testResult/log
