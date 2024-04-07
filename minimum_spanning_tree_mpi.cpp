#include "core/graph.h"
#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <mpi.h>

void primMSTMPI(int world_rank, int world_size, Graph &g)
{
    // to be implemented
}

int main(int argc, char *argv[])
{
    int world_rank, world_size;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Initialize command line arguments
    cxxopts::Options options(
        "minimum_weight_spanning_tree",
        "Calculate MST using serial, parallel and MPI execution");
    options.add_options(
        "custom",
        {
            {"inputFile", "Input graph file path",
             cxxopts::value<std::string>()->default_value(
                 "/scratch/input_graphs/roadNet-CA")},
        });
    auto cl_options = options.parse(argc, argv);

    std::string input_file_path = cl_options["inputFile"].as<std::string>();

    Graph g = NULL;

    if (world_rank == 0)
    {
        std::cout << "Number of processes : " << world_size << "\n";

        std::cout << "Reading graph\n";
        g.readGraphFromBinary<int>(input_file_path);
        std::cout << "Created graph\n";
    }

    primMSTMPI(world_rank, world_size, g);
    MPI_Finalize();
    return 0;
}
