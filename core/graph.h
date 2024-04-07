#ifndef GRAPH_H
#define GRAPH_H
#include "quick_sort.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <limits>
#include <string>
#include <algorithm>
#include <sstream>

class Graph
{
private:
  std::vector<std::vector<WeightType>> adjMatrix_;
  std::vector<std::vector<uintV>> neighbors_;

public:
  uintV numVertices_;
  Graph() = default;

  void initializeGraph(uintV numVertices)
  {
    numVertices_ = numVertices;
    adjMatrix_.assign(numVertices, std::vector<WeightType>(numVertices, MAX_WEIGHT));
    neighbors_.assign(numVertices, std::vector<uintV>());

    for (uintV i = 0; i < numVertices_; ++i)
    {
      adjMatrix_[i][i] = 0; // Initialize self-loops to 0
    }
  }

  template <class T>
  void readGraphFromBinary(const std::string &inputFilePath) {
    std::ifstream inputFile(inputFilePath);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening input file: " << inputFilePath << std::endl;
        throw std::runtime_error("Failed to open input file");
    }

    std::string line;
    uintV src, dest, maxVertexId = 0;
    WeightType weight; // Assuming WeightType is known and appropriate for your graph

    // Temporary storage to avoid resizing vectors multiple times
    std::vector<std::tuple<uintV, uintV, WeightType>> edges;

    // Skip the header line
    std::getline(inputFile, line);

    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        if (!(iss >> src >> dest >> weight)) {
            // Handle error or ignore malformed line
            continue;
        }

        edges.emplace_back(src, dest, weight);
        maxVertexId = std::max({maxVertexId, src, dest});
    }

    // Initialize the Graph structure based on the maxVertexId
    numVertices_ = maxVertexId + 1;
    adjMatrix_.assign(numVertices_, std::vector<WeightType>(numVertices_, MAX_WEIGHT));
    neighbors_.assign(numVertices_, std::vector<uintV>());

    for (size_t i = 0; i < numVertices_; ++i) {
        adjMatrix_[i][i] = 0; // Initialize self-loops to 0, if necessary
    }

    // Add edges to the graph
    for (const auto& edge : edges) {
        this->addEdge(std::get<0>(edge), std::get<1>(edge), std::get<2>(edge));
    }
}

  void addEdge(uintV src, uintV dest, WeightType weight)
  {
    if (src >= numVertices_ || dest >= numVertices_)
    {
      std::cerr << "Error: Vertex index out of bounds." << std::endl;
      return;
    }
    adjMatrix_[src][dest] = weight;
    adjMatrix_[dest][src] = weight;
    neighbors_[src].push_back(dest);
    neighbors_[dest].push_back(src);
  }

  const std::vector<uintV> &getNeighbors(uintV node) const
  {
    if (node >= numVertices_)
    {
      std::cerr << "Error: Vertex index out of bounds." << std::endl;
      static const std::vector<uintV> empty;
      return empty; // Safely return an empty vector for out of bounds access
    }
    return neighbors_[node];
  }

  uintV numVertices() const
  {
    return numVertices_;
  }

  WeightType getEdgeWeight(uintV src, uintV dest) const
  {
    if (src >= numVertices_ || dest >= numVertices_)
    {
      std::cerr << "Error: Vertex index out of bounds." << std::endl;
      return MAX_WEIGHT;
    }
    return adjMatrix_[src][dest];
  }
};
#endif