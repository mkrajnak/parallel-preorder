/*  Project: merge-splitting sort, project for PRL course at FIT BUT
    Author: Martin Krajnak
*/

#include <math.h>
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

/**
* Determine vertex to which given edge leads
*/
int get_vertex(int edge){
  if ((edge%4) == 1) {        // left son vertex
    return (int)ceil((edge/2.)+1);
  } else if ((edge%4) == 2) { // parent from left vertex
    return (int)((edge/4.)+1);
  } else if ((edge%4) == 3) { // right son vertex
    return (int)ceil((edge/2.)+1);
  } else if ((edge%4) == 0) { // parent from right vertex
    return (int)(edge/4.);
  }
}

/**
* Compute the Euler tour
*/
int get_next(int edge, int length){
  int v = get_vertex(edge);
  if (2*v <= length){
    if (edge==4 || (length == 2 && edge==2)) //  I am gR00t
      return edge;
    else if (edge%4 == 0)   // going up
      return ((edge-2)/2-1);
    else if (edge%2 == 0 && (2*v+1) <= length) // no right neighbor, go up
      return edge+1;
    else if (edge%2 == 0)   // the end
      return ((edge/2)-1);
    else
      return ((v-1)*4+1);   // go down
  } else {
    return edge+1;          // go right
  }
}


int main(int argc, char **argv) {
  int numprocs;               // number of cpus obtained from mpi
  int myid;                   // cpu identifier
  MPI_Status stat;

  MPI_Init(&argc,&argv);                          // MPI init
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);       // obtain numprocs
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);           // obtain of of each proc

  //printf("%s\n",argv[1]);
  size_t n = strlen(argv[1]);
  size_t edges_num = 2*n-2;

  int edges[edges_num];

  if (myid == 0) {
    // printf("%2d: %2d iterations on %2d\n",myid, (int)ceil(log2((double)edges_num)), edges_num);
    for (size_t i = 0; i < edges_num; i++) { // init edges with numbers
      edges[i] = i+1;
      printf("%3d",edges[i]);
    }
    printf("\n");
    for (size_t i = 0; i < edges_num; i++) { // init edges with numbers
      printf("%3d",get_next(edges[i],n) );
    }
    printf("\n");
  }
  // obtain the subarray (sub_nums) for every proc
  int edge;
  MPI_Scatter(edges, 1, MPI_INT, &edge, 1, MPI_INT, 0, MPI_COMM_WORLD);
  int rank = 0, pred = -1, succ = -1;
  int my_id = myid+1;
  int euler_tour = get_next(edge,n);
  // printf("%2d:%2d\n",my_id, edge );

  if (edge == euler_tour) {
    succ = -1;
  } else {
    rank = 1;
    succ = euler_tour;
  }
  // printf("%2d:%2d\n",my_id, succ );
  if (succ != -1 ) {
    MPI_Send(&my_id, 1, MPI_INT, succ-1, TAG, MPI_COMM_WORLD);
  }
  if (myid != 0) {
    MPI_Recv(&pred, 1, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &stat);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if (get_next(pred,n) != edge) {
    printf("BLYAT:%2d,Next:%2d,Pred%2d,Rank%2d\n",my_id, succ, pred, rank);
  }

  // printf("edges_num%2d\n",edges_num );
  int tmp;
  for (size_t i = 1; i <= (int)ceil(log2((double)edges_num)); i++) {
    printf("my_id:%2d, I:%2d, succ:%2d pred:%2d\n", my_id, i, succ, pred);
    if (succ==-1) {
      printf("1.CPU:%2d i:%2d WAIT\n",my_id, i, pred);
      MPI_Send(&rank, 1, MPI_INT, pred-1, TAG, MPI_COMM_WORLD);
      MPI_Send(&succ, 1, MPI_INT, pred-1, TAG, MPI_COMM_WORLD);
      MPI_Recv(&pred, 1, MPI_INT, pred-1, TAG, MPI_COMM_WORLD, &stat);
      printf("1.CPU:%2d i:%2d END\n",my_id, i, pred);
    }
    else if (myid==0) {
      printf("2.CPU:%2d i:%2d WAIT\n",my_id, i, pred);
      MPI_Recv(&tmp, 1, MPI_INT, succ-1, TAG, MPI_COMM_WORLD, &stat);
      MPI_Recv(&succ, 1, MPI_INT, succ-1, TAG, MPI_COMM_WORLD, &stat);
      MPI_Send(&pred, 1, MPI_INT, succ-1, TAG, MPI_COMM_WORLD);
      printf("2.CPU:%2d i:%2d END\n",my_id, i, pred);
      rank += tmp;
    } else {
      printf("3.CPU:%2d i:%2d WAIT\n",my_id, i, pred);
      MPI_Send(&rank, 1, MPI_INT, pred-1, TAG, MPI_COMM_WORLD);
      MPI_Send(&succ, 1, MPI_INT, pred-1, TAG, MPI_COMM_WORLD);
      MPI_Send(&pred, 1, MPI_INT, succ-1, TAG, MPI_COMM_WORLD);
      MPI_Recv(&tmp, 1, MPI_INT, succ-1, TAG, MPI_COMM_WORLD, &stat);
      MPI_Recv(&succ, 1, MPI_INT, succ-1, TAG, MPI_COMM_WORLD, &stat);
      printf("3.2CPU:%2d i:%2d WAIT\n",my_id, i, pred);
      MPI_Recv(&pred, 1, MPI_INT, pred-1, TAG, MPI_COMM_WORLD, &stat);
      printf("3.CPU:%2d i:%2d WAIT\n",my_id, i, pred);
      rank += tmp;
    }

  }
  MPI_Barrier(MPI_COMM_WORLD);
  printf("%2d,%2d\n",my_id, rank);
  // printf("1.Myid:%2d,Next:%2d,Pred%2d,Rank%2d\n",myid+1, succ+1, pred, rank);
  // MPI_Gather(&edge, 1, MPI_INT, rank, 1, MPI_INT, 0, MPI_COMM_WORLD);
  //
  // if (myid == 0) {
  //   for (size_t i = 0; i < edges_num; i++) { // init edges with numbers
  //     printf("%2d",edges_num-rank[i]);
  //   }
  //   printf("\n");
  // }

  MPI_Finalize();
  return 0;
 }
