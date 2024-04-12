#include "core/graph.h"
#include "core/utils.h"
#include <climits>
#include <mpi.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <fstream>
#include <sstream>

#define INF INT_MAX

struct Edge {
    int weight;
    int vertex1;
    int vertex2;

    Edge() : weight(INF), vertex1(-1), vertex2(-1) {}
    Edge(int w, int v1, int v2) : weight(w), vertex1(v1), vertex2(v2) {}

    bool operator<(const Edge& e) const {
        return weight < e.weight;
    }
};

class UnionFind {
public:
    std::vector<int> parent;
    std::vector<int> rank;

    UnionFind(int size) : parent(size), rank(size, 0) {
        std::iota(parent.begin(), parent.end(), 0);
    }

    int find(int u) {
        if (parent[u] != u)
            parent[u] = find(parent[u]);  // Path compression
        return parent[u];
    }

    bool unionSet(int u, int v) {
        int rootU = find(u);
        int rootV = find(v);
        if (rootU != rootV) {
            if (rank[rootU] > rank[rootV])
                parent[rootV] = rootU;
            else if (rank[rootU] < rank[rootV])
                parent[rootU] = rootV;
            else {
                parent[rootV] = rootU;
                rank[rootU]++;
            }
            return true;
        }
        return false;
    }
};

void computeMST(const std::vector<Edge>& edges, int V) {
    long mst_weight = 0;
    UnionFind uf(V);
    std::vector<Edge> mst;

    for (const auto& e : edges) {
        if (uf.unionSet(e.vertex1, e.vertex2)) {
            mst.push_back(e);
            mst_weight+=e.weight;
            if (mst.size() == V - 1) break;
        }
    }

    // std::cout << "Edges in the MST:" << std::endl;
    // for (const auto& e : mst) {
    //     std::cout << e.vertex1 << " - " << e.vertex2 << " with weight " << e.weight << std::endl;
    //     mst_weight+=e.weight;
    // }
    std::cout << "MST weight is : " << mst_weight << std::endl;
}

int main(int argc, char** argv) {
    cxxopts::Options options(
        "minimum_weight_spanning_tree",
        "Calculate MST using serial, parallel and MPI execution");
    options.add_options(
        "",
        {
            {"inputFile", "Input graph file path",
             cxxopts::value<std::string>()->default_value(
                 "./testing_graphs/filtered_graph_1000")},
        });

    auto cl_options = options.parse(argc, argv);
    std::string input_file_path = cl_options["inputFile"].as<std::string>();

    // std::cout << "Input File Path: " << input_file_path << std::endl;
    MPI_Init(&argc, &argv);
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    //const int V = 9; // Number of vertices
    std::vector<Edge> all_edges;
    int v=0;  //vertex counter

    
    //The leader process is responsible for reading the graph
    if (world_rank == 0) {
        std::ifstream file(input_file_path);
        if (!file.is_open()) {
            std::cerr << "Error opening file." << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        std::string line;
        // Skip the header line
        std::getline(file, line);

        int max_vertex_id = -1; // Initialize max_vertex_id
        while (std::getline(file, line)) {
            int from, to, weight;
            std::istringstream iss(line);
            if (!(iss >> from >> to >> weight)) {
                std::cerr << "Error reading line from file." << std::endl;
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            max_vertex_id = std::max({max_vertex_id, from, to}); // Update max_vertex_id
            all_edges.emplace_back(weight, from, to);
        }
        v = max_vertex_id + 1;
    }

    timer t1;
    t1.start();

    //the leader process splits data to worker processes

    int total_edges = all_edges.size();
    MPI_Bcast(&total_edges, 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<int> sendcounts(world_size, total_edges / world_size);
    std::vector<int> displs(world_size, 0);

    for (int i = 0; i < world_size; ++i) {
        if (i < total_edges % world_size) {
            sendcounts[i]++;
        }
    }

    std::partial_sum(sendcounts.begin(), sendcounts.end() - 1, displs.begin() + 1);
    std::transform(sendcounts.begin(), sendcounts.end(), sendcounts.begin(), [](int x) { return x * sizeof(Edge); });
    std::transform(displs.begin(), displs.end(), displs.begin(), [](int x) { return x * sizeof(Edge); });

    std::vector<Edge> local_edges(sendcounts[world_rank] / sizeof(Edge));

    MPI_Scatterv(all_edges.data(), sendcounts.data(), displs.data(), MPI_BYTE,
                 local_edges.data(), sendcounts[world_rank], MPI_BYTE, 0, MPI_COMM_WORLD);

    std::sort(local_edges.begin(), local_edges.end());

    std::vector<Edge> gathered_edges(total_edges);
    MPI_Gatherv(local_edges.data(), sendcounts[world_rank], MPI_BYTE,
                gathered_edges.data(), sendcounts.data(), displs.data(), MPI_BYTE, 0, MPI_COMM_WORLD);


    //the root process merges n sorted vectors from work processes
    if (world_rank == 0) {
        std::vector<Edge> fully_sorted_edges;
        fully_sorted_edges.reserve(total_edges); // Reserve space to avoid reallocations

        std::vector<int> current_index(world_size, 0);
        bool done = false;

        while (!done) {
            Edge min_edge;
            min_edge.weight = INF;

            int min_index = -1;
            for (int i = 0; i < world_size; ++i) {
                int global_index = displs[i] / sizeof(Edge) + current_index[i];
                if (current_index[i] < sendcounts[i] / sizeof(Edge) && gathered_edges[global_index].weight < min_edge.weight) {
                    min_edge = gathered_edges[global_index];
                    min_index = i;
                }
            }

            if (min_edge.weight == INF) {  // All vectors are exhausted
                done = true;
            } else {
                fully_sorted_edges.push_back(min_edge);
                current_index[min_index]++;
            }
        }

        // std::cout << "Fully Sorted Edges:" << std::endl;
        // for (const auto& edge : fully_sorted_edges) {
        //     std::cout << edge.vertex1 << " - " << edge.vertex2 << " with weight " << edge.weight << std::endl;
        // }

        computeMST(fully_sorted_edges, v);
        double total_time = t1.stop();
        std::cout << "Total time taken: " << total_time << std::endl;
    }
    MPI_Finalize();
    return 0;
}