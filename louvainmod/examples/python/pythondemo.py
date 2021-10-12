"""
 * Copyright 2020-2021 Xilinx, Inc.
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

import xilLouvainmod as xlm
import argparse


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Run LouvainMod Demo')
    parser.add_argument('--mode', choices=['partition', 'load', 'run'])
    parser.add_argument('--graph', type=str)
    parser.add_argument('--name', type=str) 
    parser.add_argument('--num_pars', type=int)
    parser.add_argument('--mode_zmq', choices=['driver', 'worker0', 'worker1'])
    parser.add_argument('--numDevices', type=int)
    parser.add_argument('--num_wrk', type=int)
    parser.add_argument('--deviceNames', type=str)
    parser.add_argument('--alveoProject', type=str)
    parser.add_argument('--xclbin', type=str)
    parser.add_argument('--num_iter', type=int)
    parser.add_argument('--num_level', type=int)
    args = parser.parse_args()

    
    opt = xlm.options()
    
    if args.mode == 'partition':
      opt.modeAlveo = 1
      opt.hostIpAddress = xlm.xString('192.168.0.11')
      opt.clusterIpAddresses= xlm.xString('192.168.0.11')
      opt.nameProj = xlm.xString(args.name)
    else:
      opt.modeAlveo = 2
      opt.hostIpAddress = xlm.xString('127.0.0.1')
      opt.clusterIpAddresses = xlm.xString('127.0.0.1')
      if args.num_wrk == 2:
        opt.clusterIpAddresses = xlm.xString('127.0.0.1 127.0.0.1 192.168.1.21 192.168.1.31')
      opt.xclbinPath = xlm.xString(args.xclbin)
      opt.alveoProject = xlm.xString(args.alveoProject)
      opt.numDevices = args.numDevices
      opt.deviceNames = xlm.xString(args.deviceNames)
     
    # Create options for louvainMod
    
    opt.kernelMode = 2
    
    if args.mode_zmq == 'driver':
      opt.nodeId = 0
    elif args.mode_zmq == 'worker0':
      opt.nodeId = 1
    else:
      opt.nodeId = 2
      
    opt.hostName = xlm.xString("localhost")
    
    # Create louvainMod object
    lm = xlm.louvainMod(opt)
    
    if args.mode == 'partition':
      parOpt = xlm.partitionOptions()
      parOpt.numPars = args.num_pars
      parOpt.par_prune = 1
      lm.partitionDataFile(args.graph, parOpt)
    elif args.mode == 'load':
      comOpt = xlm.computeOptions()
      #comOpt.outputFile = 
      comOpt.max_iter = args.num_level 
      comOpt.max_level = args.num_iter
      comOpt.tolerance = 1e-06
      comOpt.intermediateResult = False
      comOpt.final_Q = True
      comOpt.all_Q = False
      finalQ = lm.loadAlveoAndComputeLouvain(comOpt)
      if finalQ < -1:
            print("ERROR: loadAlveoAndComputeLouvain completed with error. ErrorCode=", finalQ)
      elif args.mode_zmq == 'driver':
            print("INFO: loadAlveoAndComputeLouvain completed. finalQ=", finalQ);
    else:
      pass
