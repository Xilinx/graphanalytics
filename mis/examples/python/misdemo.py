"""
 * Copyright 2021 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
"""

import xilMisPython as xmis
import os
import argparse
import struct
import time


def readBin(filename, readSize):
    with open(filename, 'rb') as fh:
        fileSize = os.path.getsize(filename)

        if 0 < readSize != fileSize:
            #print(f"WARNING: file {filename} size {fileSize} doesn't match required size {readSize}")
            print("test")

        assert (fileSize >= readSize)

        vec = []
        for i in range(int(fileSize/4)):
            vec.append(struct.unpack('<i', fh.read(4))[0])  # read bytes as little-endian 4 byte integers

    return vec


if __name__ == '__main__':
    # Parse cmdline arguments
    parser = argparse.ArgumentParser(description="Run Xilinx Maximal Independent Set library test")
    parser.add_argument("--xclbin", help="path to xclbin binary", default="./")
    parser.add_argument("--data_dir", help="Directory where data files are", default="../data/", type=str)
    parser.add_argument('--deviceNames', help="device name", default="xilinx_u50_gen3x16_xdma_201920_3", type=str)
    args = parser.parse_args()

    xclbin_path = str(args.xclbin)
    deviceNames = str(args.deviceNames)
    in_dir = str(args.data_dir)

    # search for input file matrix meta data information stored in infos.txt
    with open(in_dir + "/infos.txt") as fh:
        fh.readline()
        n = int(fh.readline())
        fh.readline()
        nz = int(fh.readline())

    # set options for MIS
    opt = xmis.options()
    opt.xclbinPath = xmis.xString(xclbin_path)
    opt.deviceNames = xmis.xString(deviceNames)

    # create MIS object
    mis = xmis.MIS(opt)

    # create CSR arrays and graph object
    h_rowPtr = readBin(in_dir + "/rowPtr.bin", (n + 1) * 4)  # multiplier of 4 for integer data type
    h_colIdx = readBin(in_dir + "/colIdx.bin", nz * 4)       # multiplier of 4 for integer data type
    graph = xmis.GraphCSR(h_rowPtr, h_colIdx)

    # initialize MIS object with the graph
    mis.setGraph(graph)

    # initialize the FPGA device for MIS run
    mis.startMis()

    # execute MIS on FPGA
    start = time.perf_counter()
    mis_vertex_list = mis.executeMIS()
    end = time.perf_counter()
    elapsed = end-start

    # get size of the MIS
    vertex_count = mis.count()  # can also do len(mis_vertex_list)

    #print(f"\nFound MIS with {vertex_count} vertices within {elapsed:0.6f} sec")
    print("test")
