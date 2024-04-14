## CMPT 431 PROJECT

Step1: unzip the file (proj.tar) and generate the binary files:
    tar xvzf proj.tar.gz
    make

Step2: generate input file:
    (1) change directory to testing_graphs: cd testing_graphs 
    (2) generate a graph of size 1000: python3 generate_graph.py 1000 (takes approx. 20 sec)
        generate a graph of size 10000: python3 generate_graph.py 10000 (takes approx. 50 sec )
        generate a graph of size n: python3 generate_graph.py {n} (may take about 10 minutes when n=40000)

Step3: run the serial code:
    (1) go back to a directory from step2: cd ..
    (2) run the serial code: ./minimum_spanning_tree_serial --inputFile absolute_path_of_one_input_file_that_you_generated_in_step2.txt
    (3) run the parallel code: ./minimum_spanning_tree_parallel --nThreads 4 --inputFile absolute_path_of_one_input_file_that_you_generated_in_step2.txt
        you can change the nThread value to any reasonable positive integers (1,2,3,4,5...) suitable for your running environment to represent the number of threads
    (4) run the mpi code: mpiexec -n 4 ./minimum_spanning_tree_mpi --inputFile absolute_path_of_one_input_file_that_you_generated_in_step2.txt
        you can change the n value to any reasonable positive integers (1,2,3,4,5...) suitable for your running environment to represent the number of processes

Step4: Check outputs
    (1) change directory to outputs: cd outputs
    (2) cat output.out
