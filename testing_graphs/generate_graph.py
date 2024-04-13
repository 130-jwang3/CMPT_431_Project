import random
import sys

def filter_graph(input_file, output_file, filter):
    # Create a dictionary to store edges
    edges = {}

    # Read the graph from the input file
    with open(input_file, 'r') as f:
        next(f)
        for line in f:
            if not line.startswith('#'):
                parts = line.split()
                from_node = int(parts[0])
                to_node = int(parts[1])
                weight = int(parts[2])

                # Check if the nodes are within the desired range
                if from_node <= filter and to_node <= filter:
                    edges[(from_node, to_node)] = weight

    # Find connected components starting from vertex 0
    connected_components = find_connected_components(edges, filter)

    # Select the largest connected component starting from vertex 0
    largest_component = max(connected_components, key=len)

    # Renumber vertices starting from 0 and ensure consecutive indices
    renumbered_edges, num_vertices = renumber_vertices(edges)

    # Add missing vertices and randomly connect them to existing vertices
    complete_graph = add_missing_vertices(renumbered_edges, num_vertices)

    # Sort the edges by 'from_node'
    sorted_edges = sorted(complete_graph.items(), key=lambda x: x[0])

    # Write the sorted graph to the output file
    with open(output_file, 'w') as f:
        for edge, weight in sorted_edges:
            f.write(f"{edge[0]} {edge[1]} {weight}\n")

def find_connected_components(graph, filter):
    visited = set()
    connected_components = []

    for vertex in range(filter + 1):
        if vertex not in visited:
            component = set()
            dfs_iterative(vertex, graph, visited, component)
            connected_components.append(component)

    return connected_components

def dfs_iterative(start_vertex, graph, visited, component):
    stack = [start_vertex]

    while stack:
        vertex = stack.pop()
        if vertex not in visited:
            visited.add(vertex)
            component.add(vertex)
            for neighbor in graph:
                if vertex in neighbor:
                    next_vertex = neighbor[0] if neighbor[0] != vertex else neighbor[1]
                    if next_vertex not in visited:
                        stack.append(next_vertex)

def renumber_vertices(edges):
    vertex_map = {}
    new_edges = {}
    count = 0

    for edge in edges:
        from_vertex, to_vertex = edge
        if from_vertex not in vertex_map:
            vertex_map[from_vertex] = count
            count += 1
        if to_vertex not in vertex_map:
            vertex_map[to_vertex] = count
            count += 1
        new_edges[(vertex_map[from_vertex], vertex_map[to_vertex])] = edges[edge]

    return new_edges, count

def add_missing_vertices(edges, num_vertices):
    existing_vertices = set(edge[0] for edge in edges)
    complete_graph = edges.copy()

    for i in range(num_vertices):
        if i not in existing_vertices:
            random_vertex = random.choice(list(existing_vertices))
            random_weight = random.randint(1, 1000)
            complete_graph[(i, random_vertex)] = random_weight

    return complete_graph

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("need to specify num_vertices")
        sys.exit(1)

    input_file = "weighted_graph.txt"
    output_file = "filtered_graph.txt"
    num_vertices = int(sys.argv[1])

    filter_graph(input_file, output_file, num_vertices)