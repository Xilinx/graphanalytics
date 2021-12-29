hopNum=1;
i=4096;
bypass=0;
duplicate=1;

testpath=/proj/xsjhdstaff5/yunleiz/nobkup/gitgragh/u50page/nhop/L2/tests/pocnHop
datapath=/proj/xsjhdstaff5/siyangw/nobkup/git_hub_test/nhop10/L2/tests/nHop
#for((i=64; i<=4096; i=i*2))
#do

#    $testpath/build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/host.exe -xclbin build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/nHop_kernel.xclbin --offset csr/europe_osm-csr-offset.mtx --index csr//europe_osm-csr-indicesweights.mtx --pair pair_small/europe_osm-twoHopPair.mtx --golden data/data-golden-small.mtx --batch $i --hop $hopNum --bypass $bypass --duplicate $duplicate
#    mv opencl_summary.csv testResult/europe_osm-batch$i-hop$hopNum.csv

#    $testpath/build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/host.exe -xclbin build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/nHop_kernel.xclbin --offset $datapath/csr/cit-Patents-csr-offset.mtx --index $datapath/csr//cit-Patents-csr-indicesweights.mtx --pair $datapath/pair_small/cit-Patents-twoHopPair.mtx --golden $datapath/data/data-golden-small.mtx --batch $i --hop $hopNum --bypass $bypass --duplicate $duplicate
#    mv opencl_summary.csv $testpath/testResult/cit-Patents-batch$i-hop$hopNum.csv

    $testpath/build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/host.exe -xclbin build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/nHop_kernel.xclbin --offset $datapath/csr/as-Skitter-csr-offset.mtx --index $datapath/csr//as-Skitter-csr-indicesweights.mtx --pair $datapath/pair_small/as-Skitter-twoHopPair.mtx --golden $datapath/data/data-golden-small.mtx --batch $i --hop $hopNum --bypass $bypass --duplicate $duplicate
    mv opencl_summary.csv $testpath/testResult/as-Skitter-batch$i-hop$hopNum.csv

#   $testpath/build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/host.exe -xclbin build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/nHop_kernel.xclbin --offset csr/soc-LiveJournal1-csr-offset.mtx --index csr//soc-LiveJournal1-csr-indicesweights.mtx --pair pair_small/soc-LiveJournal1-twoHopPair.mtx --golden data/data-golden-small.mtx --batch $i --hop $hopNum --bypass $bypass --duplicate $duplicate
#    mv opencl_summary.csv testResult/soc-LiveJournal1-batch$i-hop$hopNum.csv

    $testpath/build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/host.exe -xclbin build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/nHop_kernel.xclbin --offset $datapath/csr/coPapersDBLP-csr-offset.mtx --index $datapath/csr//coPapersDBLP-csr-indicesweights.mtx --pair $datapath/pair_small/coPapersDBLP-twoHopPair.mtx --golden $datapath/data/data-golden-small.mtx --batch $i --hop $hopNum --bypass $bypass --duplicate $duplicate
    mv opencl_summary.csv $testpath/testResult/coPapersDBLP-batch$i-hop$hopNum.csv

    $testpath/build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/host.exe -xclbin build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/nHop_kernel.xclbin --offset $datapath/csr/coPapersCiteseer-csr-offset.mtx --index $datapath/csr//coPapersCiteseer-csr-indicesweights.mtx --pair $datapath/pair_small/coPapersCiteseer-twoHopPair.mtx --golden $datapath/data/data-golden-small.mtx --batch $i --hop $hopNum --bypass $bypass --duplicate $duplicate
    mv opencl_summary.csv $testpath/testResult/coPapersCiteseer-batch$i-hop$hopNum.csv

#    $testpath/build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/host.exe -xclbin build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/nHop_kernel.xclbin --offset csr/ljournal-2008-csr-offset.mtx --index csr//ljournal-2008-csr-indicesweights.mtx --pair pair_small/ljournal-2008-twoHopPair.mtx --golden data/data-golden-small.mtx --batch $i --hop $hopNum --bypass $bypass --duplicate $duplicate
#    mv opencl_summary.csv testResult/ljournal-2008-batch$i-hop$hopNum.csv

#    $testpath/build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/host.exe -xclbin build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/nHop_kernel.xclbin --offset csr/hollywood-csr-offset.mtx --index csr//hollywood-csr-indicesweights.mtx --pair pair_small/hollywood-twoHopPair.mtx --golden data/data-golden-small.mtx --batch $i --hop $hopNum --bypass $bypass --duplicate $duplicate
#    mv opencl_summary.csv testResult/hollywood-batch$i-hop$hopNum.csv

#done

grep "nHop_kernel,1," $testpath/testResult -r | grep ":nHop_kernel,1," | tee $testpath/testResult/log
