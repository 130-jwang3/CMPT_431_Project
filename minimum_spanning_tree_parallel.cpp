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

std::mutex my_mutex; 

void HelperMSTParallel(Graph &g, int startVertex, int endVertex, std::vector<bool> &inMST, std::vector<WeightType> &key, std::vector<uintV> &parent, std::mutex &mtx, std::vector<uintV> &colors, int thread_id, std::priority_queue<std::pair<WeightType, uintV>,
                      std::vector<std::pair<WeightType, uintV>>,
                      std::greater<std::pair<WeightType, uintV>>> & minHeap)
{
  uintV numVertices = g.numVertices_;
  // Min-heap to store vertices based on key value
  
  // Initialize the min-heap with the first vertex
  key[startVertex] = 0; //NOTE: Change to 0 leads to full tree for all threads
  minHeap.push({0, startVertex}); // (weight, vertex) 

  while (!minHeap.empty())
  {
    // Extract the vertex with minimum key value
    uintV u;
    {
        std::lock_guard<std::mutex> lock(mtx);
        u = minHeap.top().second;
        minHeap.pop();
    }

    // std::cout << "u: " << u << std:: endl;
    if (u < startVertex && u > endVertex) 
      continue;

    if (inMST[u])
      continue; // Skip if vertex is already included in MST

    {
      std::lock_guard<std::mutex> lock(mtx);
      inMST[u] = true; // Include vertex in MST
    }

    // Iterate over all vertices to find neighbors of u (since we don't have direct access to neighbors with weights)
    const auto &neighbors = g.getNeighbors(u);
    for (const auto &v : neighbors)
    {
      if (v >= startVertex && v < endVertex) 
      {
        WeightType weight = g.getEdgeWeight(u, v); // Directly access the weight from the adjacency matrix

        // If v is not in MST and weight of (u,v) is smaller than the current key of v
        {
          
          if (!inMST[v] && weight < key[v])
          {
            std::lock_guard<std::mutex> lock(mtx);

            if (weight < key[v]){
              key[v] = weight;
              parent[v] = u;
              minHeap.push({key[v], v});
            }
          }
        }
      }
    }
  }
}


void primMSTParallel(Graph &g, uintE n_threads) {
  // to be implemented
  timer thread_timer;

  uintV numVertices = g.numVertices_;
  // std::cout << numVertices << std::endl;
  std::vector<bool> inMST(numVertices, false);          // Tracks if a vertex is in MST
  std::vector<WeightType> key(numVertices, MAX_WEIGHT); // Key values used to pick minimum weight edge
  std::vector<uintV> parent(numVertices, -1);           // Stores the MST
  std::vector<uintV> colors(numVertices, -1);

  std::vector<std::priority_queue<std::pair<WeightType, uintV>,
                      std::vector<std::pair<WeightType, uintV>>,
                      std::greater<std::pair<WeightType, uintV>>>>
      minHeap(n_threads);


  std::vector<double> time_taken_vector(n_threads, 0.0);
  std::mutex mtx;

  int chunk_size = (numVertices + n_threads - 1) / n_threads;
  std::vector<std::thread> threads;

  for (int tid = 0; tid < n_threads; tid++){ 
    int start = tid * chunk_size;
    int end = std::min((tid + 1) * chunk_size, numVertices); // to account for the last one
    std::cout << "start: " << start << std:: endl;
    std::cout << "end: " << end << std:: endl;

    // threads.emplace_back(HelperMSTParallel, std::ref(g), start, end, std::ref(inMST), std::ref(key), std::ref(parent), std::ref(mtx));
    threads.emplace_back(HelperMSTParallel, std::ref(g), start, end, std::ref(inMST), std::ref(key), std::ref(parent), std::ref(mtx), std::ref(colors), tid, std::ref(minHeap[tid]));
  }

  for (auto& thread : threads) {
    thread.join();
  }

  for (size_t i = 0; i < numVertices; ++i)
  {
    if (parent[i] != -1)
    { // Check if there is a parent
      std::cout << parent[i] << " <-> " << i << " " << key[i] << std::endl;
    }
  }

  //=== Merge
  // std::priority_queue<std::pair<WeightType, uintV>,
  //                     std::vector<std::pair<WeightType, uintV>>,
  //                     std::greater<std::pair<WeightType, uintV>>>
  //     fullMinHeap;

  // fullMinHeap.push({0, 0}); // (weight, vertex) 

  // for (int tid = 0; tid < n_threads; tid++){
  //   while (!minHeap[tid].empty()) {
  //     fullMinHeap.push(minHeap[tid].top());
  //     minHeap[tid].pop();
  //   }
  // }

  // for (int tid = 0; tid < n_threads; tid++){
  //   while (!minHeap[tid].empty()) {
  //     std::cout << minHeap[tid].top().first << " - " << minHeap[tid].top().second << std::endl;
  //     minHeap[tid].pop();
  //   }
  // }
  

}

int main(int argc, char *argv[]) {
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
