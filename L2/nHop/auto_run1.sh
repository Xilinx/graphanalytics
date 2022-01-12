hopNum=2;
i=1024;
bypass=1;
duplicate=0;
testpath=/proj/xsjhdstaff5/yunleiz/nobkup/gitgragh/u50page/nhop/L2/tests/pocnHop
datapath=/proj/xsjhdstaff5/siyangw/nobkup/git_hub_test/
mkdir testResult1

    build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/host.exe -xclbin $testpath/nHop_kernel.xclbin --graph /proj/xsjhdstaff5/yunleiz/Demo_For_webinary_WT/europe_osm-wt.mtx --pair $datapath/pair_small/europe_osm-twoHopPair.mtx --golden data/data-golden-small.mtx --batch $i --hop $hopNum --bypass $bypass --duplicate $duplicate --test 0 
    mv summary.csv ./testResult1/europe_osm-batch$i-hop$hopNum-bypass$bypass.csv

#    build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/host.exe -xclbin $testpath/nHop_kernel.xclbin --graph /proj/xsjhdstaff5/yunleiz/Demo_For_webinary_WT/soc-LiveJournal1-wt.mtx --pair $datapath/pair_small/soc-LiveJournal1-twoHopPair.mtx --golden data/data-golden-small.mtx --batch $i --hop $hopNum --bypass $bypass --duplicate $duplicate --limit 20 --test 0
#    mv summary.csv ./testResult1/soc-LiveJournal-batch$i-hop$hopNum-bypass$bypass.csv 

    build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/host.exe -xclbin $testpath/nHop_kernel.xclbin --graph /proj/xsjhdstaff5/yunleiz/Demo_For_webinary_WT/cit-Patents-wt.mtx --pair $datapath/pair_small/cit-Patents-twoHopPair.mtx --golden data/data-golden-small.mtx --batch $i --hop $hopNum --bypass $bypass --duplicate $duplicate
    mv summary.csv ./testResult/cit-Patents-batch$i-hop$hopNum-bypass$bypass.csv

#    build_dir.sw_emu.xilinx_u50_gen3x16_xdma_201920_3/host.exe -xclbin $testpath/nHop_kernel.xclbin --offset $datapath/csr/cit-Patents-csr-offset.mtx --index $datapath/csr//cit-Patents-csr-indicesweights.mtx --pair $datapath/pair_small/cit-Patents-twoHopPair.mtx --golden data/data-golden-small.mtx --batch $i --hop $hopNum --bypass $bypass --duplicate $duplicate --test
#    mv summary.csv ./testResult/cit-Patents-batch$i-hop$hopNum-bypass$bypass.csv


