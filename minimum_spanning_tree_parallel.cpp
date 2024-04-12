#include "core/graph.h"
#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <fstream>
#include <utility>
#include <limits>
#include <atomic>
#include <functional>
#include <tuple>
#include <set>

std::mutex my_mutex;

class DisjointSet {
public:
    std::vector<uintV> parent, rank;

    DisjointSet(uintV n): parent(n), rank(n, 0) {
        for (uintV i = 0; i < n; ++i) {
            parent[i] = i;
        }
    }

    int find(uintV x) {
        if (parent[x] != x) {
            parent[x] = find(parent[x]); // Path compression
        }
        return parent[x];
    }

    void merge(uintV x, uintV y) {
        x = find(x);
        y = find(y);
        if (x == y) return;

        if (rank[x] < rank[y]) {
            parent[x] = y;
        } else if (rank[x] > rank[y]) {
            parent[y] = x;
        } else {
            parent[y] = x;
            rank[x]++;
        }
    }
};

std::set<std::tuple<uintV, uintV, WeightType>> primMSTParallel(Graph &g, unsigned int n_threads) {
    std::vector<WeightType> minEdgeWeight(g.numVertices());
    std::vector<uintV> minEdgeTo(g.numVertices(), std::numeric_limits<uintV>::max());
    std::mutex graphMutex;
    std::set<std::tuple<uintV, uintV, WeightType>> mstEdges;
    DisjointSet sets(g.numVertices());

    CustomBarrier my_barrier(n_threads);

    bool finished = false;
    while (!finished) {
        std::fill(minEdgeWeight.begin(), minEdgeWeight.end(), MAX_WEIGHT);

        for (uintV i = 0; i < g.numVertices(); ++i) {
          if (minEdgeTo[i] != std::numeric_limits<uintV>::max() && sets.find(i) == sets.find(minEdgeTo[i])) {
              minEdgeTo[i] = std::numeric_limits<uintV>::max();
          }
        }

        std::vector<std::thread> threads;
        for (unsigned int t = 0; t < n_threads; ++t) {
            threads.emplace_back([&, t]() {
                uintV start = (g.numVertices() / n_threads) * t;
                uintV end = (t == n_threads - 1) ? g.numVertices() : (g.numVertices() / n_threads) * (t + 1);
                std::cout << "start: " << start << "end: " << end << std::endl;

                for (uintV i = start; i < end; ++i) {
                    for (uintV neighbor : g.getNeighbors(i)) {
                        WeightType weight = g.getEdgeWeight(i, neighbor);
                        if (sets.find(i) != sets.find(neighbor) && weight < minEdgeWeight[i]) {
                            std::lock_guard<std::mutex> lock(graphMutex);
                            if (weight < minEdgeWeight[i]) {
                                if(i==2){
                                  // std::cout << "i: " << i << " weight: " << weight << " minEdgeWeight[i]: " << minEdgeWeight[i] << " " << std::endl;
                                }
                                minEdgeWeight[i] = weight;
                                minEdgeTo[i] = neighbor;

                                std::cout << "tid: " << t <<  " i: " << i << " weight: " << weight << " minEdgeWeight[i]: " << minEdgeWeight[i] << " " << std::endl;
                                
                            }
                        }
                    }
                }
            });
        }

        for (auto &thread : threads) {
            thread.join();
        }

        for (uintV i = 0; i < g.numVertices(); ++i) {
          std::cout << "i: " << i << " minEdgeTo[i]: " << minEdgeTo[i] << " minEdgeWeight[i]: " << minEdgeWeight[i] << " " << std::endl;
        }

        // Use DisjointSet to merge components based on min edges found
        for (uintV i = 0; i < g.numVertices(); ++i) {
            if (minEdgeWeight[i] != MAX_WEIGHT && sets.find(i ) != sets.find(minEdgeTo[i])) {
                uintV fromComp = sets.find(i);
                uintV toComp = sets.find(minEdgeTo[i]);
                sets.merge(fromComp, toComp);
                mstEdges.insert({i, minEdgeTo[i], minEdgeWeight[i]});
                // std::cout << "i: " << i << "minEdgeTo[i]: " << minEdgeTo[i] << "minEdgeWeight[i]: " << minEdgeWeight[i] << " " << std::endl;
            }
        }

        // Check if all vertices are in the same component
        finished = true;
        uintV firstComp = sets.find(0);
        for (uintV i = 1; i < g.numVertices(); ++i) {
            if (sets.find(i) != firstComp) {
                finished = false;
                break;
            }
        }

        my_barrier.wait();
    }

    return mstEdges;
}

void writeMSTEdgesToFile(const std::set<std::tuple<uintV, uintV, WeightType>> &mstEdges)
{
  std::ofstream outFile("output.out");
  for (const auto &edge : mstEdges)
  {
    uintV fromNode, toNode;
    WeightType weight;
    std::tie(fromNode, toNode, weight) = edge;
    outFile << fromNode << " <-> " << toNode << " " << weight << "\n";
  }
  outFile.close();
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
  auto mstEdges = primMSTParallel(g, n_threads);
  writeMSTEdgesToFile(mstEdges);

  return 0;
}
