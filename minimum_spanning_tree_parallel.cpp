#include "core/graph.h"
#include "core/utils.h"
#include "core/ThreadPool.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <fstream>
#include <utility>
#include <limits>

// Mutex for managing access to shared resources
std::mutex mtx;

struct Edge {
    int src, dest, weight;
    Edge(int s, int d, int w) : src(s), dest(d), weight(w) {}
    bool operator<(const Edge &other) const {
        return weight < other.weight;
    }
};

void findLocalMinEdge(Graph &g, const std::vector<bool> &inMST, int start, int end, Edge &localMinEdge) {
    WeightType localMinWeight = std::numeric_limits<WeightType>::max();
    int localSrc = -1, localDest = -1;

    for (int i = start; i < end; ++i) {
        if (inMST[i]) {
            const std::vector<uintV> &neighbors = g.getNeighbors(i);
            for (uintV v : neighbors) {
                WeightType weight = g.getEdgeWeight(i, v);
                if (!inMST[v] && weight < localMinWeight) {
                    localMinWeight = weight;
                    localSrc = i;
                    localDest = v;
                }
            }
        }
    }

    localMinEdge = Edge(localSrc, localDest, localMinWeight);
}

void primMSTParallel(Graph &g, uintE n_threads) {
    ThreadPool pool(n_threads);
    std::vector<bool> inMST(g.numVertices(), false);
    std::vector<Edge> mstEdges;
    inMST[0] = true;
    long mstWeight = 0;
    timer t1;
    t1.start();

    for (int count = 1; count < g.numVertices(); ++count) {
        std::vector<Edge> localMins(n_threads, Edge(-1, -1, std::numeric_limits<WeightType>::max()));
        int chunkSize = (g.numVertices() + n_threads - 1) / n_threads;

        std::vector<std::future<void>> futures;
        for (uintE i = 0; i < n_threads; ++i) {
            int start = i * chunkSize;
            int end = std::min((i + 1) * chunkSize, g.numVertices());
            futures.emplace_back(pool.enqueue(findLocalMinEdge, std::ref(g), std::ref(inMST), start, end, std::ref(localMins[i])));
        }

        for (auto &f : futures) {
            f.get();
        }

        // Reduce phase: Find the overall minimum edge
        Edge minEdge = *std::min_element(localMins.begin(), localMins.end());

        if (minEdge.src != -1) {
            inMST[minEdge.dest] = true;
            mstEdges.push_back(minEdge);
            mstWeight += minEdge.weight;
            // std::cout << "Edge (" << minEdge.src << " - " << minEdge.dest << ") with weight " << minEdge.weight << " added to MST\n";
        }
    }
    double total_time = t1.stop();

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
             cxxopts::value<uintE>()->default_value(DEFAULT_NUMBER_OF_THREADS)},
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
    g.readGraphFromBinary<int>(input_file_path);
    std::cout << "Created graph\n";
    primMSTParallel(g, n_threads);

    return 0;
}
