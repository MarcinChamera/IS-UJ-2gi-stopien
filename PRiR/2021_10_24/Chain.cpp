#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include "mpi.h"

using namespace std;

const int SIZE = 5;
int * aarray = new int[ SIZE ];
int my_rank;
int processes;

useconds_t SLEEP_TIME = 150000;

void calc() {
   for ( int i = 0; i < SIZE; i++ ) {
      aarray[ i ]++;
      cout << my_rank << " aarray[ " << i << " ] = " << aarray[ i ] << endl;
   }
}

void aarrayInit() {
   for ( int i = 0; i < SIZE; i++ )
      aarray[ i ] = i;
}

void sendaarray( int destination ) {
  cout << "Send " << my_rank << " -> " << destination << endl;
  usleep( SLEEP_TIME );
  MPI_Send(aarray, SIZE, MPI_INT, destination, 0, MPI_COMM_WORLD);
}

void receiveaarray( int source ) {
  MPI_Status status;
  MPI_Recv(aarray, SIZE, MPI_INT, source, 0, MPI_COMM_WORLD, &status);
  cout << "aarray received " << my_rank << " <- " << source << endl;
  usleep( SLEEP_TIME );
}

int main(int argc, char **argv) {
  
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &processes);
  
  cout << "Oto ja " << my_rank << "/" << processes << endl;

  if ( ! my_rank ) aarrayInit(); // w procesie o numerze 0 inicjacja tablicy

  int destination = ( my_rank + 1 ) % processes;
  int source = ( processes + my_rank - 1 ) % processes;
  
  for ( int i = 0; i < 10; i++ ) {
    if (! my_rank) {
        calc();
        sendaarray( destination );
        receiveaarray( source );
    } else {
        receiveaarray( source );
        calc();
        sendaarray( destination );
    }
  }   
 
  MPI_Finalize();
}