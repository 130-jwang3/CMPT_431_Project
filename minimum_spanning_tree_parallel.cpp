#include "core/graph.h"
#include "core/utils.h"
#include "core/ThreadPool.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <utility>
#include <limits>

// Mutex for managing access to the priority queue
std::mutex pq_mutex;

// Structure to represent an edge
struct Edge {
    int src, dest, weight;
    Edge(int s, int d, int w) : src(s), dest(d), weight(w) {}
    bool operator>(const Edge &other) const {
        return weight > other.weight; // Comparison operator for min heap
    }
};

// Function to compute Minimum Spanning Tree (MST) in parallel
void primMSTParallel(Graph &g, uintE n_threads) {
    ThreadPool pool(n_threads); // ThreadPool object with specified number of threads
    std::vector<bool> inMST(g.numVertices(), false); // Array to track if a vertex is in MST
    std::vector<Edge> mstEdges; // List of edges in MST
    std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> pq; // Priority queue for edges
    inMST[0] = true; // Mark first vertex as visited
    long mstWeight = 0; // Total weight of MST
    timer t1;
    t1.start(); // Start timer

    // Initially, add all edges from the first vertex to the priority queue
    const std::vector<uintV> &initialNeighbors = g.getNeighbors(0);
    for (auto v : initialNeighbors) {
        pq.emplace(0, v, g.getEdgeWeight(0, v)); // Add edges to priority queue
    }

    while (!pq.empty()) {
        std::lock_guard<std::mutex> lock(pq_mutex); // Lock access to priority queue
        Edge minEdge = pq.top(); // Get minimum weight edge
        pq.pop(); // Remove edge from priority queue

        if (!inMST[minEdge.dest]) {
            inMST[minEdge.dest] = true; // Mark vertex as visited
            mstEdges.push_back(minEdge); // Add edge to MST
            mstWeight += minEdge.weight; // Update total weight

            // Add all valid edges from the newly added vertex
            const std::vector<uintV> &newNeighbors = g.getNeighbors(minEdge.dest);
            for (auto v : newNeighbors) {
                if (!inMST[v]) {
                    pq.emplace(minEdge.dest, v, g.getEdgeWeight(minEdge.dest, v)); // Add new edges to priority queue
                }
            }
        }
    }
    
    double total_time = t1.stop(); // Stop timer

    // Output the MST edges to a file
    std::ofstream outFile("./outputs/result_parallel.out");
    if (outFile.is_open()) {
        for (const Edge& edge : mstEdges) {
            outFile << edge.src << " <-> " << edge.dest << " " << edge.weight << std::endl;
        }
        outFile.close();
    } else {
        std::cerr << "Failed to open file for writing: " << "./outputs/result_parallel.out" << std::endl;
    }

    std::cout << "Total weight of MST: " << mstWeight << std::endl;
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
             cxxopts::value<uintE>()->default_value("1")},
            {"inputFile", "Input graph file path",
             cxxopts::value<std::string>()->default_value(
                 "/scratch/input_graphs/roadNet-CA")},
        });

    auto cl_options = options.parse(argc, argv);
    uintE n_threads = cl_options["nThreads"].as<uintE>();
    std::string input_file_path = cl_options["inputFile"].as<std::string>();

    std::cout << std::fixed;
    std::cout << "Number of Threads : " << n_threads << std::endl;

    Graph g;
    std::cout << "Reading graph\n";
    g.readGraphFromBinary<int>(input_file_path); // Read graph from binary file
    std::cout << "Created graph\n";

    // Compute MST in parallel
    primMSTParallel(g, n_threads);

    return 0;
}
