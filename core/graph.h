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

class Graph
{
private:
    std::vector<std::vector<WeightType>> adjMatrix_;
    std::vector<std::vector<uintV>> neighbors_;
    size_t numVertices_;

public:
    Graph() = default;

    Graph(size_t numVertices) : numVertices_(numVertices), adjMatrix_(numVertices, std::vector<WeightType>(numVertices, MAX_WEIGHT)), neighbors_(numVertices) {
        for (size_t i = 0; i < numVertices_; ++i) {
            adjMatrix_[i][i] = 0; // Initialize self-loops to 0
        }
    }

    template <class T>
    static Graph readGraphFromBinary(const std::string& inputFilePath) {
        std::ifstream inputFile(inputFilePath, std::ios::binary);
        if (!inputFile.is_open()) {
            std::cerr << "Error opening input file: " << inputFilePath << std::endl;
            exit(1); // or handle error differently
        }

        uintV src, dest;
        T weight;
        uintV maxVertexId = 0;

        // Find the max vertex ID (assuming the file's cursor is at the beginning)
        while (inputFile.read(reinterpret_cast<char*>(&src), sizeof(src))
               && inputFile.read(reinterpret_cast<char*>(&dest), sizeof(dest))
               && inputFile.read(reinterpret_cast<char*>(&weight), sizeof(weight))) {
            maxVertexId = std::max({maxVertexId, src, dest});
        }

        Graph g(maxVertexId + 1); // Create a Graph with vertices numbered from 0 to maxVertexId

        // Reset to read the file again
        inputFile.clear(); // Clear eof and fail flags
        inputFile.seekg(0, std::ios::beg);

        // Second pass to actually read and add the edges
        while (inputFile.read(reinterpret_cast<char*>(&src), sizeof(src))
               && inputFile.read(reinterpret_cast<char*>(&dest), sizeof(dest))
               && inputFile.read(reinterpret_cast<char*>(&weight), sizeof(weight))) {
            // Convert T weight to uint32_t if necessary or adjust Graph to template class
            g.addEdge(src, dest, static_cast<WeightType>(weight));
        }

        return g;
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

    size_t numVertices() const
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

    std::vector<Graph> partition(int numPartitions) const
    {
        std::vector<Graph> partitions;
        size_t partitionSize = numVertices_ / numPartitions;
        size_t remainder = numVertices_ % numPartitions;

        size_t start = 0;
        for (int i = 0; i < numPartitions; ++i)
        {
            size_t end = start + partitionSize + (i < remainder ? 1 : 0);
            Graph subGraph(end - start);

            for (size_t j = start; j < end; ++j)
            {
                for (auto neighbor : neighbors_[j])
                {
                    if (neighbor >= start && neighbor < end)
                    {
                        WeightType weight = adjMatrix_[j][neighbor];
                        subGraph.addEdge(j - start, neighbor - start, weight);
                    }
                }
            }

            partitions.push_back(subGraph);
            start = end;
        }

        return partitions;
    }

};
#endif