/*  Project: merge-splitting sort, project for PRL course at FIT BUT
    Author: Martin Krajnak
*/

#include <mpi.h>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>
#include <climits>

using namespace std;
#define TAG 0

int next_left(int i) {
  return 2*(i+1);
}

int next_right(int i) {
  return 2*(i+1)+1;
}

int main(int argc, char **argv) {
  int numprocs;               // number of cpus obtained from mpi
  int myid;                   // cpu identifier
  MPI_Status stat;

  MPI_Init(&argc,&argv);                          // MPI init
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);       // obtain numprocs
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);           // obtain of of each proc

  if (myid == 0) {
    printf("%s\n",argv[1]);
    size_t n = strlen(argv[1])+1;
    char odd[n];
    char even[n];
    char str[n];
    size_t even_counter = 0;
    size_t odd_counter = 0;

    for (size_t i = 1; i <n ; i++) {
      str[i] = argv[1][i-1];
      even[i] = even_counter = even_counter+2;
      odd[i] = odd_counter = odd_counter+2;
      printf("Odd:%2d\tEven:%2d\tC:%c\n",odd[i],even[i],str[i]);
    }

  }
  MPI_Finalize();
  return 0;
 }
