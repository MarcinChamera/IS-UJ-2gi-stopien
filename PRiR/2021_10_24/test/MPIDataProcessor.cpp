/*
 * MPIDataProcessor.cpp
 *
 *  Created on: 27.01.2021
 *      Author: chamera
 */

#include "MPIDataProcessor.h"
#include "Alloc.h"
#include "mpi.h"

MPIDataProcessor::MPIDataProcessor() {
	MPI_Comm_size(MPI_COMM_WORLD, &numOfProcesses);
}

void MPIDataProcessor::shareData() {
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Status status;
	if (rank == 0) {
		for (int i = 1; i < numOfProcesses; i++) {
			MPI_Send(&dataSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
	}
	else {
		MPI_Recv(&dataSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
	}

	columnsNotFirstProcess = int(dataSize / numOfProcesses);
	columnsFirstProcess = columnsNotFirstProcess + dataSize % numOfProcesses;
	
	int col = columnsFirstProcess;
	if (rank == 0) {
		for (int processNumber = 1; processNumber < numOfProcesses; processNumber++) {
			int colStop = col + columnsNotFirstProcess;
			for (col; col < colStop; col++) {
				for (int row = 0; row < dataSize; row++) {
					MPI_Send(&(data[col][row]), 1, MPI_DOUBLE, processNumber, processNumber, MPI_COMM_WORLD);
				}
			}
			col = colStop;
		}
	}
	else {
		int howMuchMargin = rank != numOfProcesses - 1 ? 2 * margin : margin;
		data = new double* [columnsNotFirstProcess + howMuchMargin];
		for (int i = 0; i < columnsNotFirstProcess + howMuchMargin; i++) {
			data[i] = new double[dataSize];
		}
		
		for (int col = margin; col < columnsNotFirstProcess + margin; col++) {
			for (int row = 0; row < dataSize; row++) {
				MPI_Recv(&(data[col][row]), 1, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD, &status);			}
		}
	}	

	nextData = tableAlloc(dataSize);

	if (rank == 0) {
		for (int i = 0; i < columnsFirstProcess; i++) {
			for (int j = 0; j < dataSize; j++) {
				nextData[i][j] = data[i][j];
			}
		}
	}
	else {
		for (int i = margin; i < columnsNotFirstProcess + margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				nextData[i][j] = data[i][j];
			}
		}
	}
}

void MPIDataProcessor::singleExecution() {
	MPI_Status status;
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {
		for (int i = columnsFirstProcess - margin; i < columnsFirstProcess; i++) {
			for (int j = 0; j < dataSize; j++) {
				MPI_Send(&(data[i][j]), 1, MPI_DOUBLE, rank + 1, rank + 1, MPI_COMM_WORLD);
			}
		}
	}
	else {
		for (int i = margin; i < 2 * margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				MPI_Send(&(data[i][j]), 1, MPI_DOUBLE, rank - 1, rank - 1, MPI_COMM_WORLD);			
			}
		}
		if (rank != numOfProcesses - 1) {
			for (int i = columnsNotFirstProcess; i < columnsNotFirstProcess + margin; i++) {
				for (int j = 0; j < dataSize; j++) {
					MPI_Send(&(data[i][j]), 1, MPI_DOUBLE, rank + 1, rank + 1, MPI_COMM_WORLD);			
				}
			}
		}
	}

	if (rank == 0) {
		for (int i = columnsFirstProcess; i < columnsFirstProcess + margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				MPI_Recv(&(data[i][j]), 1, MPI_DOUBLE, rank + 1, rank, MPI_COMM_WORLD, &status);
			}
		}
	}
	else {
		for (int i = 0; i < margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				MPI_Recv(&(data[i][j]), 1, MPI_DOUBLE, rank - 1, rank, MPI_COMM_WORLD, &status);
			}
		}
		if (rank != numOfProcesses - 1) {
			for (int i = columnsNotFirstProcess + margin; i < columnsNotFirstProcess + 2 * margin; i++) {
				for (int j = 0; j < dataSize; j++) {
					MPI_Recv(&(data[i][j]), 1, MPI_DOUBLE, rank + 1, rank, MPI_COMM_WORLD, &status);
				}
			}
		}
	}

	int columnStop;
	if (rank == 0) {
		columnStop = columnsFirstProcess;
	}
	else if (rank > 0 && rank < numOfProcesses - 1) {
		columnStop = columnsNotFirstProcess + margin;
	}
	else {
		columnStop = columnsNotFirstProcess;
	}

	double *dataPortionBuffer = new double[dataPortionSize];
	for (int col = margin; col < columnStop; col++) {
		for (int row = margin; row < dataSize - margin; row++) {
			createDataPortion(col, row, dataPortionBuffer);
			nextData[col][row] = function -> calc(dataPortionBuffer);
		}
	}
	
	delete[] dataPortionBuffer;

	double **tmp = data;
	data = nextData;
	nextData = tmp;
}

void MPIDataProcessor::createDataPortion(int col, int row, double *buffer) {
	int counter = 0;
	for (int i = col - margin; i <= col + margin; i++) {
		for (int j = row - margin; j <= row + margin; j++) {
			buffer[counter++] = data[i][j];
		}
	}
}

void MPIDataProcessor::collectData() {
	MPI_Status status;
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if (rank == 0) {
		for (int processNumber = 1; processNumber < numOfProcesses; processNumber++) {
			// for (int i = processNumber * columnsNotFirstProcess + (columnsFirstProcess - columnsNotFirstProcess); i < processNumber * columnsNotFirstProcess + columnsFirstProcess; i++ ) {
			for (int i = columnsFirstProcess + (processNumber - 1) * columnsNotFirstProcess; i < processNumber * columnsNotFirstProcess + columnsFirstProcess; i++ ) {
				for (int j = 0; j < dataSize; j++) {
					MPI_Recv(&(data[i][j]), 1, MPI_DOUBLE, processNumber, 0, MPI_COMM_WORLD, &status);
				}
			}
		}
	}
	else {
		for (int i = margin; i < columnsNotFirstProcess + margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				MPI_Send(&(data[i][j]), 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
			}
		}
	}
}