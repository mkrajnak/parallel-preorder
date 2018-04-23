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
  int rank[edges_num];
  char euler_tour[edges_num];

  if (myid == 0) {
    for (size_t i = 0; i < edges_num; i++) { // init edges with numbers
      edges[i] = i;
      //printf("%2d:%2d\n",i,edges[i] );
    }
  }
  int edge;
  // obtain the subarray (sub_nums) for every proc
  MPI_Scatter(edges, 1, MPI_INT, &edge, 1, MPI_INT, 0, MPI_COMM_WORLD);

  rank[myid] = euler_tour[myid] = get_next(edge+1,n);

  int last = 0, succ = 0;
  if (myid == edges_num-1) {
    last = edge;
    edge = 0;
    succ = myid;
  } else {
    succ = myid+1;
    edge = 1;
  };
  // printf("%2d: Will do %2d iterations\n",myid, (int)log(edges_num));

  int tmp = 0;
  for (size_t i = 1; i <= edges_num; i++) {
    if (myid == succ || succ == edges_num-1) {
      int sendto = myid-i;
      if (sendto > -1) {
        // printf("CPU:%d i:%d SENDING %2d\n",myid, i, sendto);
        MPI_Send(&edge, 1, MPI_INT, sendto, TAG, MPI_COMM_WORLD);
        continue; // We have the result, nothing to compute
      } else {
        break;    // I am no successor, quit
      }
    }
    else if (myid == 0) {
      // printf("CPU:%d i:%d WAITING\n",myid, i);
      MPI_Recv(&tmp, 1, MPI_INT, succ, TAG, MPI_COMM_WORLD, &stat);
      // printf("CPU:%d i:%d RECEIVED %2d\n",myid, i, tmp);
    } else {
      int sendto = myid-i;
      if (sendto > -1) {
        // printf("CPU:%d i:%d SENDING %2d\n",myid, i, sendto);
        MPI_Send(&edge, 1, MPI_INT, sendto, TAG, MPI_COMM_WORLD);
      }
      // printf("CPU:%d i:%d WAITING for  %2d\n",myid, i, succ);
      MPI_Recv(&tmp, 1, MPI_INT, succ, TAG, MPI_COMM_WORLD, &stat);
      // printf("CPU:%d i:%d RECEIVED %2d:\n",myid, i, tmp);
    }
    edge += tmp;
    succ += i;
    if (succ >= edges_num-1) {
      succ = edges_num-1;
    }
  }
  printf("CPU:%2d Calculated %2d:succ %2d\n", myid, edges_num - edge, succ);
  //MPI_Gather(&edge, 1, MPI_INT, edges, 1, MPI_INT, 0, MPI_COMM_WORLD);

  MPI_Finalize();
  return 0;
 }
