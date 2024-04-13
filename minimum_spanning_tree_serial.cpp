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
    timer t1;
    t1.start();
    uintV numVertices = g.numVertices_;
    std::vector<bool> inMST(numVertices, false);
    std::vector<WeightType> key(numVertices, MAX_WEIGHT);
    std::vector<uintV> parent(numVertices, -1);

    // Initialize total weight of MST
    long totalWeight = 0;

    std::priority_queue<std::pair<WeightType, uintV>,
                        std::vector<std::pair<WeightType, uintV>>,
                        std::greater<std::pair<WeightType, uintV>>>
        minHeap;

    key[0] = 0;
    minHeap.push({0, 0});
    while (!minHeap.empty())
    {
        uintV u = minHeap.top().second;
        minHeap.pop();

        if (inMST[u])
            continue;

        inMST[u] = true;

        // Total weight update happens here
        if (u != 0) // Skip for the first vertex as it does not contribute to the MST weight
        {
            totalWeight += key[u];
        }

        const auto &neighbors = g.getNeighbors(u);
        for (const auto &v : neighbors)
        {
            WeightType weight = g.getEdgeWeight(u, v);

            if (!inMST[v] && weight < key[v])
            {
                key[v] = weight;
                parent[v] = u;
                minHeap.push({key[v], v});
            }
        }
    }

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
    
    double total_time = t1.stop();

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
    g.readGraphFromBinary<int>(input_file_path);
    std::cout << "Created graph\n";
    primMSTSerial(g);

    return 0;
}
