/*
 * SequentialDataProcessor.cpp
 *
 *  Created on: 20 paź 2021
 *      Author: oramus
 */

#include "MPIDataProcessor.h"
#include "Alloc.h"
#include "mpi.h" 
#include <iomanip>
#include <iostream>

using namespace std;

void MPIDataProcessor::processZeroJob(int numberOfProcesses){

	int myDataPortionSize = int(dataSize/numberOfProcesses);
	int restMyDataPortionSize = dataSize % numberOfProcesses;
	int a = myDataPortionSize+restMyDataPortionSize;
	for(int i = 1; i<numberOfProcesses; i++){
		MPI_Send(&dataSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		MPI_Send(&myDataPortionSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		int stop = a+myDataPortionSize;
		for(a; a<stop; a++){
			for(int b = 0; b<dataSize; b++){
				MPI_Send( &(data[a][b]), 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
			}
		}
		a = stop;
	}
}

void MPIDataProcessor::shareData() {
	
	int proccessNumber, numberOfProcesses, myDataPortionSize;
	
	MPI_Comm_rank( MPI_COMM_WORLD, &proccessNumber );
	MPI_Comm_size( MPI_COMM_WORLD, &numberOfProcesses );

	if(proccessNumber == 0)
	{
		processZeroJob(numberOfProcesses);
	}	
	else{

		MPI_Status status;
		MPI_Recv(&dataSize, 1, MPI_INT, 0,0, MPI_COMM_WORLD, &status);
		MPI_Recv(&myDataPortionSize, 1, MPI_INT, 0,0, MPI_COMM_WORLD, &status);
		
		data = new double*[dataSize];
		int allocMemory = 0;
		if(proccessNumber == numberOfProcesses-1)
		{
			allocMemory = myDataPortionSize+margin;
		}else{
			allocMemory = myDataPortionSize+2*margin;
		}
		
		for(int i=0;i<allocMemory;i++){
			data[i] = new double[dataSize];
		}

		for(int a = margin; a<myDataPortionSize+margin; a++){
			for(int b = 0; b<dataSize; b++){
				MPI_Recv(&(data[a][b]), 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
			}
		}

	}
	nextData = tableAlloc(dataSize);

	if(proccessNumber==0){
		for(int a = 0 ; a < myDataPortionSize+(dataSize % numberOfProcesses); a++){
			for(int b = 0; b< dataSize; b++){
				nextData[a][b] = data[a][b];
			}
		}
	}
	else if(proccessNumber == numberOfProcesses-1){
		for(int a = margin; a < myDataPortionSize+margin; a++){
			for(int b = 0; b< dataSize; b++){
				nextData[a][b] = data[a][b];
			}
		}
	}else{
		for(int a = margin; a < myDataPortionSize+margin; a++){
			for(int b = 0; b< dataSize; b++){
				nextData[a][b] = data[a][b];
			}
		}
	}
}

void MPIDataProcessor::sendData(){

	int proccessNumber, numberOfProcesses;

	MPI_Comm_rank( MPI_COMM_WORLD, &proccessNumber );
	MPI_Comm_size( MPI_COMM_WORLD, &numberOfProcesses );

	int myDataPortionSize = int(dataSize/numberOfProcesses);
	int restMyDataPortionSize = dataSize % numberOfProcesses;

	if(proccessNumber==0){
		for(int a = myDataPortionSize+restMyDataPortionSize-margin; a<myDataPortionSize+restMyDataPortionSize; a++){
			for(int b = 0; b<dataSize; b++){
				MPI_Send(&(data[a][b]), 1, MPI_DOUBLE, proccessNumber+1, 0, MPI_COMM_WORLD);
			}
		}
	}else{
		for(int a = margin; a<2*margin; a++){
			for(int b = 0; b<dataSize; b++){
				MPI_Send(&(data[a][b]), 1, MPI_DOUBLE, proccessNumber-1, 0, MPI_COMM_WORLD);
			}
		}
		if(proccessNumber != numberOfProcesses-1)
		{
			for(int a = myDataPortionSize; a<myDataPortionSize+margin; a++){
				for(int b = 0; b<dataSize; b++){
					MPI_Send(&(data[a][b]), 1, MPI_DOUBLE, proccessNumber+1, 0, MPI_COMM_WORLD);
				}
			}
		}
		
	}
}

void MPIDataProcessor::receiveData(){

	int proccessNumber, numberOfProcesses;

	MPI_Comm_rank( MPI_COMM_WORLD, &proccessNumber );
	MPI_Comm_size( MPI_COMM_WORLD, &numberOfProcesses );

	int myDataPortionSize = int(dataSize/numberOfProcesses);
	int restMyDataPortionSize = dataSize % numberOfProcesses;
	MPI_Status status;

	if(proccessNumber==0){
		for(int a = myDataPortionSize+restMyDataPortionSize; a<myDataPortionSize+restMyDataPortionSize+margin; a++){
			for(int b = 0; b<dataSize; b++){
				MPI_Recv(&(data[a][b]), 1, MPI_DOUBLE, proccessNumber+1, 0, MPI_COMM_WORLD, &status);
			}
		}

	}else{
		for(int a = 0; a<margin; a++){
			for(int b = 0; b<dataSize; b++){
				MPI_Recv(&(data[a][b]), 1, MPI_DOUBLE, proccessNumber-1, 0, MPI_COMM_WORLD, &status);
			}
		}
		if(proccessNumber != numberOfProcesses-1)
		{
			for(int a = myDataPortionSize+margin; a<myDataPortionSize+margin*2; a++){
				for(int b = 0; b<dataSize; b++){
					MPI_Recv(&(data[a][b]), 1, MPI_DOUBLE, proccessNumber+1, 0, MPI_COMM_WORLD, &status);
				}
			}
		}	
	}

}

void MPIDataProcessor::singleExecution(){


	int proccessNumber, numberOfProcesses;

	MPI_Comm_rank( MPI_COMM_WORLD, &proccessNumber );
	MPI_Comm_size( MPI_COMM_WORLD, &numberOfProcesses );

	int myDataPortionSize = int(dataSize/numberOfProcesses);
	int restMyDataPortionSize = dataSize % numberOfProcesses;
	MPI_Status status;

	// dotad wszystko co by bylo wykonane (czyli shareData i powyzszy blok w singleExecution)
	// zostaly raczej dobrze przeniesione

	int kuTemp =0;
	if(proccessNumber == 0){
		kuTemp=myDataPortionSize+restMyDataPortionSize+margin;
	}else if(numberOfProcesses-1 == proccessNumber){
		kuTemp = int( dataSize / numberOfProcesses );
		kuTemp = margin + kuTemp;
	}else{
		kuTemp = int( dataSize / numberOfProcesses );
		kuTemp = margin + kuTemp + margin;
	}
	// receive i send data maja taki sam zakres indexow jak moje rozwiazanie - czyli jest git
	sendData();
	receiveData();
	double *buffer = new double[dataPortionSize];
	
	for (int row = margin; row < kuTemp-margin; row++){
		for (int col = margin; col < dataSize - margin; col++) {
			createDataPortion(row, col, buffer);
			nextData[row][col] = function->calc(buffer);
		}
	}
	
	delete[] buffer;
	double **tmp = data;
	data = nextData;
	nextData = tmp;

		if(proccessNumber==0){
		for(int a = 0 ; a < myDataPortionSize+restMyDataPortionSize; a++){
			for(int b = 0; b< dataSize; b++){
				nextData[a][b] = data[a][b];
			}
		}
	}
	else if(proccessNumber == numberOfProcesses-1){
		for(int a = margin; a < myDataPortionSize+margin; a++){
			for(int b = 0; b< dataSize; b++){
				nextData[a][b] = data[a][b];
			}
		}
	}else{
		for(int a = margin; a < myDataPortionSize+margin; a++){
			for(int b = 0; b< dataSize; b++){
				nextData[a][b] = data[a][b];
			}
		}
	}
}

void MPIDataProcessor::collectData(){
	int proccessNumber, numberOfProcesses;

	MPI_Comm_rank( MPI_COMM_WORLD, &proccessNumber );
	MPI_Comm_size( MPI_COMM_WORLD, &numberOfProcesses );

	int myDataPortionSize = int(dataSize/numberOfProcesses);
	int restMyDataPortionSize = dataSize % numberOfProcesses;
	MPI_Status status;

	if(proccessNumber == 0){
		for(int i = 1; i<numberOfProcesses; i++){
			for(int a = i*myDataPortionSize+restMyDataPortionSize;a<i*myDataPortionSize+restMyDataPortionSize+myDataPortionSize;a++){
				for(int b = 0; b<dataSize; b++){
					MPI_Recv(&(data[a][b]), 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
				}
			}
		}
	}else{
		for(int a = margin; a<myDataPortionSize+margin;a++){
			for(int b = 0; b<dataSize; b++){
				MPI_Send(&(data[a][b]), 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
			}
		}
	}
}



void MPIDataProcessor::createDataPortion(int row, int col,
		double *buffer) {
	int counter = 0;
	for (int i = row - margin; i <= row + margin; i++)
		for (int j = col - margin; j <= col + margin; j++)
			buffer[counter++] = data[i][j];
}