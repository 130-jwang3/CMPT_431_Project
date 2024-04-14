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

// Function to compute Minimum Spanning Tree (MST) serially
void primMSTSerial(Graph &g)
{
    timer t1;
    t1.start();
    uintV numVertices = g.numVertices_; // Number of vertices in the graph
    std::vector<bool> inMST(numVertices, false); // Track if a vertex is in MST
    std::vector<WeightType> key(numVertices, MAX_WEIGHT); // Key values used to pick minimum weight edge
    std::vector<uintV> parent(numVertices, -1); // Parent array to store the MST

    // Initialize total weight of MST
    long totalWeight = 0;

    // Priority queue to store edges ordered by weight
    std::priority_queue<std::pair<WeightType, uintV>,
                        std::vector<std::pair<WeightType, uintV>>,
                        std::greater<std::pair<WeightType, uintV>>>
        minHeap;

    // Initialize starting vertex
    key[0] = 0;
    minHeap.push({0, 0});

    // Main loop to construct MST
    while (!minHeap.empty())
    {
        uintV u = minHeap.top().second; // Get vertex with smallest key
        minHeap.pop(); // Remove vertex from priority queue

        if (inMST[u])
            continue; // Skip if vertex is already in MST

        inMST[u] = true; // Mark vertex as visited

        // Total weight update
        if (u != 0) // Skip the first vertex as it does not contribute to the MST weight
        {
            totalWeight += key[u];
        }

        // Explore neighbors of u
        const auto &neighbors = g.getNeighbors(u); // Get weight of edge (u, v)
        for (const auto &v : neighbors)
        {
            WeightType weight = g.getEdgeWeight(u, v);

            // Update key value and parent if weight is smaller
            if (!inMST[v] && weight < key[v])
            {
                key[v] = weight;
                parent[v] = u;
                minHeap.push({key[v], v}); // Update minHeap with new key value for v
            }
        }
    }

    // Write MST edges to output file
    std::ofstream outFile("./outputs/result_serial.out");
    if (!outFile)
    {
        std::cerr << "Error opening output file." << std::endl;
        return;
    }

    for (size_t i = 1; i < numVertices; ++i)
    {
        if (parent[i] != -1)
        {
            outFile << parent[i] << " <-> " << i << " " << key[i] << std::endl;
        }
    }
    
    double total_time = t1.stop(); // Stop timer

    // Output the total weight of the MST
    std::cout << "Total weight of MST: " << totalWeight << std::endl;
    std::cout << "Total time taken: " << total_time << std::endl;
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
    g.readGraphFromBinary<int>(input_file_path); // Read graph from binary file
    std::cout << "Created graph\n";

    // Compute MST serially
    primMSTSerial(g);

    return 0;
}
