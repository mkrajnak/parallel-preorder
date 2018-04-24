/*  Project: Paraller preorder with list ranking,
*   project for PRL course at FIT BUT
*   Author: Martin Krajnak, xkrajn02@vutbr.cz
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
#define PRD_TAG 0       // compute predecessor
#define LR_TAG 1        // list ranking
#define PRORDR_TAG 2    // preorder

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

  size_t n = strlen(argv[1]);
  size_t edges_num = 2*n-2;   // edge count
  int edges[edges_num];
  int ranks[edges_num];

  MPI_Init(&argc,&argv);                          // MPI init
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);       // obtain numprocs
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);           // obtain of of each proc

  int rank = 0, pred = -1, succ = -1, temp = 0;
  int my_id = myid+1;   // helper value, counting cpus from 1
  int edge = my_id;     // assign edge for every cpu
  // calculatre successors
  int euler_tour = get_next(edge,n);
  if (edge == euler_tour) {
    succ = -1;          // last edge has no successor, rank remain 0
  } else {
    rank = 1;
    succ = euler_tour;  // sucessor is calculated via euler tour
  }
  // each cpu except last sends his ID to his sucessor to calulate predecessor
  if (succ != -1 )
    MPI_Send(&my_id, 1, MPI_INT, succ-1, PRD_TAG , MPI_COMM_WORLD);
  if (myid != 0)
    MPI_Recv(&pred, 1, MPI_INT, MPI_ANY_SOURCE, PRD_TAG, MPI_COMM_WORLD, &stat);
  // All cpus need to have predecessors before the list ranking starts
  for (size_t i = 1; i <= (int)ceil(log2((double)edges_num)); i++) {
    if (succ==-1) {     // edges that dont have successor
      MPI_Send(&rank, 1, MPI_INT, pred-1, LR_TAG, MPI_COMM_WORLD);
      MPI_Send(&succ, 1, MPI_INT, pred-1, LR_TAG, MPI_COMM_WORLD);
      MPI_Recv(&pred, 1, MPI_INT, pred-1, LR_TAG, MPI_COMM_WORLD, &stat);
      continue;         // reached the end nothing to add
    }
    else if (myid==0) { // first cpu, doesnt have predecessor
      MPI_Send(&pred, 1, MPI_INT, succ-1, LR_TAG, MPI_COMM_WORLD);
      MPI_Recv(&temp, 1, MPI_INT, succ-1, LR_TAG, MPI_COMM_WORLD, &stat);
      MPI_Recv(&succ, 1, MPI_INT, succ-1, LR_TAG, MPI_COMM_WORLD, &stat);
    } else {            // middle
      MPI_Send(&rank, 1, MPI_INT, pred-1, LR_TAG, MPI_COMM_WORLD);
      MPI_Send(&succ, 1, MPI_INT, pred-1, LR_TAG, MPI_COMM_WORLD);
      MPI_Send(&pred, 1, MPI_INT, succ-1, LR_TAG, MPI_COMM_WORLD);
      MPI_Recv(&temp, 1, MPI_INT, succ-1, LR_TAG, MPI_COMM_WORLD, &stat);
      MPI_Recv(&succ, 1, MPI_INT, succ-1, LR_TAG, MPI_COMM_WORLD, &stat);
      MPI_Recv(&pred, 1, MPI_INT, pred-1, LR_TAG, MPI_COMM_WORLD, &stat);
    }
    rank += temp;           // add value received from successor
  }
  rank = edges_num - rank;  // distance to end -> distance to end
  // sort by sending id to rank
  MPI_Send(&my_id, 1, MPI_INT, rank-1, PRORDR_TAG , MPI_COMM_WORLD);
  MPI_Recv(&rank, 1, MPI_INT, MPI_ANY_SOURCE, PRORDR_TAG, MPI_COMM_WORLD, &stat);
  // filter the edges
  if (rank%2 != 0)      // we need only the odd "forward" edges
    rank = (rank+1)/2;  // calculate the vertex in which edge lead
  else
    rank = -1;          // discarding the odd ones
  // collect calculated values
  MPI_Gather(&rank, 1, MPI_INT, ranks, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if (myid == 0) {                        // print result
    printf("%c",argv[1][0]);              // first vertex is always root
    for (size_t i = 0; i < edges_num; i++) {
      if (ranks[i] !=-1) {
        printf("%c",argv[1][ranks[i]]);   // print preorder
      }
    }
    printf("\n");
  }
  MPI_Finalize();
  return 0;
 }
