# Parallel_Programming

Using MPI to implement parallel program

* Homework1 :  using tree-structured communication to send and receive data between process

* Homework2 :  1.using multiprocessor to smooth picture 2.Odd enen sort using multiprocessor

* Homework3 :  create a cluster,no code.

* Homework4 :  Modify hw2_1.Using pthread to smooth picture

* Homework5 :  1.Count sort 2.producer and consumer .All using OpenMp.

* Homework6 :  using ant algorithm to find shortest path.

# Compile:
* hw1,hw2: mpiicc  -g  -Wall  -o  hw1  hw1.cpp

* hw4:     g++ -o hw4 hw4.cpp -lpthread

* hw5:     gcc -g -Wall -fopenmp -o hw5_1 hw5_1.c

* hw6:     mpicc  -g -Wall -openmp -o hw6 hw6.c

# Run:
* hw1,hw2: mpiexec -n x ./hw4.out		(x is the number of processor)

* hw4: 	   ./ hw4 x		(x is the number of thread)

* hw5:     ./hw5_1 x (x is the number of thread)

* hw6:     mpiexec -n 4 ./hw6 10 att48_d.txt (4:number of process 10:thread per process)
