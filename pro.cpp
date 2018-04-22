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
    size_t n = strlen(argv[1]);
    size_t array[n+1];
    size_t alloc = (n/2)*7;
    char tree[alloc];
    size_t edge_counter = 0;
    size_t vertex_counter = 0;
    for (size_t i = 1; i <= n; i++) {
      array[i] = argv[1][i-1];
    }

    for (size_t i = 1; i <= (size_t)(n/2); i++) {
      tree[vertex_counter] = array[i];
      if ((2*i) <=n ) {
        tree[vertex_counter+1] = (int)(++edge_counter);
        tree[vertex_counter+2] = (int)(++edge_counter);
        tree[vertex_counter+3] = array[2*i];
      }
      if ((2*i+1) <=n ) {
        tree[vertex_counter+4] = (int)(++edge_counter);
        tree[vertex_counter+5] = (int)(++edge_counter);
        tree[vertex_counter+6] = array[(2*i)+1];
      } else {
        tree[vertex_counter+4] = -1;
        tree[vertex_counter+5] = -1;
        tree[vertex_counter+6] = -1;
      }
      vertex_counter +=7;
    }
    printf("Alloc:%d\n",alloc );
    for (size_t i = 0; i < alloc; i++) {
      printf("i:%2d:%2d:\t%c\n",i,tree[i],tree[i]);
    }

    int next[edge_counter];
    for (size_t i = 0; i < alloc; i+=7) {
      if ((i+3) < (int)(alloc/2)) {
        printf("1.Calculating: %c\n",tree[i+3] );
        next[tree[i+1]] = (int) tree[i+8];
        next[tree[i+2]] = (int) tree[i+4];
      }
      if ((i+6) < (int)(alloc/2)) {
        printf("2.Calculating: %c\n",tree[i+6] );
        if (i+15 < alloc)
          next[tree[i+4]] = (int) tree[i+15];
        else
          next[tree[i+4]] = (int) tree[i+5];
        next[tree[i+5]] = (int) tree[i+1];
      }
      if ((i+3) >= (int)(alloc/2)) {
        printf("3.Calculating: %c\n",tree[i+3] );
        next[tree[i+1]] = (int) tree[i+2];
        if (tree[i+4] == -1 && (i/7)%2 != 0)
          next[tree[i+2]] = (int) tree[i-5];
        else if (tree[i+4] == -1)
          next[tree[i+2]] = (int) tree[i-9];
        else
          next[tree[i+2]] = (int) tree[i+4];
      }
      if ((i+6) >= (int)(alloc/2)) {
        printf("4.Calculating: %c\n",tree[i+6] );
        next[tree[i+4]] = (int) tree[i+5];
        if (i == 0)
          next[tree[i+5]] = (int) tree[i+1];
        else if ((i/7)%2 != 0)
          next[tree[i+5]] = (int) tree[i-5];
        else
          next[tree[i+5]] = (int) tree[i-9];
      }
    }
    for (size_t i = 0; i <= edge_counter; i++) {
      printf("i:%2d:\t%2d\n",i,next[i]);
    }

  }
  MPI_Finalize();
  return 0;
 }
