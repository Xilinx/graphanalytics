#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

import sys

in_file = sys.argv[1]
out_file = sys.argv[2]
factor = sys.argv[3]

out_fh = open(out_file, 'w')
line_cnt = 1
num_vertices = 0
num_edges = 0
out_line_cnt = 0
out_num_vertices = 0
out_num_edges = 0

# first pass: to get number of edges in output
with open(in_file) as in_fh:
    for line in in_fh:
        line = line.strip()
        if line_cnt <= 2:
            fields = line.split(' ')
            if fields[0] == '*Vertices':
                num_vertices = int(fields[1])
                print('INFO: number of vertices from the input graph:', num_vertices)
                out_num_vertices = int(num_vertices/100)

            if fields[0] == '*Edges':
                num_edges = int(fields[1])
                print('INFO: number of edges from the input graph:', num_edges)
        elif line_cnt > 2:
            fields = line.split(' ')
            start_vertex = int(fields[0])
            end_vertex = int(fields[1])
            weight = fields[2]
            if start_vertex <= out_num_vertices and end_vertex <= out_num_vertices:
                out_num_edges += 1

            #out_fh.write(line)
            #out_line_cnt += 1
            
        line_cnt += 1

# second pass: save data to output
line_cnt = 1
print('*Vertices', out_num_vertices, file=out_fh)
print('*Edges', out_num_edges, file=out_fh)
with open(in_file) as in_fh:
    for line in in_fh:
        line = line.strip()
        if line_cnt > 2:
            fields = line.split(' ')
            start_vertex = int(fields[0])
            end_vertex = int(fields[1])
            weight = fields[2]
            if start_vertex <= out_num_vertices and end_vertex <= out_num_vertices:
                print(start_vertex, end_vertex, weight, file=out_fh)
                out_num_edges += 1
        
        line_cnt += 1

out_fh.close()
print('INFO: the output graph is saved to', out_file)
print('INFO: number of vertices in the output graph', out_num_vertices)
print('INFO: number of vertices in the output graph', out_num_edges)


