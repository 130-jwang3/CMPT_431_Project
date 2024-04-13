import random
import sys
import zipfile

def filter_graph(input_zip, output_file, filter):
    edges = {}
    node_set = set()

    # Open the zip file
    with zipfile.ZipFile(input_zip) as z:
        # Open the specific text file within the zip file
        with z.open('weighted_graph.txt') as f:
            next(f)  # Skip the header line if it exists
            for line in f:
                line = line.decode('utf-8')  # Decode bytes to str
                if not line.startswith('#'):
                    parts = line.split()
                    from_node = int(parts[0])
                    to_node = int(parts[1])
                    weight = int(parts[2])

                    if from_node <= filter and to_node <= filter:
                        edges[(from_node, to_node)] = weight
                        node_set.update([from_node, to_node])

    # Find connected components
    connected_components = find_connected_components(edges, filter, node_set)
    largest_component = max(connected_components, key=len)

    # Make sure to include all vertices from 0 to filter in the graph
    all_vertices = set(range(filter + 1))
    missing_vertices = all_vertices - largest_component

    # Collect all valid edges
    valid_edges = [
        (from_node, to_node, edges[(from_node, to_node)])
        for from_node, to_node in edges
        if from_node in largest_component and to_node in largest_component
    ]

    # Connect missing vertices to the largest component
    for missing in missing_vertices:
        connect_to = random.choice(list(largest_component))
        random_weight = random.randint(1, 999)
        valid_edges.append((missing, connect_to, random_weight))
        # Add the reverse connection for undirected graph consistency
        valid_edges.append((connect_to, missing, random_weight))

    # Sort edges by 'fromNode'
    valid_edges.sort()

    # Write the sorted edges to the output file
    with open(output_file, 'w') as f:
        for from_node, to_node, weight in valid_edges:
            f.write(f"{from_node} {to_node} {weight}\n")

def find_connected_components(graph, filter, node_set):
    visited = set()
    connected_components = []

    def dfs_iterative(start_vertex):
        stack = [start_vertex]
        component = set()

        while stack:
            vertex = stack.pop()
            if vertex not in visited:
                visited.add(vertex)
                component.add(vertex)
                for (u, v), w in graph.items():
                    if vertex == u and v not in visited:
                        stack.append(v)
                    elif vertex == v and u not in visited:
                        stack.append(u)
        return component

    for vertex in range(filter + 1):
        if vertex not in visited and vertex in node_set:  # Start DFS only for nodes present in edges
            component = dfs_iterative(vertex)
            if component:  # only add non-empty components
                connected_components.append(component)

    return connected_components


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("need to specify num_vertices")
        sys.exit(1)

    input_file = "weighted_graph.zip"
    num_vertices = int(sys.argv[1])
    output_file = f"graph_{num_vertices}.txt"

    filter_graph(input_file, output_file, num_vertices)