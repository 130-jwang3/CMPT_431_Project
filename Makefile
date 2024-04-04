# Conditional definition based on the command line or environment
ifdef USE_INT
CXXFLAGS += -DUSE_INT
endif

# Compiler setup
CXX = g++
MPICXX = mpic++
CXXFLAGS += -std=c++14 -O3 -pthread

# Source and header files
COMMON_HEADERS = core/utils.h core/cxxopts.h core/get_time.h core/graph.h core/quick_sort.h
SERIAL_SRC = minimum_spanning_tree_serial.cpp
PARALLEL_SRC = minimum_spanning_tree_parallel.cpp
MPI_SRC = minimum_spanning_tree_mpi.cpp

# Targets
SERIAL = minimum_spanning_tree_serial
PARALLEL = minimum_spanning_tree_parallel
MPI = minimum_spanning_tree_mpi
ALL = $(SERIAL) $(PARALLEL) $(MPI)

.PHONY: all clean

all: $(ALL)

$(SERIAL): $(SERIAL_SRC) $(COMMON_HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(SERIAL_SRC)

$(PARALLEL): $(PARALLEL_SRC) $(COMMON_HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(PARALLEL_SRC)

$(MPI): $(MPI_SRC) $(COMMON_HEADERS)
	$(MPICXX) $(CXXFLAGS) -o $@ $(MPI_SRC)

clean:
	rm -f *.o *.obj $(ALL)