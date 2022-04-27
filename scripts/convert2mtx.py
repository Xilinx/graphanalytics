"""
 * Copyright 2022 AMD, Inc.
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
from pathlib import Path
import argparse

parser = argparse.ArgumentParser(description="Convert binary file to mtx file")
parser.add_argument("--binary_file", help="binary file", default="./g500-6", type=str )

args = parser.parse_args()
binary_file = str(args.binary_file)
mtx_file = binary_file + ".mtx"
print(binary_file)
data = Path(binary_file).read_bytes() #Python 3.5+

#to create int from byte 0-7 of the data
i = int.from_bytes(data[:8], byteorder='little', signed=False)
print(i)

#iterate the data, for each 8 byte convert to int
print(len(data)/16)

edges_num = int(len(data)/16)
#find the max vertices
verticenum = -9999
f = open(mtx_file, "w")
weight = "1.00000"
for idx in range(edges_num):
   i = int.from_bytes(data[idx*8:(idx+1)*8], byteorder='little', signed=False)
   j = int.from_bytes(data[(idx+1)*8:(idx+2)*8], byteorder='little', signed=False)
   verticenum = max(verticenum,i)
   verticenum = max(verticenum,j)

input_line = "*Vertices " + str(verticenum) + "\n"
f.write(input_line)
input_line = "*Edges " + str(edges_num) + "\n"
f.write(input_line)

for idx in range(edges_num):
   i = int.from_bytes(data[idx*8:(idx+1)*8], byteorder='little', signed=False)
   j = int.from_bytes(data[(idx+1)*8:(idx+2)*8], byteorder='little', signed=False)
   #print(i,",", j, weight)
   input_line = str(i) + " " + str(j) + " " + weight + "\n"
   f.write(input_line)


print("Vertices:" ,  verticenum)
print("Edges:" , edges_num)
f.close()

