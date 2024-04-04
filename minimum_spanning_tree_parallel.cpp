#include "core/graph.h"
#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>

void primMSTParallel(Graph &g) {
    // to be implemented
}

int main(int argc, char *argv[]) {
  cxxopts::Options options(
      "minimum_weight_spanning_tree",
      "Calculate MST using serial, parallel and MPI execution");
  options.add_options(
      "",
      {
          {"nThreads", "Number of Threads",
           cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_THREADS)},
          {"inputFile", "Input graph file path",
           cxxopts::value<std::string>()->default_value(
               "/scratch/input_graphs/roadNet-CA")},
      });

  auto cl_options = options.parse(argc, argv);
  uint n_threads = cl_options["nThreads"].as<uint>();
  std::string input_file_path = cl_options["inputFile"].as<std::string>();

  std::cout << std::fixed;
  std::cout << "Number of Threads : " << n_threads << std::endl;

  Graph g;
  std::cout << "Reading graph\n";
  g.readGraphFromBinary<int>(input_file_path);
  std::cout << "Created graph\n";
  primMSTParallel(g);

  return 0;
}
