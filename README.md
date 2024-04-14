## CMPT 431 Project: Minimum-weight Spanning Tree (MST)

## Overview
The Minimum-weight Spanning Tree (MST) of a graph is the spanning tree with the minimum total edge weight. This projects implements the MST using three different algorithms: Serial, Parallel using C++ threads, and Distributed using MPI.

## Steps to run the program 

### Obtain the binary files
1. Unzip the file (`proj.tar`): `tar xvzf proj.tar.gz`
2. Generate the binary files: `make`

### Generate input files:
1. Go to the `testing_graphs` directory: `cd testing_graphs`
2. Generate a graph of size 1000: `python3 generate_graph.py 1000` (takes approx. 20 sec)
3. Generate a graph of size 10000: `python3 generate_graph.py 10000` (takes approx. 50 sec )
4. Generate a graph of size n: `python3 generate_graph.py {n}` (may take about 10 minutes when n=40000)

### Run the Serial, Parallel, and Distributed implementations
1. Go back to the original directory: `cd ..`
2. Run the Serial implementation: `./minimum_spanning_tree_serial --inputFile absolute_path_of_generated_input_file.txt`
3. Run the Parallel implementation: `./minimum_spanning_tree_parallel --nThreads 4 --inputFile absolute_path_of_generated_input_file.txt`
    - **Note:** The number of threads can be changed by setting `nThread` to any reasonable positive integer (1,2,3,4,5...) suitable for your running environment.
4. Run the MPI implementation: `mpiexec -n 4 ./minimum_spanning_tree_mpi --inputFile absolute_path_of_generated_input_file.txt`
    - **Note:** The number of processes can be changed by setting `n` to any reasonable positive integer (1,2,3,4,5...) suitable for your running environment.

### Check outputs
1. Go to the `outputs` directory: `cd outputs`
2. View results: `cat output.out`
