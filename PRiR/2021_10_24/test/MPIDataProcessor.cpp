/*
 * MPIDataProcessor.cpp
 *
 *  Created on: 19.12.2021
 *      Author: chamera
 */

#include "MPIDataProcessor.h"
#include "Alloc.h"
#include "mpi.h"
#include <string>

MPIDataProcessor::MPIDataProcessor() {
	MPI_Comm_size( MPI_COMM_WORLD, &numOfProcesses );
}

void MPIDataProcessor::shareData() {
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Status status;
	if (rank == 0) {
		int *dataSizeBuffer = new int[1];
		dataSizeBuffer[0] = dataSize;
		for (int i = 1; i < numOfProcesses; i++)
			MPI_Send(dataSizeBuffer, sizeof(int), MPI_INT, i, 0, MPI_COMM_WORLD);
		delete[] dataSizeBuffer;
	}
	else {
		int *dataSizeBuffer = new int[1];
		MPI_Recv(dataSizeBuffer, sizeof(int), MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		dataSize = dataSizeBuffer[0];
		delete[] dataSizeBuffer;
	}

	columnsNotLastProcess = (dataSize % numOfProcesses != 0 ? dataSize / numOfProcesses + 1 : dataSize / numOfProcesses);
	columnsLastProcess = dataSize - (numOfProcesses - 1) * columnsNotLastProcess;
	int columnsInCurrentProcess = (rank != numOfProcesses - 1 ? columnsNotLastProcess : columnsLastProcess);
	
	if (rank == 0) {
		for (int processNumber = 1; processNumber < numOfProcesses; processNumber++) {
			int counter = 0;
			int columnsInReceivingProcess = processNumber != numOfProcesses - 1 ? columnsNotLastProcess : columnsLastProcess;
			double *tablePortionSend = new double[columnsInReceivingProcess * dataSize];
			for (int col = processNumber * columnsNotLastProcess; col < processNumber * columnsNotLastProcess + columnsInReceivingProcess; col++) {
				for (int row = 0; row < dataSize; row++) {
					tablePortionSend[counter++] = data[col][row];
				}
			}
			MPI_Send(tablePortionSend, columnsInReceivingProcess * dataSize, MPI_DOUBLE, processNumber, processNumber, MPI_COMM_WORLD);
			delete[] tablePortionSend;
		}
	}
	if (rank == 0) {
		nextData = tableAlloc(dataSize);
		for (int i = 0; i < dataSize; i++) {
			for (int j = 0; j < dataSize; j++) {
				nextData[i][j] = data[i][j];
			}
		}
	}
	else {
		int columnsToAllocate = rank < numOfProcesses - 1 ? columnsInCurrentProcess + 2 * margin : columnsInCurrentProcess + margin;
		data = new double* [columnsToAllocate];
		nextData = new double* [columnsToAllocate];
		for (int i = 0; i < columnsToAllocate; i++) {
			data[i] = new double[dataSize];
			nextData[i] = new double[dataSize];
		}

		processTablePortion = new double[columnsInCurrentProcess * dataSize];
		MPI_Recv(processTablePortion, columnsInCurrentProcess * dataSize, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD, &status);
		for (int col = 0; col < columnsInCurrentProcess; col++) {
			for (int row = 0; row < dataSize; row++) {
				data[col + margin][row] = processTablePortion[col * dataSize + row];
				nextData[col + margin][row] = data[col + margin][row];
			}
		}

		delete[] processTablePortion;
	}	
}

void MPIDataProcessor::singleExecution() {
	MPI_Status status;
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int columnsInCurrentProcess = (rank != numOfProcesses - 1 ? columnsNotLastProcess : columnsLastProcess);
	int currentProcessStartingColumn = rank * columnsNotLastProcess;
	double *columnsFromPreviousProcess = new double[margin * dataSize]; 
	double *columnsFromNextProcess = new double[margin * dataSize]; 
	double *columnsForNextProcess = new double[margin * dataSize]; 
	double *columnsForPreviousProcess = new double[margin * dataSize];

	if (rank == 0 && numOfProcesses > 1) {
		int counter = 0;
		for (int i = columnsInCurrentProcess - margin; i < columnsInCurrentProcess; i++) {
			for (int j = 0; j < dataSize; j++) {
				columnsForNextProcess[counter++] = data[i][j];
			}
		}
		MPI_Send(columnsForNextProcess, margin * dataSize, MPI_DOUBLE, rank + 1, rank + 1, MPI_COMM_WORLD);

		MPI_Recv(columnsFromNextProcess, margin * dataSize, MPI_DOUBLE, rank + 1, rank, MPI_COMM_WORLD, &status);
	}
	else if (rank != 0 && rank != numOfProcesses - 1) {
		int counter = 0;
		for (int i = columnsInCurrentProcess; i < columnsInCurrentProcess + margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				columnsForNextProcess[counter++] = data[i][j];
			}
		}
		
		MPI_Send(columnsForNextProcess, margin * dataSize, MPI_DOUBLE, rank + 1, rank + 1, MPI_COMM_WORLD);

		MPI_Recv(columnsFromNextProcess, margin * dataSize, MPI_DOUBLE, rank + 1, rank, MPI_COMM_WORLD, &status);

		counter = 0;
		for (int i = margin; i < 2 * margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				columnsForPreviousProcess[counter++] = data[i][j];
			}
		}
		MPI_Send(columnsForPreviousProcess, margin * dataSize, MPI_DOUBLE, rank - 1, rank - 1, MPI_COMM_WORLD);

		MPI_Recv(columnsFromPreviousProcess, margin * dataSize, MPI_DOUBLE, rank - 1, rank, MPI_COMM_WORLD, &status);
	}
	else {
		int counter = 0;
		for (int i = margin; i < 2 * margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				columnsForPreviousProcess[counter++] = data[i][j];
			}
		}
		MPI_Send(columnsForPreviousProcess, margin * dataSize, MPI_DOUBLE, rank - 1, rank - 1, MPI_COMM_WORLD);


		MPI_Recv(columnsFromPreviousProcess, margin * dataSize, MPI_DOUBLE, rank - 1, rank, MPI_COMM_WORLD, &status);
	}
	delete[] columnsForNextProcess;
	delete[] columnsForPreviousProcess;

	int counter = 0;
	if (rank > 0 && rank < numOfProcesses - 1) {
		for (int i = 0; i < margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				data[i][j] = columnsFromPreviousProcess[counter++];
			}
		}
		counter = 0;
		for (int i = columnsInCurrentProcess + margin; i < columnsInCurrentProcess + 2 * margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				data[i][j] = columnsFromNextProcess[counter++];
			}
		}
	}
	else {
		if (rank == 0) {
			for (int i = columnsInCurrentProcess; i < columnsInCurrentProcess + margin; i++) {
				for (int j = 0; j < dataSize; j++) {
					data[i][j] = columnsFromNextProcess[counter++];
				}
			}
		}
		else {
			for (int i = 0; i < margin; i++) {
				for (int j = 0; j < dataSize; j++) {
					data[i][j] = columnsFromPreviousProcess[counter++];
				}
			}
		}
	}
	delete[] columnsFromPreviousProcess;
	delete[] columnsFromNextProcess;

	int columnStop;
	if (rank == 0) {
		columnStop = columnsInCurrentProcess;
	} else if (rank < numOfProcesses - 1) {
		columnStop = columnsInCurrentProcess + margin;
		if (rank == numOfProcesses - 2 && margin > columnsLastProcess)
			columnStop = columnsInCurrentProcess + margin - (margin - columnsLastProcess);
	}
	else {
		columnStop = columnsLastProcess;
		if (margin > columnsLastProcess)
			columnStop = 0;
	}
	counter = 0;
	double *dataPortionBuffer = new double[dataPortionSize];
	double *calculatedDataPortionBuffer = new double[(dataSize - 2 * margin) * columnsInCurrentProcess];
	for (int col = margin; col < columnStop; col++) {
		for (int row = margin; row < dataSize - margin; row++) {
			createDataPortion(row, col, dataPortionBuffer, rank);
			double result = function -> calc(dataPortionBuffer);
			nextData[col][row] = result;
			if (rank != 0) {
				calculatedDataPortionBuffer[counter++] = result;
			}
		}
	}
	
	delete[] dataPortionBuffer;
	delete[] calculatedDataPortionBuffer;

	if (rank != 0) {
		int sizeToSend = (dataSize - 2 * margin) * (columnStop - margin);
		MPI_Send(calculatedDataPortionBuffer, sizeToSend, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD);
	}
	else if (rank == 0) {
		int dataFromOtherProcessesSize = (dataSize - 2 * margin) * (dataSize - 2 * margin) - ((dataSize - 2 * margin) * (columnsInCurrentProcess - margin));
		double *calculatedDataFromOtherProcesses = new double[dataFromOtherProcessesSize];
		MPI_Status status;
		int sizeToReceive;
		double *p = calculatedDataFromOtherProcesses;
		for (int i = 1; i < numOfProcesses; i++) {
			sizeToReceive = i != numOfProcesses - 1 ? (dataSize - 2 * margin) * columnsNotLastProcess : (dataSize - 2 * margin) * (columnsLastProcess - margin);
			if (sizeToReceive > 0) {
				MPI_Recv(p, sizeToReceive, MPI_DOUBLE, i, i, MPI_COMM_WORLD, &status);
				if (i < numOfProcesses - 1) {
					p += sizeToReceive;
				}
			}
		}
		counter = 0;
		for (int j = columnsNotLastProcess; j < dataSize - margin; j++)
			for (int i = margin; i < dataSize - margin; i++) {
				nextData[j][i] = calculatedDataFromOtherProcesses[counter++];
		}
		delete[] calculatedDataFromOtherProcesses;
	}

	double **tmp = data;
	data = nextData;
	nextData = tmp;
}

void MPIDataProcessor::createDataPortion(int row, int col, double *buffer, int rank) {
	int counter = 0;
	for (int j = col - margin; j <= col + margin; j++) {
		for (int i = row - margin; i <= row + margin; i++) {
			buffer[counter++] = data[j][i];
		}
	}
}

double **MPIDataProcessor::tablePortionAlloc( int rows, int cols ) {
	double **result;
	result = new double* [ rows ]; 
	for ( int i = 0; i < rows; i++ )
		result[ i ] = new double[ cols ]; 
	return result;
}