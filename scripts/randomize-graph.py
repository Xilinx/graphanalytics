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
import numpy as np


in_file = sys.argv[1]
out_file = sys.argv[2]

out_fh = open(out_file, 'w')
line_cnt = 1
num_vertices = 0
num_edges = 0
out_line_cnt = 0
out_num_vertices = 0
out_num_edges = 0

# first pass: to get number of edges in output
print('INFO: Getting number of vertices and edges')
with open(in_file) as in_fh:
    for line in in_fh:
        line = line.strip()
        if line_cnt <= 2:
            fields = line.split(' ')
            if fields[0] == '*Vertices':
                num_vertices = int(fields[1])
                print('INFO: number of vertices from the input graph:', num_vertices)
            if fields[0] == '*Edges':
                num_edges = int(fields[1])
                print('INFO: number of edges from the input graph:', num_edges)
        elif line_cnt > 2:
            break
        
        line_cnt += 1

print('INFO: Initializing edge list...')
edge_list_head = np.zeros(num_edges, dtype=int)
edge_list_tail = np.zeros(num_edges, dtype=int)
edge_list_weight = np.zeros(num_edges, dtype=float)
vertex_ids = np.arange(num_vertices) + 1

# vertex ID in mtx starts with 1
vertex_map = np.random.choice(vertex_ids, size=num_vertices, replace=False)
#for v in vertex_map:
#    print(v)

edge_cnt = 0
line_cnt = 0
print('INFO: Populating edges with renumbered vertex id...')
with open(in_file) as in_fh:
    for line in in_fh:
        line = line.strip()
        if line_cnt <= 2:
            pass
        elif line_cnt > 2:
            fields = line.split(' ')
            head_new = vertex_map[int(fields[0]) - 1]
            tail_new = vertex_map[int(fields[1]) - 1]
            edge_list_head[edge_cnt] = head_new
            edge_list_tail[edge_cnt] = tail_new
            edge_list_weight[edge_cnt] = float(fields[2])
            edge_cnt += 1
            #print(int(fields[0]), int(fields[1]), head_new, tail_new)

        line_cnt += 1
        if (line_cnt % 5000000 == 0):
            print('INFO: input line_cnt=', line_cnt)

# sort by the head
head_sort_idx = np.argsort(edge_list_head)

print('INFO: Saving new graph...')
line_cnt = 1
print('*Vertices', num_vertices, file=out_fh)
print('*Edges', num_edges, file=out_fh)
with open(in_file) as in_fh:
    for idx in head_sort_idx:
        if edge_list_weight[idx] > 0:
            print(edge_list_head[idx], edge_list_tail[idx], edge_list_weight[idx], file=out_fh)

        line_cnt += 1
        if (line_cnt % 5000000 == 0):
            print('INFO: out line_cnt=', line_cnt)

out_fh.close()
print('INFO: the output graph is saved to', out_file)


