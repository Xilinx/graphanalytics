#!/usr/bin/env python3

# Prerequisite
# pip install -r louvain-py-requirements.txt

import networkx as nx
import community as community_louvain
import time
import sys

mtxFile = sys.argv[1]
# 1: compute partition using Python Louvain
# 0: load partition from an inputfile
computePartition = int(sys.argv[2])
partitionFile = sys.argv[3]
nodes = int(sys.argv[4])
outClusterInfoFile = None
if len(sys.argv) > 5:
    outClusterInfoFile = sys.argv[5]

delimiter = ' '
if len(sys.argv) > 6:
    delimiter = sys.argv[6]

print(outClusterInfoFile, delimiter) 

# mtx index starts at 1 while C index starts at 1. Add the offset below to C 
# based index
vertexIdOffset = 1

print('INFO: Loading graph from', mtxFile)
edges = []
tStart = time.perf_counter()
with open(mtxFile, 'r') as fp:
    for line in fp:
        #print(line)
        edgeIn = line.split()
        if edgeIn[0] == '*Vertices' or edgeIn[0] == '*Edges':
            continue
        else:
            edge = (int(edgeIn[0]), int(edgeIn[1]))
            edges.append(edge)

graph = nx.Graph()
graph.add_edges_from(edges)
print(f'INFO: Completed in {time.perf_counter() - tStart:.4f} secs')
print('INFO: Total edges', len(edges))

partition = {}
if computePartition:
    print('INFO: Computing best community partition...', partitionFile)
    tStart = time.perf_counter()
    partition = community_louvain.best_partition(graph)
    print(f'INFO: Completed in {time.perf_counter() - tStart:.4f} secs')

    if outClusterInfoFile is not None:
        with open(outClusterInfoFile, 'w') as fp:
            for k in partition.keys():
                print(k-vertexIdOffset, partition[k], file=fp)
else:
    print('INFO: Reading cluster partition from', mtxFile)

    # read partition files
    for i in range(1, nodes+1):
        # start from index 1
        if nodes == 1:
            parFile = partitionFile
        else:
            parFile = partitionFile + str(i)

        with open(parFile, 'r') as fp:
            for line in fp:
                vertexCommunity = line.rstrip().split(delimiter)
                partition[int(vertexCommunity[0])+vertexIdOffset] = vertexCommunity[1]

print('INFO: Number of communities:', len(set(partition.values())))
print('INFO: Number of vertices in all communities:', len(partition))

print('INFO: Computing modularity...', mtxFile)
tStart = time.perf_counter()
modularity = community_louvain.modularity(partition, graph)
print(f'INFO: Completed in {time.perf_counter() - tStart:.4f} secs')
print('INFO: modularity=', modularity)

