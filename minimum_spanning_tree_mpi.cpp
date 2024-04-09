#include "core/graph.h"
#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <fstream>

using namespace std;

#define V 9 // Number of vertices
#define INF INT_MAX

void primMSTMPI(int rank, int size, int graph[V][V]) {
    int vertices_per_process = V / size;
    int start_vertex = rank * vertices_per_process;
    int end_vertex = (rank + 1) * vertices_per_process - 1;

    if (rank == size - 1) { // Adjust for the last process to handle all remaining vertices
        end_vertex = V - 1;
    }

    int local_key[V], local_parent[V];
    bool local_mstSet[V];

    for (int i = 0; i < V; i++) {
        local_key[i] = INF;
        local_mstSet[i] = false;
        local_parent[i] = -1;
    }

    if (rank == 0) {
        local_key[0] = 0; // Start with vertex 0 for process 0
    }

    for (int count = 0; count < V; count++) {
        int global_min_key = INF, local_min_index = -1;

        // Find local minimum key vertex
        for (int v = start_vertex; v <= end_vertex; v++) {
            if (!local_mstSet[v] && local_key[v] < global_min_key) {
                global_min_key = local_key[v];
                local_min_index = v;
            }
        }

        struct { int key; int index; } local_min = {global_min_key, local_min_index}, global_min;

        // Existing MPI_Allreduce call to find the global minimum
        MPI_Allreduce(&local_min, &global_min, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);

        int global_min_index = global_min.index;

        // Synchronize local_mstSet across all processes
        bool is_min_in_set = (global_min_index >= start_vertex && global_min_index <= end_vertex);
        // Broadcast the decision of including the global minimum vertex in MST
        MPI_Bcast(&is_min_in_set, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

        if (is_min_in_set) {
            local_mstSet[global_min_index] = true;
        }

        // Synchronize local_mstSet across all processes to ensure a consistent view
        MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, local_mstSet, V, MPI_C_BOOL, MPI_COMM_WORLD);

        //update local_key and local_parent based on the selected global minimum
        for (int v = 0; v < V; v++) {
            if (graph[global_min_index][v] != INF && !local_mstSet[v] && graph[global_min_index][v] < local_key[v]) {
                local_parent[v] = global_min_index;
                local_key[v] = graph[global_min_index][v];
            }
        }
    }

    // Synchronize the local_parent array before printing MST
    int *gathered_parents = new int[V]; 

    MPI_Allgather(local_parent + start_vertex, vertices_per_process, MPI_INT, 
                  gathered_parents, vertices_per_process, MPI_INT, 
                  MPI_COMM_WORLD);

    // Update local_parent with the gathered values
    memcpy(local_parent, gathered_parents, V * sizeof(int));
    delete[] gathered_parents;

    // Print the constructed MST
    if (rank == 0) {
        cout << "Minimum Spanning Tree (Prim's Algorithm):" << endl;
        for (int i = 1; i < V; i++) {
            cout << local_parent[i] << " - " << i << " with weight " << graph[i][local_parent[i]] << endl;
        }
    }
}

int main(int argc, char *argv[]){
    int world_rank, world_size;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_rank == 0){
        std::cout << "Number of processes : " << world_size << "\n";
    }

    // Dynamically initialize graph matrix
    int graph[V][V];

    // Initialize all values to INF
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            graph[i][j] = INF;
        }
    }

    // Read graph data from file
    string line;
    ifstream graphFile("./testing_graphs/mini_graphs.txt"); // Make sure to specify the correct path to your file

    if (graphFile.is_open()) {
        // Skip the first line (header)
        getline(graphFile, line);

        while (getline(graphFile, line)) {
            int fromNode, toNode, weight;
            sscanf(line.c_str(), "%d\t%d\t%d", &fromNode, &toNode, &weight);
            graph[fromNode][toNode] = weight;
            graph[toNode][fromNode] = weight; // Assuming the graph is undirected
        }
        graphFile.close();
    } else {
        std::cout << "Unable to open file" << std::endl;
        MPI_Finalize();
        return -1; // Exit if file cannot be opened
    }

    primMSTMPI(world_rank, world_size, graph);
    MPI_Finalize();
    return 0;
}
