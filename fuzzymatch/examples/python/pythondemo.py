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

import xilFuzzyMatchPython as xfm
import os
import os.path
import sys
import argparse
import pandas as pd
import numpy as np
from timeit import default_timer as timer


def print_result(id, r, timeTaken):
    res = str(id)+','
    if r:
        res += "KO" 
        res += ":"
        res += "Sender"
    else:
        res+= "OK"
    
    res += ","
    res += '{:.6f}'.format(timeTaken)

    return res



parser = argparse.ArgumentParser(description="Run fuzzyMatch test")
parser.add_argument("--xclbin", help="xclbin path", default="./")
parser.add_argument("--data_dir", help="Directory where data files are", default="../data/", type=str )
parser.add_argument('--deviceNames', help="device name", default="xilinx_u50_gen3x16_xdma_201920_3", type=str)
parser.add_argument('--threshold', help="threshold", default=90, type=int)
args = parser.parse_args()

xclbin_path= str(args.xclbin)
deviceNames= str(args.deviceNames)
threshold = int(args.threshold)
#load csv
peopleFile = str(args.data_dir) + "all-names.csv"
trans_num=100
test_input = str(args.data_dir) + "new-names.csv"
stats=pd.read_csv(test_input, delimiter=',', names=['Id','Name'])
peopleVecs=pd.read_csv(peopleFile, delimiter=',',names = ['Id','Name'])

totalEntities = 10000000

stats=stats.iloc[1:]
peopleVecs=peopleVecs.iloc[1:]
peopleVec=peopleVecs[['Name']]
data_vec=stats[['Name']]

inputVec=[]
inputId=[]
print(len(peopleVec['Name']))
for idx in range(1,len(peopleVec['Name'])):
    #print(peopleVec['Name'][idx])
    inputVec.append(peopleVec['Name'][idx])

#create options
opt = xfm.options()
opt.xclbinPath=xfm.xString(xclbin_path)
opt.deviceNames=xfm.xString(deviceNames)
#create fuzzymatch object
mchecker = xfm.FuzzyMatch(opt)
#start fuzzymatch 
stat_check=mchecker.startFuzzyMatch()
#load big table
stat_check=mchecker.fuzzyMatchLoadVec(inputVec,inputId)

# create input patterns 
test_transaction=[]
print('the size of data_vec',len(data_vec))

for idx in range (1,len(data_vec)):
    test_transaction.append(data_vec['Name'][idx])

result_list={}

# run fuzzymatch on input patterns in batch mode 
start=timer()
result_list = mchecker.executefuzzyMatch(test_transaction, threshold)
end = timer()
timeTaken = (end - start)*1000

#print(result_list)

for idx in range (0,len(data_vec)-1):
    print(test_transaction[idx],"---> ", end=' ')
    if not result_list[idx]:
        print("no match")
    else :
        for item in result_list[idx]:
            print('{',inputVec[item[0]-1],':', item[1], '}', end=';  ')
        print()

    

print('Average time taken per string', '{:.3f}'.format(timeTaken/len(test_transaction)) , '\n')

