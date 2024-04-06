import random

def read_graph_from_file(file_path):
    graph = {}
    with open(file_path, 'r') as file:
        next(file)  # Skip the header line
        for line in file:
            parts = line.strip().split()
            node1 = int(parts[0])
            node2 = int(parts[1])
            if node1 not in graph:
                graph[node1] = []
            if node2 not in graph:
                graph[node2] = []
            if node2 not in graph[node1]:
                graph[node1].append(node2)
            if node1 not in graph[node2]:
                graph[node2].append(node1)
    return graph

def add_random_weights(graph):
    weighted_graph = {}
    existing_neighbors = {}
    for node in graph:
        weighted_graph[node] = []
        existing_neighbors[node] = []
    for node in graph:
        neighbors = graph[node]
        for neighbor in neighbors:
            weight = random.randint(1, 1000)
            if(neighbor not in existing_neighbors[node]):
                weighted_graph[node].append((neighbor, weight))
                existing_neighbors[node].append(neighbor)
            if(node not in existing_neighbors[neighbor]):
                weighted_graph[neighbor].append((node, weight))
                existing_neighbors[neighbor].append(node)
    #print(weighted_graph)
    return weighted_graph

def save_weighted_graph_to_file(weighted_graph, file_path):
    with open(file_path, 'w') as file:
        file.write("# FromNodeId  ToNodeId  Weight\n")
        sorted_nodes = sorted(weighted_graph.keys())
        for node in sorted_nodes:
            for neighbor, weight in weighted_graph[node]:
                file.write(f"{node}  {neighbor}  {weight}\n")

def main():
    # Read undirected graph from file
    undirected_graph = read_graph_from_file("sorted.txt")
    #print(undirected_graph)
    # Add random weights
    weighted_graph = add_random_weights(undirected_graph)
    #print(weighted_graph)
    # Save weighted graph to file
    save_weighted_graph_to_file(weighted_graph, "weighted_graph.txt")

if __name__ == "__main__":
    main()

