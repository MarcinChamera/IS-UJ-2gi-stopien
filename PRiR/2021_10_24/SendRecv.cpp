#include <iostream>
#include <mpi.h>

using namespace std;

int main(int argc, char ** argv)
{
	MPI_Init(&argc,&argv);

        int rank ;
        MPI_Comm_rank( MPI_COMM_WORLD, &rank );
        
        switch ( rank ) {
        case 0: {
              char str[] = "Witaj z procesu 0" ;
              MPI_Send( str, sizeof( str ), MPI_CHAR, 1, 100, MPI_COMM_WORLD );
              cout << "Tu proces 0 - dane wyslane" << endl;
              break;
           }
        case 1: {
              int size = 512;
              char *buf = new char[ size ];
              MPI_Status status;
	      //             vvvv --- tu przyciac rozmiar
              MPI_Recv( buf, size, MPI_CHAR, 0, 100, MPI_COMM_WORLD, &status );
              cout << "Tu proces 1 - odebralem : " << buf << endl;
              cout << "Status operacji : SOURCE: " << status.MPI_SOURCE 
                   << " TAG: " << status.MPI_TAG   << " ERROR: " << status.MPI_ERROR
                   << " COUNT: " << status._ucount << endl;
              int count;
              MPI_Get_count( &status, MPI_CHAR, &count );
              cout << "Count " << count << endl;
           }
        }

	MPI_Finalize();
}
