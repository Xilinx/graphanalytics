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
    print('INFO: Computing best community partition...', mtxFile)
    tStart = time.perf_counter()
    partition = community_louvain.best_partition(graph)
    print(f'INFO: Completed in {time.perf_counter() - tStart:.4f} secs')
else:
    # read partition files
    for i in range(1, nodes+1):
        # start from index 1
        parFile = partitionFile + str(i)
        with open(parFile, 'r') as fp:
            for line in fp:
                vertexCommunity = line.rstrip().split(',')
                partition[int(vertexCommunity[0])] = vertexCommunity[1]

print('INFO: Number of communities:', len(set(partition.values())))
print('INFO: Number of vertices in all communities:', len(partition))

#print(partition)
#new_partition = {}
#for k in partition.keys():
#    new_partition[k] = k
#print('INFO: Number of communities:', len(set(new_partition.values())))
#print('INFO: Number of vertices in all communities:', len(new_partition))

print('INFO: Computing modularity...', mtxFile)
tStart = time.perf_counter()
modularity = community_louvain.modularity(new_partition, graph)
print(f'INFO: Completed in {time.perf_counter() - tStart:.4f} secs')
print('INFO: modularity=', modularity)
