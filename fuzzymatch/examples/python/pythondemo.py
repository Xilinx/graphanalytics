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
args = parser.parse_args()

xclbin_path= str(args.xclbin)
deviceNames= str(args.deviceNames)
#load csv
peopleFile = str(args.data_dir) + "people.csv"
trans_num=100
test_input = str(args.data_dir) + "txdata.csv"


stats=pd.read_csv(test_input, delimiter=',',names = ['Id','Company,Channel','OperationType','Contract','Product','ProductSubtype','OperationTypeforAML','Currency','Amount','TransactionDescription','SwiftCode1','Bank1','SwiftCode2','Bank2','NombrePersona1','TipoPersona1','CodigoPersona1','NombrePersona2','TipoPersona2','CodigoPersona2','FechaDeCorte','FechaDeValor'])

peopleVecs=pd.read_csv(peopleFile, delimiter=',',names = ['Id','Name'])

totalEntities = 10000000

stats=stats.iloc[1:]
peopleVecs=peopleVecs.iloc[1:]
peopleVec=peopleVecs[['Name']]
#print(peopleVec)
data_vec=stats[['NombrePersona1','NombrePersona2']]

inputVec=[]
print(len(peopleVec['Name']))
for idx in range(1,len(peopleVec['Name'])):
    #print(peopleVec['Name'][idx])
    inputVec.append(peopleVec['Name'][idx])

#peopleFile
opt = xfm.options()
opt.xclbinPath=xfm.xString(xclbin_path)
opt.deviceNames=xfm.xString(deviceNames)
mchecker = xfm.FuzzyMatch(opt)
stat_check=mchecker.startFuzzyMatch()
stat_check=mchecker.fuzzyMatchLoadVec(inputVec)

test_transaction=[]


for idx in range (trans_num):
    test_transaction.append(data_vec['NombrePersona1'][idx])
    #test_transaction.append(data_vec['NombrePersona2'][idx])

ccnt=0
result_list=[]
performance=[]
min = sys.float_info.max
max = 0.0
sum = 0.0
for idx in range (trans_num):
    start=timer()
    #print(test_transaction[idx])
    resMatch = mchecker.executefuzzyMatch(test_transaction[idx])
    #print(resMatch)
    end = timer()
    timeTaken = (end - start)*1000
    result_list.append(resMatch)
    if min > timeTaken:
        min = timeTaken
    if max < timeTaken:
        max = timeTaken
    sum += timeTaken
    performance.append(timeTaken)
    ccnt += 1

#print result
print('\nTransaction Id, OK/KO, Field of match, Time taken(:ms)')
for idx in range (trans_num):
    s=print_result(idx,result_list[idx],performance[idx])
    print(s)
#
print('\nFor FPGA')
if ccnt < trans_num :
    num = trans_num-ccnt
    print(ccnt,'transactions were processed', num, 'were skipped due to the existence of empty field(s) in these transaction.\n(Details are given above)\n')
else:
    print(trans_num,'transactions were processed\n')
#
print('Min(ms)\t\tMax(ms)\t\tAvg(ms)\n')
print('----------------------------------------')
print('{:.3f}'.format(min) , '\t\t','{:.3f}'.format(max),'\t\t', '{:.3f}'.format(sum / ccnt) , '\n')
print('----------------------------------------')


