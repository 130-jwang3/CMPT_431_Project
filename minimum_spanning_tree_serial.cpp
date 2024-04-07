#include "core/graph.h"
#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <queue>
#include <vector>
#include <fstream>
#include <utility>
#include <limits>

void primMSTSerial(Graph &g)
{
    uintV numVertices = g.numVertices_;
    std::cout << numVertices << std::endl;
    std::vector<bool> inMST(numVertices, false);          // Tracks if a vertex is in MST
    std::vector<WeightType> key(numVertices, MAX_WEIGHT); // Key values used to pick minimum weight edge
    std::vector<uintV> parent(numVertices, -1);           // Stores the MST
    std::cout << "here" << std::endl;
    // Min-heap to store vertices based on key value
    std::priority_queue<std::pair<WeightType, uintV>,
                        std::vector<std::pair<WeightType, uintV>>,
                        std::greater<std::pair<WeightType, uintV>>>
        minHeap;

    // Initialize the min-heap with the first vertex
    key[0] = 0;
    minHeap.push({0, 0}); // (weight, vertex)
    std::cout << "here" << std::endl;
    while (!minHeap.empty())
    {
        // Extract the vertex with minimum key value
        uintV u = minHeap.top().second;
        minHeap.pop();

        if (inMST[u])
            continue; // Skip if vertex is already included in MST

        inMST[u] = true; // Include vertex in MST

        // Iterate over all vertices to find neighbors of u (since we don't have direct access to neighbors with weights)
        const auto &neighbors = g.getNeighbors(u);
        for (const auto &v : neighbors)
        {
            WeightType weight = g.getEdgeWeight(u, v); // Directly access the weight from the adjacency matrix

            // If v is not in MST and weight of (u,v) is smaller than the current key of v
            if (!inMST[v] && weight < key[v])
            {
                key[v] = weight;
                parent[v] = u;
                minHeap.push({key[v], v});
            }
        }
    }
    std::cout << "here" << std::endl;

    // Output the MST to a file
    std::ofstream outFile("./outputs/weighted_graph.out");
    if (!outFile)
    {
        std::cerr << "Error opening output file." << std::endl;
        return;
    }

    for (size_t i = 1; i < numVertices; ++i)
    {
        if (parent[i] != -1)
        { // Check if there is a parent
            outFile << parent[i] << " <-> " << i << " " << key[i] << std::endl;
        }
    }
}
int main(int argc, char *argv[])
{
    cxxopts::Options options(
        "minimum_weight_spanning_tree",
        "Calculate MST using serial, parallel and MPI execution");
    options.add_options(
        "",
        {
            {"nThreads", "Number of Threads",
             cxxopts::value<uintE>()->default_value(DEFAULT_NUMBER_OF_THREADS)},
            {"inputFile", "Input graph file path",
             cxxopts::value<std::string>()->default_value(
                 "/scratch/testing_graphs/weighted_graph")},
        });

    auto cl_options = options.parse(argc, argv);
    uintE n_threads = cl_options["nThreads"].as<uintE>();
    std::string input_file_path = cl_options["inputFile"].as<std::string>();

    std::cout << std::fixed;
    std::cout << "Number of Threads : " << n_threads << std::endl;
    std::cout << "Input File Path: " << input_file_path << std::endl;

    Graph g;
    std::cout << "Reading graph\n";
    g.readGraphFromBinary<int>(input_file_path);
    std::cout << g.numVertices_ << std::endl;
    std::cout << "Created graph\n";
    primMSTSerial(g);

    return 0;
}
