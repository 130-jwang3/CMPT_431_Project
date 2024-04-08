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

    # Write the filtered graph to the output file
    with open(output_file, 'w') as f:
        for edge, weight in edges.items():
            f.write(f"{edge[0]} {edge[1]} {weight}\n")

# Usage example
input_file = "weighted_graph.txt"
output_file = "filtered_graph.txt"
filter_graph(input_file, output_file, 10)   #remove vertices that have >500000 index
