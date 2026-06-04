# M/M/1 Queuing System Implementation in C
This is a simple implementation of the M/M/1 queuing system, written in the C programming language.

## Building and Running
This project uses CMake as the build system. If you don't have Cmake installed, skip to [Building manually with GCC](#building-manually-using-gcc).  

To build the project, run the following:

```bash
cmake -S. build
cmake --build build
```

This will build the project. The system has two modes of running the simulation:
1. Generating a new set of interval arrival and service time for N customers,
2. Reading and input CSV file with previously generated IATs and service times.

You can run the first mode using:
```bash
./main [N]
```

Where `N` is the number of customers you want to add to the system, for example `./main 10` for 10 customers. This will also create a `input.csv` file in the current working directory containing the generated IATs and service times.

To run the second mode using an input file:

```bash
./main -f input.csv
```

Where `input.csv` is a CSV file generated from running the first mode of the program.

## Building manually using GCC
If you prefer to compile the code manually using GCC, or if you don't have CMake installed, you can run the following:

```bash
gcc main.c system/system.c -I ./system -o main -lm
```