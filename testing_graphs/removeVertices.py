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

    # Write the filtered graph to the output file
    with open(output_file, 'w') as f:
        for edge in edges:
            if edge[0] in largest_component and edge[1] in largest_component:
                f.write(f"{edge[0]} {edge[1]} {edges[edge]}\n")

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

input_file = "weighted_graph.txt"
output_file = "filtered_graph.txt"
filter_graph(input_file, output_file, 40000)
