#!/usr/bin/env python3

import argparse


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def add_entry(edge_list, start_vertex, end_vertex, weight):
    # If start_vertex doesn't have edges yet, create a new list for the vertex
    if start_vertex not in edge_list:
        edge_list[start_vertex] = []

    # If the edge already exists, discard the new one (even if the weights are different)
    for edge in edge_list[start_vertex]:
        if edge[0] == end_vertex:
            return 0

    # Add the new edge to the edge list
    edge_list[start_vertex].append((end_vertex, weight))
    return 1


def tuple_sort_key(t):
    return t[0]


def renumber_vertex(old_vertex, vertex_map):
    if old_vertex in vertex_map:
        return vertex_map[old_vertex]
    else:
        return old_vertex


def main():
    argParser = argparse.ArgumentParser(description="Sorts a .mtx file.",
                                        formatter_class=argparse.RawDescriptionHelpFormatter)
    argParser.add_argument('input', help="the source .mtx file to sort")
    argParser.add_argument('output', help="the destination .mtx file")
    argParser.add_argument('-r', '--renumber', action='store_true',
        help="renumber the vertices to consecutive numbers, starting from 0")
    argParser.add_argument('-d', '--directed', action='store_true', help="trim edges to go in one direction only")
    argParser.add_argument('-w', '--weight', action='store_true', help="sets all weights to 1")
    args = argParser.parse_args()

    in_file_name = args.input
    out_file_name = args.output

    # Read the input file, forming an dictionary of edges, where the key is the start vertex and the value
    # is a list of tuples of (end vertex, weight)
    print("Reading input file " + in_file_name + "...")
    num_vertices = 0
    num_edges = 0
    try:
        in_file = open(in_file_name, 'r')
        line_num = 1
        edge_list = dict()
        for line in in_file:
            if line[0] == '*':
                continue
            if line_num % 1000000 == 0:
                print("  Line " + str(line_num))
            tokens = line.split();
            if len(tokens) == 0:
                continue
            if len(tokens) != 3:
                eprint("WARNING: line " + str(line_num) + ": malformed edge: " + line)
                continue
            try:
                start_vertex = int(tokens[0])
                end_vertex = int(tokens[1])
                weight = float(tokens[2])
                if start_vertex > end_vertex:
                    start_vertex, end_vertex = end_vertex, start_vertex
                num_edges += add_entry(edge_list, start_vertex, end_vertex, weight)
                if not args.directed:
                    num_edges += add_entry(edge_list, end_vertex, start_vertex, weight)
            except ValueError as ex:
                eprint("WARNING: line " + str(line_num) + ": non-integer vertex: " + line)
            line_num += 1
        in_file.close()
    except OSError as ex:
        eprint("Couldn't open file " + in_file_name + " for reading: " + ex.strerror)
        return

    # Sort the vertex list
    print("Sorting vertices...")
    sorted_vertices = sorted(edge_list.keys())
    num_vertices = len(sorted_vertices)

    # If renumbering, make a dictionary of old vertex numbers to new vertex numbers
    new_vertices = dict()
    if args.renumber:
        print("Renumbering vertices...")
        next_id = 0
        for old_vertex in sorted_vertices:
            new_vertices[old_vertex] = next_id
            next_id += 1

    # Write the output .mtx file in sorted order
    print("Writing output file " + out_file_name + "...")
    try:
        out_file = open(out_file_name, 'w')
        out_file.write('*Vertices ' + str(num_vertices) + '\n')
        out_file.write('*Edges ' + str(num_edges) + '\n')
        for vertex in sorted_vertices:
            sorted_edges = sorted(edge_list[vertex], key=tuple_sort_key)
            for edge in sorted_edges:
                if args.weight:
                    weight_str = '1'
                else:
                    weight_str = str(edge[1])
                out_file.write(str(renumber_vertex(vertex, new_vertices)) + ' '
                        + str(renumber_vertex(edge[0], new_vertices)) + ' ' + weight_str + '\n')
        out_file.close()
    except OSError as ex:
        eprint("Couldn't open file " + out_file_name + " for writing: " + ex.strerror)
        return


if __name__ == '__main__':
    main()
